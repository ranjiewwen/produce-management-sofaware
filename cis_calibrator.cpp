#include "stdafx.h"
#include "cis_calibrator.h"
#include "device_proxy_inl.h"
#include "bitmap.h"
#include "working_parameters.h"
#include "working_parameters_inl.h"
#include "error.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CISCalibrator::s_ColorImageIndex[CIS_COUNT][COLOR_COUNT] = {{1, 8}, {33, 40}};

CISCalibrator::CISCalibrator()
  : device_(NULL) {
  memset(&errorInfo_, 0, sizeof(errorInfo_));
}

CISCalibrator::~CISCalibrator() {
}

bool CISCalibrator::Init(DeviceProxy *device, WorkingParameters *parameters) {
  ParameterBlock *model = parameters->GetCurrentModelParameters();
  ASSERT(model != NULL && !model->IsNull());
  ParameterBlock block = model->SelectBlock(_T("Config\\CISCalibration"));
  minOffset_ = block.GetIntParameter(_T("minOffset"), 0);
  maxOffset_ = block.GetIntParameter(_T("maxOffset"), 255);
  minGain_ = block.GetIntParameter(_T("minGain"), 0);
  maxGain_ = block.GetIntParameter(_T("maxGain"), 255);
  offsetBaseGray_ = block.GetFloatParameter(_T("offsetBaseGray"), 0.5);
  minExposureTime_[CIS_COLOR_RED] = block.GetIntParameter(_T("minExposureTimeR"), 5);
  minExposureTime_[CIS_COLOR_GREEN] = block.GetIntParameter(_T("minExposureTimeG"), 5);
  minExposureTime_[CIS_COLOR_BLUE] = block.GetIntParameter(_T("minExposureTimeB"), 5);
  minExposureTime_[CIS_COLOR_IR] = block.GetIntParameter(_T("minExposureTimeIR"), 5);
  maxExposureTime_[CIS_COLOR_RED] = block.GetIntParameter(_T("maxExposureTimeR"), 50);
  maxExposureTime_[CIS_COLOR_GREEN] = block.GetIntParameter(_T("maxExposureTimeG"), 50);
  maxExposureTime_[CIS_COLOR_BLUE] = block.GetIntParameter(_T("maxExposureTimeB"), 50);
  maxExposureTime_[CIS_COLOR_IR] = block.GetIntParameter(_T("maxExposureTimeIR"), 50);
  standardExposureColor_[COLOR_GREEN] = block.GetIntParameter(_T("standardExposureG"), 180);
  standardExposureColor_[COLOR_IR] = block.GetIntParameter(_T("standardExposureIR"), 180);
  acceptableExposureColorDifference_[COLOR_GREEN] = block.GetIntParameter(_T("acceptableExposureDifferenceG"), 10);
  acceptableExposureColorDifference_[COLOR_IR] = block.GetIntParameter(_T("acceptableExposureDifferenceIR"), 10);
  standardWhite_[COLOR_GREEN] = block.GetIntParameter(_T("standardWhiteG"), 200);
  standardWhite_[COLOR_IR] = block.GetIntParameter(_T("standardWhiteIR"), 210);
  acceptableWhiteDifference_[COLOR_GREEN] = block.GetIntParameter(_T("acceptableWhiteDifferenceG"), 10);
  acceptableWhiteDifference_[COLOR_IR] = block.GetIntParameter(_T("acceptableWhiteDifferenceIR"), 10);
  segmentWidth_[0] = block.GetIntParameter(_T("segmentWidth0"), 216 );
  segmentWidth_[1] = block.GetIntParameter(_T("segmentWidth1"), 216 );
  segmentWidth_[2] = block.GetIntParameter(_T("segmentWidth2"), 288);
  acceptableGrayLevel_ = block.GetIntParameter(_T("acceptableGrayLevel"), 128);
  leftMargin_ = block.GetIntParameter(_T("leftMargin"), 16);
  rightMargin_ = block.GetIntParameter(_T("rightMargin"), 16);
  acceptableDitherX_ = block.GetIntParameter(_T("acceptableDitherX"), 80);
  acceptableDitherY_ = block.GetIntParameter(_T("acceptableDitherY"), 30);

  device_ = device;

  memset(dustCount_, 0, sizeof(dustCount_));
  memset(&errorInfo_, 0, sizeof(errorInfo_));

  /*for (int side = 0; side < CIS_COUNT; side++) {
    for (int color = 0; color < COLOR_COUNT; color++) {
      images_[CIS_TOP][COLOR_GREEN][0].Destroy();
      images_[CIS_COUNT][COLOR_COUNT][1].Destroy();
    }
  }*/
  return true;
}

