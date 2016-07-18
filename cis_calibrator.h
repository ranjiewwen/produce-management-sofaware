#pragma once

#include "bitmap.h"
#include "device_proxy.h"

class WorkingParameters;

struct CISErrorInfo {
  int result;
  int side;
  int segment;
  int color;  
};

class CISCalibrator {
public:
  enum {
    BLACK,
    WHITE,
  };
  enum {
    BOTTOM,
    TOP,
  };
  
  CISCalibrator();
  virtual ~CISCalibrator();

  bool Init(DeviceProxy *device, WorkingParameters *parameters);
  bool AdjustExposure();
  bool AdjustGain();
  bool Correct();
  bool AdjustOffset();
  bool CheckUniformity();
  bool TakeImages(int white);
  const CISErrorInfo *GetErrorInfo() const { return &errorInfo_; }
  int GetDustCount(int side, int color) const;
  int GetDustPosition(int side, int color, int index) const;  
  Bitmap *GetImage(int side, int color, int white);
  bool Submit();
  bool CheckStripExists();

  const CISParameter *GetParameters() const { return parameters_; }

private:
  struct LineInfo {
    int minGray;
    int maxGray;
    double avgGray;
  };

  static int      s_ColorImageIndex[CIS_COUNT][COLOR_COUNT];
  DeviceProxy     *device_;
  int             dustPositions_[CIS_COUNT][COLOR_COUNT][CIS_IMAGE_WIDTH];
  int             dustCount_[CIS_COUNT][COLOR_COUNT];
  CISErrorInfo    errorInfo_;
  Bitmap          images_[CIS_COUNT][COLOR_COUNT][2];
  unsigned char   correctionTable_[CIS_COUNT][COLOR_COUNT][CIS_IMAGE_WIDTH][256];
  LineInfo        whiteLine_[CIS_COUNT][COLOR_COUNT][CIS_IMAGE_WIDTH];
  LineInfo        blackLine_[CIS_COUNT][COLOR_COUNT][CIS_IMAGE_WIDTH];  
  CISParameter    parameters_[CIS_COUNT];

  int             minOffset_;
  int             maxOffset_;
  double          offsetBaseGray_;
  int             minGain_;
  int             maxGain_;
  int             leftMargin_;
  int             rightMargin_;
  int             minExposureTime_[CIS_COLOR_COUNT];
  int             maxExposureTime_[CIS_COLOR_COUNT];
  int             standardExposureColor_[COLOR_COUNT];
  int             acceptableExposureColorDifference_[COLOR_COUNT];
  int             standardWhite_[COLOR_COUNT];
  int             acceptableWhiteDifference_[COLOR_COUNT];
  int             segmentWidth_[CIS_SEGMENT_COUNT];
  int             acceptableGrayLevel_;
  int             acceptableDitherX_;
  int             acceptableDitherY_;

  bool CheckUniformity(const LineInfo *lines, int start, int width);
  void SetErrorInfo(int side, int segment, int color, int code);

  static void CalcLines(Bitmap *image, int start, int width, LineInfo *lines);
  static double CalcMeans(const LineInfo *lines, int start, int width);
  static double CalcDiff(const LineInfo *lines, int start, int width);
};