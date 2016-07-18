#pragma once

class DeviceProxy;
class WorkingParameters;

enum {
  IR_STATUS_UNKNOWN,
  IR_STATUS_OK,
  IR_STATUS_ERROR,
};

struct IRState {  
  int   emissionValue;
  int   collectionValue[2];
  int   status[2];
};

class IRCalibration {
public:
  enum { IR_COUNT = 6 };

  enum { WAIT_TAKE_OUT_PAPER = 1 };

  IRCalibration();

  bool Init(DeviceProxy *device, WorkingParameters *parameters);
  bool AdjustWithPaper();
  bool VerifyNoPaper();
  bool Submit();
  const IRState *GetStates() const;

private:
  DeviceProxy *device_;
  IRState     states_[IR_COUNT];  
  int         minEmissionValue_;
  int         maxEmissionValue_;
  int         referenceCollectionValueWithPaper_;
  int         acceptableBiasValueWithPaper_;
  int         referenceCollectionValueNoPaper_;
  int         acceptableBiasValueNoPaper_;
};