bool CISCalibrator::Correct() {
  bool hasError = false;

  // 设置参数取黑图
  // Exposure time: min
  // Gain: 校准过的参数
  // Offset: 校准过的参数

  int startTime = GetTickCount();

  CISParameter parameters[CIS_COUNT];
  for (int side = 0; side < CIS_COUNT; side++) {
    parameters[side].exposureTime[CIS_COLOR_RED] = minExposureTime_[CIS_COLOR_RED];
    parameters[side].exposureTime[CIS_COLOR_GREEN] = minExposureTime_[CIS_COLOR_GREEN];
    parameters[side].exposureTime[CIS_COLOR_BLUE] = minExposureTime_[CIS_COLOR_BLUE];    
    parameters[side].exposureTime[CIS_COLOR_IR] = minExposureTime_[CIS_COLOR_IR];
    for (int segment = 0; segment < CIS_SEGMENT_COUNT; segment++) {
      parameters[side].gain[segment] = parameters_[side].gain[segment];
      parameters[side].offset[segment] = parameters_[side].offset[segment];
    }
  }
  if (!device_->SetCISParameter(parameters)) {
    SetErrorInfo(-1, -1, -1, GetLastError());
    return false;
  }

  //CFile kFile(_T("d:\\k.bin"), CFile::modeCreate | CFile::typeBinary | CFile::modeWrite);
  //CFile bFile(_T("d:\\b.bin"), CFile::modeCreate | CFile::typeBinary | CFile::modeWrite);

  for (int side = 0; side < CIS_COUNT; side++) {
    for (int color = 0; color < COLOR_COUNT; color++) {
    
      if (!device_->TakeCISImage(s_ColorImageIndex[side][color], &images_[side][color][BLACK])) {
        SetErrorInfo(side, -1, color, GetLastError());
        return false;
      }

      CalcLines(&images_[side][color][BLACK], 0, CIS_IMAGE_WIDTH, blackLine_[side][color]);
      
      double standardWhite = 220; // standardWhite_[color]; // CalcMeans(whiteLine_[side][color], leftMargin_, CIS_IMAGE_WIDTH - leftMargin_ - rightMargin_);
      double standardBlack = CalcMeans(blackLine_[side][color], leftMargin_, CIS_IMAGE_WIDTH - leftMargin_ - rightMargin_);

      dustCount_[side][color] = 0;
      
      for (int x = 0; x < CIS_IMAGE_WIDTH; x++) {
        double white = whiteLine_[side][color][x].avgGray;
        double black = blackLine_[side][color][x].avgGray;
        double level = white - black;
        if (x >= leftMargin_ && x < CIS_IMAGE_WIDTH - rightMargin_) {
          // 检查有效范围内的黑白图的灰度差
          if (level < acceptableGrayLevel_) {
            dustPositions_[side][color][dustCount_[side][color]++] = x;
            hasError = true;
          }
        }
        double k = (standardWhite - standardBlack) / level;
        double b = standardWhite - k * white;
        for (int c = 0; c < 256; c++) {
          int c1 = (int)floor(k * c + b + 0.5);
          if (c1 < 0) {
            c1 = 0;
          } else if (c1 > 255) {
            c1 = 255;
          }
          correctionTable_[side][color][x][c] = c1;
        }
        //kFile.Write(&k, sizeof(k));
        //bFile.Write(&b, sizeof(b));
      }
    }
  }

  if (hasError) {
    SetErrorInfo(-1, -1, -1, ERROR_CIS_LOW_CONTRAST);
    return false;
  }  

  TRACE(_T("Running time of KB Adjust: %d\n"), GetTickCount() - startTime);

  return true;
}

