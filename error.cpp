#include "stdafx.h"
#include "error.h"
#include "resource.h"

#ifdef _DEBUG
#define new DBEUG_NEW
#endif

CString FormatErrorMessage(int code) {
  CString message;
  if (code & APPLICATION_ERROR_MASK) {
    VERIFY(message.LoadString(code & 0xFFFF));
  } else {
    LPTSTR buffer = NULL;
    if (::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
        , NULL, code, LANG_USER_DEFAULT, (LPTSTR)&buffer, 0, NULL) != 0) {
      message.SetString(buffer);
      ::LocalFree(buffer);
    } else {
      message.Format(IDS_UNKNOWN_ERROR, code);    
    }
  }
  return message;
}