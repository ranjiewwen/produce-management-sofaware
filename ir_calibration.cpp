#include "stdafx.h"
#include "ir_calibration.h"
#include "device_proxy.h"
#include "device_proxy_inl.h"
#include "working_parameters.h"
#include "working_parameters_inl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IRCalibration::IRCalibration() {
  ZeroMemory(states_, sizeof(states_));
}

bool IRCalibration::AdjustWithPaper() {
  int emissionValuesA[IR_COUNT];
  int emissionValuesB[IR_COUNT];
  int emissionValues[IR_COUNT];
  int collectionValues[IR_COUNT];

  ZeroMemory(states_, sizeof(states_));

  for (int i = 0; i < IR_COUNT; i++) {
    emissionValuesA[i] = minEmissionValue_;
    emissionValuesB[i] = maxEmissionValue_;
  }

  for (;;) {
    bool allDone = true;
    for (int i = 0; i < IR_COUNT; i++) {
      if (emissionValuesA[i] < emissionValuesB[i]) {
        emissionValues[i] = (emissionValuesA[i] + emissionValuesB[i]) / 2;
        allDone = false;
      }
    }
    if (allDone) {
      break;
    }
    if (!device_->GetIRValues(emissionValues, collectionValues)) {
      return false;
    }

    TRACE(_T("IRCalibration: [%d,%d,%d,%d,%d,%d]->[%d,%d,%d,%d,%d,%d]\n"), 
        emissionValues[0], emissionValues[1], 
        emissionValues[2], emissionValues[3],
        emissionValues[4], emissionValues[5],
        collectionValues[0], collectionValues[1], 
        collectionValues[2], collectionValues[3],
        collectionValues[4], collectionValues[5]);

    for (int i = 0; i < IR_COUNT; i++) {
      if (collectionValues[i] < referenceCollectionValueWithPaper_) {
        emissionValuesA[i] = emissionValues[i] + 1;
      } else {
        emissionValuesB[i] = emissionValues[i];
      }
    }
  }

  if (!device_->GetIRValues(emissionValues, collectionValues)) {
    return false;
  }

  bool success = true;
  for (int i = 0; i < IR_COUNT; i++) {
    states_[i].emissionValue = emissionValues[i];
    int bais = abs(collectionValues[i] - referenceCollectionValueWithPaper_);
    if (bais > acceptableBiasValueWithPaper_) {
      states_[i].status[0] = IR_STATUS_ERROR;
      success = false;
    } else {
      states_[i].status[0] = IR_STATUS_OK;
    }
    states_[i].collectionValue[0] = collectionValues[i];
  }

  return success;
}

bool IRCalibration::VerifyNoPaper() {
  int i;
  int emissionValues[IR_COUNT];
  int collectionValues[IR_COUNT];  

  for (i = 0; i < IR_COUNT; i++) {
    emissionValues[i] = states_[i].emissionValue;
  }

  if (!device_->GetIRValues(emissionValues, collectionValues)) {
    return false;
  }

  bool success = true;

  for (i = 0; i < IR_COUNT; i++) {
    int bais = abs(collectionValues[i] - referenceCollectionValueNoPaper_);
    if (bais > acceptableBiasValueNoPaper_) {
      states_[i].status[1] = IR_STATUS_ERROR;
      success = false;
    } else {
      states_[i].status[1] = IR_STATUS_OK;
    }
    states_[i].collectionValue[1] = collectionValues[i];
  }

  return success;
}

bool IRCalibration::Submit() {
  int emissionValues[IR_COUNT];
  for (int i = 0; i < IR_COUNT; i++) {
    emissionValues[i] = states_[i].emissionValue;
  }
  if (!device_->UpdateIRParameters(emissionValues)) {
    return false;
  }
  if (!device_->UpdateDebugState(DEBUG_IR_CALIBRATION_DONE, DEBUG_IR_CALIBRATION_DONE)) {
    return false;
  }
  return true;
}

bool IRCalibration::Init(DeviceProxy *device, WorkingParameters *parameters) {
  ParameterBlock *model = parameters->GetCurrentModelParameters();
  ASSERT(model != NULL && !model->IsNull());
  ParameterBlock block = model->SelectBlock(_T("Config\\IRCalibration"));
  minEmissionValue_ = block.GetIntParameter(_T("minEmissionValue"), 0);
  maxEmissionValue_ = block.GetIntParameter(_T("maxEmissionValue"), 5000);
  referenceCollectionValueWithPaper_ = block.GetIntParameter(_T("referenceCollectionValueWithPaper"), 1100);
  acceptableBiasValueWithPaper_ = block.GetIntParameter(_T("acceptableBiasValueWithPaper"), 100);
  referenceCollectionValueNoPaper_ = block.GetIntParameter(_T("referenceCollectionValueNoPaper"), 5000);
  acceptableBiasValueNoPaper_ = block.GetIntParameter(_T("acceptableBiasValueNoPaper"), 100);

  for (int i = 0; i < IR_COUNT; i++) {
    states_[i].emissionValue = (minEmissionValue_ + maxEmissionValue_) / 2;
    states_[i].collectionValue[0] = 0;
    states_[i].collectionValue[1] = 0;
    states_[i].status[0] = IR_STATUS_UNKNOWN;
    states_[i].status[1] = IR_STATUS_UNKNOWN;
  }

  device_ = device;

  return true;
}

const IRState *IRCalibration::GetStates() const {
  return states_;
}