bool CISCalibrator::CheckUniformity() {
  // 设置参数取中间图应用校准表后检查均匀性
  // Exposure time: middle
  // Gain: 校准过的参数
  // Offset: 校准过的参数

	int startTime = GetTickCount();

  CISParameter parameters[CIS_COUNT];
  for (int side = 0; side < CIS_COUNT; side++) {
    parameters[side].exposureTime[CIS_COLOR_RED] = minExposureTime_[CIS_COLOR_RED];
    parameters[side].exposureTime[CIS_COLOR_GREEN] = (minExposureTime_[CIS_COLOR_GREEN] + parameters_[side].exposureTime[CIS_COLOR_GREEN]) / 2;
    parameters[side].exposureTime[CIS_COLOR_BLUE] = minExposureTime_[CIS_COLOR_BLUE];    
    parameters[side].exposureTime[CIS_COLOR_IR] = (minExposureTime_[CIS_COLOR_IR] +  parameters_[side].exposureTime[CIS_COLOR_IR]) / 2;
    for (int segment = 0; segment < CIS_SEGMENT_COUNT; segment++) {
      parameters[side].gain[segment] = parameters_[side].gain[segment];
      parameters[side].offset[segment] = parameters_[side].offset[segment];
    }
  }
  if (!device_->SetCISParameter(parameters)) {
    SetErrorInfo(-1, -1, -1, GetLastError());
    return false;
  }

  for (int side = 0; side < CIS_COUNT; side++) {
    for (int color = 0; color < COLOR_COUNT; color++) {
      if (!device_->TakeCISImage(s_ColorImageIndex[side][color], &images_[side][color][WHITE])) {
        SetErrorInfo(side, -1, color, GetLastError());
        return false;
      }

      // 应用校准表
      int width = images_[side][color][WHITE].GetWidth();
      int height = images_[side][color][WHITE].GetHeight();
      ASSERT(width == CIS_IMAGE_WIDTH);
      unsigned char *p = (unsigned char *)images_[side][color][WHITE].GetBits();
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          *p = correctionTable_[side][color][x][*p];
          p++;
        }
      }

      CalcLines(&images_[side][color][WHITE], 0, CIS_IMAGE_WIDTH, whiteLine_[side][color]);

      if (!CheckUniformity(whiteLine_[side][color], leftMargin_, CIS_IMAGE_WIDTH - leftMargin_ - rightMargin_)) {
        SetErrorInfo(side, -1, color, GetLastError());
        return false;
      }
    }
  }

  if (!device_->SetCISParameter(parameters_)) {
    SetErrorInfo(-1, -1, -1, GetLastError());
    return false;
  }

  TRACE(_T("Running time of Uniformity Checking: %d\n"), GetTickCount() - startTime);

  return true;
}

