#include "stdafx.h"
#include "common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CreateDirectoryRecusively(LPCTSTR path) {
  int ret = SHCreateDirectory(NULL, path);
  if (ret != ERROR_SUCCESS && ret != ERROR_ALREADY_EXISTS) {
    SetLastError(ret);
    return false;
  }
  return true;
}

bool GetAppDataPath(LPTSTR path) {
  if (!SHGetSpecialFolderPath(NULL, path, CSIDL_LOCAL_APPDATA, TRUE)) {
    return false;
  }
  PathAddBackslash(path);
  _tcscat(path, _T("ZMVision\\ccmdbg"));
  if (!PathFileExists(path)) {
    if (!CreateDirectoryRecusively(path)) {
      return false;
    }
  }
  return true;
}