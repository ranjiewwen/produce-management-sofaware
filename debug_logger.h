#pragma once

#include "singleton.h"

struct DeviceInfo;
class Cash;
struct IRState;
struct MagneticSignal;
struct CISParameter;
class Bitmap;
struct CISErrorInfo;

struct CISImageInfo {
  char    tag[40];
  Bitmap  *image;
};

class DebugLogger {
public:
  bool Init();
  void StartDebug();
  void Upgrade(LPCTSTR toVersion, int result);
  void CalibrateInfrared(int result, const IRState states[6]);  
  void DetectMagneticSignal(int result, int object, const MagneticSignal *signals);
  void CalibrateCIS(int step, const CISErrorInfo *error, const CISParameter *parameters, const CISImageInfo *images, int imageCount);
  bool SaveCash(Cash *cash);
  void EndDebug();

private:
  TCHAR   logPath_[MAX_PATH];
  CFile   logFile_;

  DebugLogger();
  virtual ~DebugLogger();

  bool WriteRecord(const char *tag, const void *data, int length);
  bool IsUploadDetail(int level, int result);

  DECLARE_SINGLETON(DebugLogger)
};