bool CISCalibrator::AdjustOffset() {
  // 设置初始参数
  // Exposure time: min
  // Gain: min

	int startTime = GetTickCount();

	for (int side = 0; side < CIS_COUNT; side++) 
	{
		parameters_[side].exposureTime[CIS_COLOR_RED] = minExposureTime_[CIS_COLOR_RED];
		parameters_[side].exposureTime[CIS_COLOR_GREEN] = minExposureTime_[CIS_COLOR_GREEN];
		parameters_[side].exposureTime[CIS_COLOR_BLUE] = minExposureTime_[CIS_COLOR_BLUE];    
		parameters_[side].exposureTime[CIS_COLOR_IR] = minExposureTime_[CIS_COLOR_IR];
		for (int segment = 0; segment < CIS_SEGMENT_COUNT; segment++) 
		{
			parameters_[side].gain[segment] = minGain_;
			parameters_[side].offset[segment] = (maxOffset_ + minOffset_) / 2;
		}
	}

	// 为优化CIS校准时间而重写CIS校准代码
	int times = 0;
	for (int side = 0; side < CIS_COUNT; side++)
	{
		int a[CIS_SEGMENT_COUNT] = { 0 };
		int b[CIS_SEGMENT_COUNT] = { 0 };
		for (int segment = 0; segment < CIS_SEGMENT_COUNT; segment++)
		{
			a[segment] = minOffset_;
			b[segment] = maxOffset_;
		}

		int gotIt[CIS_SEGMENT_COUNT] = { 0 };
		while (1)
		{
			for (int color = 0; color < COLOR_COUNT; color++)
			{
				if (!device_->TakeCISImage(parameters_, s_ColorImageIndex[side][color], &images_[side][color][BLACK]))
				{
					SetErrorInfo(side, 0, color, GetLastError());
					return false;
				}
				times++;
			}

			int start = 0;
			bool exit = true;
			for (int segment = 0; segment < CIS_SEGMENT_COUNT; segment++)
			{
				// 该段的有效区间
				int effectiveStart = (segment == 0 ? leftMargin_ : start);
				int effectiveWidth = segmentWidth_[segment] -
					(segment == 0 ? leftMargin_ : segment == CIS_SEGMENT_COUNT - 1 ? rightMargin_ : 0);
				// 调整Offset使所有点的灰度都恰好大于0
				bool satisfy = true;
				for (int color = 0; color < COLOR_COUNT; color++)
				{
					CalcLines(&images_[side][color][BLACK], start, segmentWidth_[segment], blackLine_[side][color]);

					// 统计灰度大于0的点的个数
					int points = 0;
					for (int x = effectiveStart; x < effectiveStart + effectiveWidth; x++) 
					{
						if (blackLine_[side][color][x].avgGray > offsetBaseGray_) 
						{
							points++;
						}
					}

					if (points != effectiveWidth)
					{
						satisfy = false;
					}
				}
				if (satisfy)
				{
					b[segment] = parameters_[side].offset[segment];
					gotIt[segment] = 1;
				}
				else 
				{
					a[segment] = parameters_[side].offset[segment] + 1;
				}

				if (a[segment] < b[segment])
				{
					parameters_[side].offset[segment] = (a[segment] + b[segment]) / 2;
					exit = false;
				}

				// 进行下一段
				start += segmentWidth_[segment];
			}

			if (exit)
			{
				break;
			}
		}

		for (int segment = 0; segment < CIS_SEGMENT_COUNT; segment++)
		{
			if (gotIt[segment] == 0)
			{
				// 无法通过offset将的所有点的灰度都调整到大于0
				SetErrorInfo(side, segment, 0, ERROR_CIS_ADJUST_OFFSET);
				return false;
			}
			parameters_[side].offset[segment] = a[segment];
		}
	}

	TRACE(_T("Offset Adjust Times: %d\n"), times);
	TRACE(_T("Running time of Offset Adjust: %d\n"), GetTickCount() - startTime);

	TRACE(_T("Adjust Offset Finished:[%d, %d, %d]-[%d, %d, %d]\n"), 
		  parameters_[0].offset[0], parameters_[0].offset[1], parameters_[0].offset[2],
		  parameters_[1].offset[0], parameters_[1].offset[1], parameters_[1].offset[2]);
  
	return true;
}

bool CISCalibrator::CheckUniformity(const LineInfo *lines, int start, int width) {
  LastErrorHolder errorHolder;

  // 检查同一点的采样抖动
  for (int x = start; x < start + width; x++) {
    int dither = lines[x].maxGray - lines[x].minGray;
    //TRACE("Dither x(%d)=%d\n", x, dither);
    if (dither > acceptableDitherY_) {
      errorHolder.SetError(ERROR_CIS_DITHER_Y);
      return false;
    }
  }
  // 检查列间的不均匀性  
  double diff = CalcDiff(lines, start, width);
  if (diff > acceptableDitherX_) {
    errorHolder.SetError(ERROR_CIS_DITHER_X);
    return false;
  }
  return true;
}

void CISCalibrator::SetErrorInfo(int side, int segment, int color, int result) {
  errorInfo_.side = side;
  errorInfo_.segment = segment;
  errorInfo_.color = color;
  errorInfo_.result = result;
}

