#include "stdafx.h"
#include "factory_server.h"
#include "curl\curl.h"
#include "error.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SINGLETON(FactoryServer)

size_t write_noop(const void *data, size_t size, size_t count, void *ctx) {
  return count;
}

bool FactoryServer::Login(LPCTSTR userName, LPCTSTR password) {
  CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
  if (ret != CURLE_OK) {
    SetLastError(ERROR_NETWORK);
    return false;
  }
  CURL *curl = curl_easy_init();
  if (curl == NULL) {
    SetLastError(ERROR_NETWORK);
    return false;
  }
  CString url = AfxGetApp()->GetProfileString(_T("Server"), _T("Address"), _T("ftp://127.0.0.1:21"));
  curl_easy_setopt(curl, CURLOPT_URL, static_cast<char *>(CT2A(url)));
  CStringA loginStr;
  loginStr.Format("%s:%s", 
    static_cast<char *>(CT2A(userName)),
    static_cast<char *>(CT2A(password)));
  curl_easy_setopt(curl, CURLOPT_USERPWD, static_cast<const char *>(loginStr));
  // we did not need a write function.
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_noop);
  ret = curl_easy_perform(curl);
  if (ret != CURLE_OK) {    
    curl_easy_cleanup(curl);
    int errorCode = ERROR_NETWORK;
    if (ret == CURLE_LOGIN_DENIED) {
      errorCode = ERROR_LOGIN_DENIED;
    }
    SetLastError(errorCode);
    return false;
  }
  curl_easy_cleanup(curl);

  return true;
}

bool FactoryServer::LoadWorkingParameters() {
  return true;
}

bool FactoryServer::UploadLogFile(LPCTSTR fileName) {
  return true;
}

void FactoryServer::Logout() {
  curl_global_cleanup();
}