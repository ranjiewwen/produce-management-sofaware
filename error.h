#pragma once

#include "resource.h"

#define MAKE_APP_ERROR(code)    (APPLICATION_ERROR_MASK | ERROR_SEVERITY_ERROR | (code))

#define ERROR_DEVICE_NOT_CONNECT            MAKE_APP_ERROR(IDS_DEVICE_NOT_CONNECT)
#define ERROR_DEVICE_ALREADY_CONNECTED      MAKE_APP_ERROR(IDS_DEVICE_ALREADY_CONNECTED)
#define ERROR_DEVICE_RESULT_ERROR           MAKE_APP_ERROR(IDS_DEVICE_RESULT_ERROR)
#define ERROR_NETWORK_UNAVAILABLE           MAKE_APP_ERROR(IDS_NETWORK_UNAVAILABLE)
#define ERROR_SERVER_DENIED                 MAKE_APP_ERROR(IDS_SERVER_DENIED)
#define ERROR_SERVER_UNAVAILABLE            MAKE_APP_ERROR(IDS_SERVER_UNAVAILABLE)
#define ERROR_SERVER_CONFIG                 MAKE_APP_ERROR(IDS_SERVER_CONFIG)
#define ERROR_XML_COMPONENT_DAMAGE          MAKE_APP_ERROR(IDS_XML_COMPONENT_DAMAGE)
#define ERROR_CONFIG_UNAVAILABLE            MAKE_APP_ERROR(IDS_CONFIG_UNAVAILABLE)
#define ERROR_GUID_UNAVAILABLE				MAKE_APP_ERROR(IDS_GUID_UNAVAILABLE)

#define ERROR_CIS_LOW_CONTRAST              MAKE_APP_ERROR(IDS_CIS_LOW_CONTRAST)
#define ERROR_CIS_ADJUST_OFFSET             MAKE_APP_ERROR(IDS_CIS_ADJUST_OFFSET)
#define ERROR_CIS_DITHER_Y                  MAKE_APP_ERROR(IDS_CIS_DITHER_Y)
#define ERROR_CIS_DITHER_X                  MAKE_APP_ERROR(IDS_CIS_DITHER_X)
#define ERROR_CIS_ADJUST_EXPOSURE           MAKE_APP_ERROR(IDS_CIS_ADJUST_EXPOSURE)
#define ERROR_CIS_ADJUST_GAIN               MAKE_APP_ERROR(IDS_CIS_ADJUST_GAIN)

class LastErrorHolder {
public:
  LastErrorHolder() {
    error_ = GetLastError();
  }
  LastErrorHolder(int error) {
    error_ = error;
  }
  ~LastErrorHolder() {
    SetLastError(error_);
  }
  operator int() const {
    return error_;
  }
  int GetError() const {
    return error_;
  }
  void SetError(int error) {
    error_ = error;
  }
  void SaveLastError() {
    error_ = GetLastError();
  }

private:
  int   error_;
};

CString FormatErrorMessage(int code);