bool CISCalibrator::AdjustExposure() {
  // 设置初始参数
  // offset: 校准过的参数
  // gain: min

	int startTime = GetTickCount();

  for (int side = 0; side < CIS_COUNT; side++) {
    parameters_[side].exposureTime[CIS_COLOR_RED] = minExposureTime_[CIS_COLOR_RED];
    parameters_[side].exposureTime[CIS_COLOR_GREEN] = minExposureTime_[CIS_COLOR_GREEN];
    parameters_[side].exposureTime[CIS_COLOR_BLUE] = minExposureTime_[CIS_COLOR_BLUE];    
    parameters_[side].exposureTime[CIS_COLOR_IR] = minExposureTime_[CIS_COLOR_IR];
    for (int segment = 0; segment < CIS_SEGMENT_COUNT; segment++) {
      parameters_[side].gain[segment] = minGain_;
      parameters_[side].offset[segment] = parameters_[side].offset[segment];
    }
  }

  for (int side = 0; side < CIS_COUNT; side++) {
    for (int color = 0; color < COLOR_COUNT; color++) {
      int cisColor = (color == COLOR_GREEN ? CIS_COLOR_GREEN : CIS_COLOR_IR);
      int a = minExposureTime_[cisColor];
      int b = maxExposureTime_[cisColor];
	  int times = 0;
      while (a < b) {
        parameters_[side].exposureTime[cisColor] = (a + b) / 2;
        
        if (!device_->TakeCISImage(parameters_, s_ColorImageIndex[side][color], &images_[side][color][WHITE])) {
          SetErrorInfo(side, -1, color, GetLastError());
          return false;
        }

        CalcLines(&images_[side][color][WHITE], 0, CIS_IMAGE_WIDTH, whiteLine_[side][color]);
          
        double means = CalcMeans(whiteLine_[side][color], 
            leftMargin_, CIS_IMAGE_WIDTH - leftMargin_ - rightMargin_);

        if (means < standardExposureColor_[color]) {
          a = parameters_[side].exposureTime[cisColor] + 1;
        } else {
          b = parameters_[side].exposureTime[cisColor];
        }
		times++;
      }
	  TRACE(_T("Exposure Adjust Times: %d\n"), times);

      parameters_[side].exposureTime[cisColor] = a;
      if (!device_->TakeCISImage(parameters_, s_ColorImageIndex[side][color], &images_[side][color][WHITE])) {
        SetErrorInfo(side, -1, color, GetLastError());
        return false;
      }
      CalcLines(&images_[side][color][WHITE], 0, CIS_IMAGE_WIDTH, whiteLine_[side][color]);          
      double means = CalcMeans(whiteLine_[side][color], leftMargin_, CIS_IMAGE_WIDTH - leftMargin_ - rightMargin_);
      double difference = fabs(means - standardExposureColor_[color]);
      if (difference > acceptableExposureColorDifference_[color]) {
        SetErrorInfo(side, -1, color, ERROR_CIS_ADJUST_EXPOSURE);
        return false;
      }
    }
  }

  TRACE(_T("Running time of Exposure Adjust: %d\n"), GetTickCount() - startTime);

  TRACE(_T("Adjust Exposure Finished:[%d, %d, %d, %d]-[%d, %d, %d, %d]\n"), 
      parameters_[0].exposureTime[0], parameters_[0].exposureTime[1], parameters_[0].exposureTime[2], parameters_[0].exposureTime[3],
      parameters_[1].exposureTime[0], parameters_[1].exposureTime[1], parameters_[1].exposureTime[2], parameters_[1].exposureTime[3]);

  return true;
}

