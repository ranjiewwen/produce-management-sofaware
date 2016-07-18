#pragma once

#include "singleton.h"

class FactoryServer {
public:
  bool Login(LPCTSTR userName, LPCTSTR password);
  bool LoadWorkingParameters();
  bool UploadLogFile(LPCTSTR fileName);
  void Logout();

private:
  DECLARE_SINGLETON(FactoryServer)
};