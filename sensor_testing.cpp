#include "stdafx.h"
#include "sensor_testing.h"
#include "working_parameters.h"
#include "working_parameters_inl.h"
#include "magnetic\BMHD_AmountMatrixParameter.h"
#include "magnetic\RMBresult_check.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SensorTesting::SensorTesting() {
  dataBuffer_ = new Tdn_DataBuf;
}

SensorTesting::~SensorTesting() {
  delete dataBuffer_;
}

bool SensorTesting::Init(WorkingParameters *parameters) {
  ParameterBlock *model = parameters->GetCurrentModelParameters();
  ASSERT(model != NULL && !model->IsNull());
  ParameterBlock magneticBlock = model->SelectBlock(_T("Config\\SensorTesting\\Magnetic"));
  whitePaperMagneticMeansMin_ = magneticBlock.GetIntParameter(_T("whitePaperMeansMin"), 1500);
  whitePaperMagneticMeansMax_ = magneticBlock.GetIntParameter(_T("whitePaperMeansMax"), 1800);
  whitePaperMagneticVarianceMin_ = magneticBlock.GetIntParameter(_T("whitePaperVarianceMin"), 100);
  whitePaperMagneticVarianceMax_ =  magneticBlock.GetIntParameter(_T("whitePaperVarianceMax"), 40000);
  magneticMinLength_ =  magneticBlock.GetIntParameter(_T("minLength"), 1000);

  ParameterBlock wheelBlock = model->SelectBlock(_T("Config\\SensorTesting\\CodeWheel"));
  codeWheelStepMin_ = wheelBlock.GetIntParameter(_T("stepMin"), 5);
  codeWheelStepMax_ = wheelBlock.GetIntParameter(_T("stepMax"), 14);

  return true;
}

bool SensorTesting::TestWhitePaperMagnetic(const MagneticSignal signals[MAGNETIC_HEADER_COUNT], int results[MAGNETIC_HEADER_COUNT]) {
  int i, j;
  int passed = 0;

  for (i = 0; i < MAGNETIC_HEADER_COUNT; i++) {
    results[i] = 1;

    if (signals[i].length < magneticMinLength_) {      
      continue;
    }

    double sum = 0.0;
    for (j = 0; j < signals[i].length; j++) {
      sum += signals[i].values[j];
    }    
    double means = sum / signals[i].length;
    if (means < whitePaperMagneticMeansMin_ ||
        means > whitePaperMagneticMeansMax_) {
      continue;
    }
    
    sum = 0.0;
    for (int j = 0; j < signals[i].length; j++) {
      double diff = signals[i].values[j] - means;
      sum += diff * diff;
    }
    double variance = sum / signals[i].length;
    if (variance < whitePaperMagneticVarianceMin_ ||
        variance > whitePaperMagneticVarianceMax_) {
      continue;
    }

    results[i] = 0;
    passed++;
  }

  return passed == MAGNETIC_HEADER_COUNT;
}

bool SensorTesting::TestRMB100V05Magnetic(const MagneticSignal signals[MAGNETIC_HEADER_COUNT], int result[MAGNETIC_HEADER_COUNT], int *side) {
  /*MAGNETIC_HEADER_HD,
  MAGNETIC_HEADER_B,
  MAGNETIC_HEADER_L,
  MAGNETIC_HEADER_R,
  MAGNETIC_HEADER_LS,
  MAGNETIC_HEADER_RS,*/
  int *valuePtr[] = {
    dataBuffer_->HD_V,
    dataBuffer_->BM_V,
    dataBuffer_->LM_V,
    dataBuffer_->RM_V,
    dataBuffer_->LSM_V,
    dataBuffer_->RSM_V,
  };
  int *codePtr[] = {
    dataBuffer_->HD_W,
    dataBuffer_->BM_W,
    dataBuffer_->LM_W,
    dataBuffer_->RM_W,
    dataBuffer_->LSM_W,
    dataBuffer_->RSM_W,
  };
  int *countPtr[] = {
    &dataBuffer_->HD_Count,
    &dataBuffer_->BM_Count,
    &dataBuffer_->LM_Count,
    &dataBuffer_->RM_Count,
    &dataBuffer_->LSM_Count,
    &dataBuffer_->RSM_Count,
  };
  for (int i = 0; i < MAGNETIC_HEADER_COUNT; i++) {
    *countPtr[i] = signals[i].length;
    ASSERT(signals[i].length < 5 * WAVELENGTH);
    for (int j = 0; j < signals[i].length; j++) {
      valuePtr[i][j] = signals[i].values[j];
      codePtr[i][j] = signals[i].codes[j];
    }
  }

  Result_RMB rmbResult;
  ZeroMemory(&rmbResult, sizeof(rmbResult));
  int error[7] = { 0 };
  RMBresult_check(dataBuffer_, 1, NULL, error, &rmbResult);
  result[MAGNETIC_HEADER_B] = error[POSITION_BM];
  result[MAGNETIC_HEADER_HD] = error[POSITION_HD];
  result[MAGNETIC_HEADER_LS] = error[POSITION_LSM];
  result[MAGNETIC_HEADER_RS] = error[POSITION_RSM];
  result[MAGNETIC_HEADER_L] = error[POSITION_LM];
  result[MAGNETIC_HEADER_R] = error[POSITION_RM];
  
  *side = rmbResult.Direction;

  return rmbResult.ERRtype == 0;
}

bool SensorTesting::TestCodeWheel(const short *codes, int length) {
  int start = 1, end = length - 1;

  while (start < length) {
    if (codes[start] != codes[start - 1]) {
      break;
    }
    start++;
  }

  while (end > 0) {
    if (codes[end] != codes[end - 1]) {
      break;
    }
    end--;
  }

  int step = 0;
  for (int i = start; i < end; i++) {
    if (codes[i] == codes[i - 1]) {
      step++;
    } else {
      if (step < codeWheelStepMin_ || step > codeWheelStepMax_) {
        return false;
      }
      step = 0;
    }
  }
  if (step < codeWheelStepMin_ || step > codeWheelStepMax_) {
    return false;
  }
  return true;
}