bool CISCalibrator::AdjustGain() {
  // 设置初始参数
  // exposure time: 校准过的参数
  // offset: 校准过的参数

	int startTime = GetTickCount();

  for (int side = 0; side < CIS_COUNT; side++) 
  {    
    parameters_[side].exposureTime[CIS_COLOR_RED] = parameters_[side].exposureTime[CIS_COLOR_RED];
    parameters_[side].exposureTime[CIS_COLOR_GREEN] = parameters_[side].exposureTime[CIS_COLOR_GREEN];
    parameters_[side].exposureTime[CIS_COLOR_BLUE] = parameters_[side].exposureTime[CIS_COLOR_BLUE];    
    parameters_[side].exposureTime[CIS_COLOR_IR] = parameters_[side].exposureTime[CIS_COLOR_IR];
    for (int segment = 0; segment < CIS_SEGMENT_COUNT; segment++) {
      parameters_[side].gain[segment] = minGain_;
      parameters_[side].offset[segment] = parameters_[side].offset[segment];
    }
  }

  // 为优化CIS校准时间而重写CIS校准代码
  int times = 0;
  for (int side = 0; side < CIS_COUNT; side++)
  {
	  int a[CIS_SEGMENT_COUNT] = { 0 };
	  int b[CIS_SEGMENT_COUNT] = { 0 };
	  for (int segment = 0; segment < CIS_SEGMENT_COUNT; segment++)
	  {
		  a[segment] = minGain_;
		  b[segment] = maxGain_;
	  }

	  while (1)
	  {
		  for (int color = 0; color < COLOR_COUNT; color++)
		  {
			  if (!device_->TakeCISImage(parameters_, s_ColorImageIndex[side][color], &images_[side][color][WHITE])) 
			  {
				  SetErrorInfo(side, 0, color, GetLastError());
				  return false;
			  }
			  times++;
		  }

		  int start = 0;
		  bool exit = true;
		  for (int segment = 0; segment < CIS_SEGMENT_COUNT; segment++)
		  {
			  // 该段的有效区间
			  int effectiveStart = (segment == 0 ? leftMargin_ : start);
			  int effectiveWidth = segmentWidth_[segment] -
				  (segment == 0 ? leftMargin_ : segment == CIS_SEGMENT_COUNT - 1 ? rightMargin_ : 0);

			  double maxMeans = 0;
			  for (int color = 0; color < COLOR_COUNT; color++)
			  {
				  CalcLines(&images_[side][color][WHITE], start, segmentWidth_[segment], whiteLine_[side][color]);

				  double means = CalcMeans(whiteLine_[side][color], effectiveStart, effectiveWidth);
				  
				  // 如果对于绿光，means大于standardWhite[Green]；对于IR，means小于standardWhite[IR]
				  // 则会出现gain校不过的现象，节后过来考虑如何改掉这个问题
				  if (means < standardWhite_[color])
				  {
					  a[segment] = parameters_[side].gain[segment] + 1;
				  }
				  else
				  {
					  b[segment] = parameters_[side].gain[segment];
				  }
			  }

			  if (a[segment] < b[segment])
			  {
				  parameters_[side].gain[segment] = (a[segment] + b[segment]) / 2;
				  exit = false;
			  }

			  // 进行下一段
			  start += segmentWidth_[segment];
		  }

		  if (exit)
		  {
			  break;
		  }
	  }

	  for (int color = 0; color < COLOR_COUNT; color++)
	  {
		  for (int segment = 0; segment < CIS_SEGMENT_COUNT; segment++)
		  {
			  parameters_[side].gain[segment] = a[segment];
		  }
		  if (!device_->TakeCISImage(parameters_, s_ColorImageIndex[side][color], &images_[side][color][WHITE])) 
		  {
			  SetErrorInfo(side, 0, color, GetLastError());
			  return false;
		  }
		  times++;
		  int start = 0;
		  for (int segment = 0; segment < CIS_SEGMENT_COUNT; segment++)
		  {
			  int effectiveStart = (segment == 0 ? leftMargin_ : start);
			  int effectiveWidth = segmentWidth_[segment] -
				  (segment == 0 ? leftMargin_ : segment == CIS_SEGMENT_COUNT - 1 ? rightMargin_ : 0);
			  CalcLines(&images_[side][color][WHITE], start, segmentWidth_[segment], whiteLine_[side][color]);			  
			  double means = CalcMeans(whiteLine_[side][color], effectiveStart, effectiveWidth);
			  if (means < standardWhite_[color] - acceptableWhiteDifference_[color]) 
			  {
				  SetErrorInfo(side, segment, color, ERROR_CIS_ADJUST_GAIN);
				  return false;
			  }
			  start += segmentWidth_[segment];
		  }
	  }
  }



 // for (int side = 0; side < CIS_COUNT; side++) {
 //   int start = 0;
 //   for (int segment = 0; segment < CIS_SEGMENT_COUNT; segment++) 
	//{
 //     // 该段的有效区间
 //     int effectiveStart = (segment == 0 ? leftMargin_ : start);
 //     int effectiveWidth = segmentWidth_[segment] - 
 //         (segment == 0 ? leftMargin_ : segment == CIS_SEGMENT_COUNT - 1 ? rightMargin_ : 0);

 //     int gain = minGain_;
 //     for (int color = 0; color < COLOR_COUNT; color++) {
 //       int cisColor = (color == COLOR_GREEN ? CIS_COLOR_GREEN : CIS_COLOR_IR);
 //       int a = minGain_, b = maxGain_;
	//	int times = 0;
 //       while (a < b) {
 //         parameters_[side].gain[segment] = (a + b) / 2;
 //       
 //         if (!device_->TakeCISImage(parameters_, s_ColorImageIndex[side][color], &images_[side][color][WHITE])) {
 //           SetErrorInfo(side, segment, color, GetLastError());
 //           return false;
 //         }

 //         CalcLines(&images_[side][color][WHITE], start, segmentWidth_[segment], whiteLine_[side][color]);
 //         
 //         double means = CalcMeans(whiteLine_[side][color], effectiveStart, effectiveWidth);

 //         if (means < standardWhite_[color]) {
 //           a = parameters_[side].gain[segment] + 1;
 //         } else {
 //           b = parameters_[side].gain[segment];
 //         }
	//	  times++;
 //       }

	//	TRACE(_T("Gain Adjust Times: %d\n"), times);

 //       parameters_[side].gain[segment] = a;
 //       if (!device_->TakeCISImage(parameters_, s_ColorImageIndex[side][color], &images_[side][color][WHITE])) {
 //         SetErrorInfo(side, segment, color, GetLastError());
 //         return false;
 //       }
 //       CalcLines(&images_[side][color][WHITE], start, segmentWidth_[segment], whiteLine_[side][color]);
 //       double means = CalcMeans(whiteLine_[side][color], effectiveStart, effectiveWidth);
 //       if (means < standardWhite_[color] - acceptableWhiteDifference_[color]) {
 //         SetErrorInfo(side, segment, color, ERROR_CIS_ADJUST_GAIN);
 //         return false;
 //       }

 //       if (gain < a) {
 //         gain = a;
 //       }
 //     }

 //     parameters_[side].gain[segment] = gain;

 //     start += segmentWidth_[segment];
 //   }

 //   // 获取校正完后的图像
 //   for (int color = 0; color < COLOR_COUNT; color++) {
 //     if (!device_->TakeCISImage(parameters_, s_ColorImageIndex[side][color], &images_[side][color][WHITE])) {
 //       SetErrorInfo(side, -1, color, GetLastError());
 //       return false;
 //     }
 //     CalcLines(&images_[side][color][WHITE], 0, CIS_IMAGE_WIDTH, whiteLine_[side][color]);
 //   }
 // }  



  TRACE(_T("Gain Adjust Times: %d\n"), times);
  TRACE(_T("Running time of Gain Adjust: %d\n"), GetTickCount() - startTime);

  TRACE(_T("Adjust Gain Finished:[%d, %d, %d]-[%d, %d, %d]\n"), 
      parameters_[0].gain[0], parameters_[0].gain[1], parameters_[0].gain[2],
      parameters_[1].gain[0], parameters_[1].gain[1], parameters_[1].gain[2]);

  return true;
}

