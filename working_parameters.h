#pragma once

#include <vector>
#include "singleton.h"

// class ParameterBlock
class ParameterBlock {
public:
  const static ParameterBlock Null;

  ParameterBlock();
  ParameterBlock(IXMLDOMElement *e);
  ParameterBlock(const ParameterBlock &other);
  virtual ~ParameterBlock();  

  void Reset(IXMLDOMElement *e);
  ParameterBlock &operator=(const ParameterBlock &other);

  CString GetName() const;
  int GetIntParameter(LPCTSTR name, int defaultValue);
  double GetFloatParameter(LPCTSTR name, double defaultValue);
  CString GetStringParameter(LPCTSTR name, LPCTSTR defaultValue);
  ParameterBlock SelectBlock(LPCTSTR path);
  ParameterBlock GetFirstChildBlock();
  ParameterBlock GetNextBlock();
  bool IsNull() const;
    
private:  
  IXMLDOMElement  *e_;
};

// ø…±‰≈‰÷√ lux
class VariableConfigBlock 
{
public:
	const static VariableConfigBlock Null;

	VariableConfigBlock();
	VariableConfigBlock(LPCTSTR path);
	VariableConfigBlock(const VariableConfigBlock &other);
	virtual ~VariableConfigBlock();
	VariableConfigBlock &operator=(const VariableConfigBlock &other);

	CString GetCurrentMode();
	int GetIntParameter(LPCTSTR name, int defaultValue);
	void SetIntParemeter(LPCTSTR name, int choiceValue);
	CString GetStringParameter(LPCTSTR name, LPCTSTR defaultValue);
	bool IsNull() const;

private:
	CString configPath_;
};

// class WorkingParameters
class WorkingParameters : public ParameterBlock {
public:
  bool LoadFromServer();
  bool LoadConfigFromServer();
  bool LoadXML();
  bool LoadConfig();

  bool SetCurrentModel(int id);
  ParameterBlock *GetCurrentModelParameters();
  VariableConfigBlock *GetCurrentConfigParameters();

  int Login(LPCTSTR userName, LPCTSTR password);
  const CString &GetUserName() { return userName_; }

  bool IsAppUpdated() const { return appUpdated_; }
  const CString &GetAppUpdateFilePath() const { return appUpdateFilePath_; }
  const CString &GetLastAppVersion() const { return lastAppVersion_; }

  void SetOffline(bool offline) { offline_ = offline; }
  bool IsOffline() const { return offline_; }

  int GetCurrentModelId() const { return currentModelId_; }
  int GetCurrentPermission() const { return currentPermission_; }

  bool IsFTPInfoBlank();
  bool SetFTPInfo(LPCTSTR userName, LPCTSTR password);

private:
  CComPtr<IXMLDOMDocument2> xmlDoc_;
  int                       currentModelId_;
  ParameterBlock            currentModelParameters_;
  VariableConfigBlock		currentConfigParameters_;
  CString                   userName_;
  bool                      appUpdated_;
  CString                   appUpdateFilePath_;
  CString                   lastAppVersion_;
  bool                      offline_;
  int						currentPermission_;

  WorkingParameters();
  virtual ~WorkingParameters();

  DECLARE_SINGLETON(WorkingParameters)
};