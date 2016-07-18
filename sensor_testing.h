#pragma once

class WorkingParameters;

enum {
  MAGNETIC_HEADER_HD,
  MAGNETIC_HEADER_B,
  MAGNETIC_HEADER_L,
  MAGNETIC_HEADER_R,
  MAGNETIC_HEADER_LS,
  MAGNETIC_HEADER_RS,
  MAGNETIC_HEADER_COUNT
};

struct MagneticSignal {
  const short *values;
  const short *codes;
  int length;
};

typedef struct tagTdn_DataBuf Tdn_DataBuf;

class SensorTesting {
public:
  SensorTesting();
  virtual ~SensorTesting();

  bool Init(WorkingParameters *parameters);
  bool TestWhitePaperMagnetic(const MagneticSignal signals[MAGNETIC_HEADER_COUNT], int results[MAGNETIC_HEADER_COUNT]);
  bool TestRMB100V05Magnetic(const MagneticSignal signals[MAGNETIC_HEADER_COUNT], int results[MAGNETIC_HEADER_COUNT], int *side);
  bool TestCodeWheel(const short *codes, int length);

private:
  int         magneticMinLength_;
  double      whitePaperMagneticMeansMin_;
  double      whitePaperMagneticMeansMax_;
  double      whitePaperMagneticVarianceMin_;
  double      whitePaperMagneticVarianceMax_;
  int         codeWheelStepMin_;
  int         codeWheelStepMax_;
  Tdn_DataBuf *dataBuffer_;
};