bool CISCalibrator::TakeImages(int white) {
  LastErrorHolder errorHolder;

  for (int side = 0; side < CIS_COUNT; side++) {
    if (!device_->TakeCISImage(s_ColorImageIndex[side][COLOR_GREEN], &images_[side][COLOR_GREEN][white])) {
      errorHolder.SaveLastError();
      return false;
    }
    if (!device_->TakeCISImage(s_ColorImageIndex[side][COLOR_IR], &images_[side][COLOR_IR][white])) {
      errorHolder.SaveLastError();
      return false;
    }
  }

  return true;
}

int CISCalibrator::GetDustCount(int side, int color) const {
  return dustCount_[side][color];
}

int CISCalibrator::GetDustPosition(int side, int color, int index) const {
  ASSERT(index >= 0 && index < dustCount_[side][color]);
  return dustPositions_[side][color][index];
}

Bitmap *CISCalibrator::GetImage(int side, int color, int white) {
  return &images_[side][color][white];
}

bool CISCalibrator::Submit() {
  LastErrorHolder errorHolder;

  int startTime = GetTickCount();

  TRACE(_T("Upload CIS Parameters:\n"));
  TRACE(_T("Offset:[%d, %d, %d]-[%d, %d, %d]\n"), 
      parameters_[0].offset[0], parameters_[0].offset[1], parameters_[0].offset[2],
      parameters_[1].offset[0], parameters_[1].offset[1], parameters_[1].offset[2]);
  TRACE(_T("Gain:[%d, %d, %d]-[%d, %d, %d]\n"), 
      parameters_[0].gain[0], parameters_[0].gain[1], parameters_[0].gain[2],
      parameters_[1].gain[0], parameters_[1].gain[1], parameters_[1].gain[2]);
  TRACE(_T("Exposure:[%d, %d, %d, %d]-[%d, %d, %d, %d]\n"), 
      parameters_[0].exposureTime[0], parameters_[0].exposureTime[1], parameters_[0].exposureTime[2], parameters_[0].exposureTime[3],
      parameters_[1].exposureTime[0], parameters_[1].exposureTime[1], parameters_[1].exposureTime[2], parameters_[1].exposureTime[3]);

  if (!device_->UpdateCISParameter(parameters_)) {
    errorHolder.SaveLastError();
    return false;
  }
  if (!device_->UpdateCISCorrectionTable(&correctionTable_[0][0][0][0], sizeof(correctionTable_))) {
    errorHolder.SaveLastError();
    return false;
  }
  if (!device_->UpdateDebugState(DEBUG_CIS_CALIBRATION_DONE, DEBUG_CIS_CALIBRATION_DONE)) {
    errorHolder.SaveLastError();
    return false;
  }

  TRACE(_T("Running time of Submit: %d\n"), GetTickCount() - startTime);

  return true;
}

void CISCalibrator::CalcLines(Bitmap *image, int start, int width, LineInfo *lines) {
  const int stride = CIS_IMAGE_WIDTH;
  unsigned char *pixeBuffer = (unsigned char *)image->GetBits();  
  for (int x = start; x < start + width; x++) {
    lines[x].minGray = INT_MAX;
    lines[x].maxGray = INT_MIN;
    int sum = 0;
    unsigned char *p = pixeBuffer + x;
    for (int y = 0; y < CIS_IMAGE_HEIGHT; y++) {
      if (*p < lines[x].minGray) {
        lines[x].minGray = *p;
      }
      if (*p > lines[x].maxGray) {
        lines[x].maxGray = *p;
      }
      sum += *p;
      p += stride;
    }
    lines[x].avgGray = (double)sum / CIS_IMAGE_HEIGHT;
  }
}

double CISCalibrator::CalcMeans(const LineInfo *lines, int start, int width) {
  double sum = 0;
  for (int x = start; x < start + width; x++) {
    sum += lines[x].avgGray;
  }
  return sum / width;
}

double CISCalibrator::CalcDiff(const LineInfo *lines, int start, int width) {
  double min = 256;
  double max = 0;
  for (int x = start; x < start + width; x++) {
    if (lines[x].avgGray < min) {
      min = lines[x].avgGray;
    }
    if (lines[x].avgGray > max) {
      max = lines[x].avgGray;
    }
  }
  return max - min;
}

bool CISCalibrator::CheckStripExists() {
  Bitmap image;
  if (!DeviceProxy::GetInstance()->TakeCISImage(parameters_, s_ColorImageIndex[CIS_TOP][COLOR_GREEN], &image)) {
    return false;
  }

  int width = image.GetWidth();
  int height = image.GetHeight();
  ASSERT(width == CIS_IMAGE_WIDTH);
  unsigned char *p = (unsigned char *)image.GetBits();
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      *p = correctionTable_[CIS_TOP][COLOR_GREEN][x][*p];
      p++;
    }
  }

  LineInfo lines[CIS_IMAGE_WIDTH];
  CalcLines(&image, 0, CIS_IMAGE_WIDTH, lines);
  double means = CalcMeans(lines, 0, CIS_IMAGE_WIDTH);

  return means > standardWhite_[COLOR_GREEN] / 2;
}