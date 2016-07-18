
// application.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "application.h"
#include "login_dialog.h"
#include "main_dialog.h"
#include "gui_resources.h"
#include "working_parameters.h"
#include "device_proxy.h"
#include "device_proxy_inl.h"
#include "splash_screen.h"
#include "debug_logger.h"
#include "local_file_saver.h"
#include "input_name_key.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define APPID   _T("{673DA19F-482E-413F-8A09-46EEF44B39D6}")

// Application

BEGIN_MESSAGE_MAP(Application, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// Application 构造

Application::Application()
: skipUpdate_(false)
{
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 Application 对象

Application theApp;


// Application 初始化

BOOL Application::InitInstance()
{
  // 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

  HANDLE hMutex = CreateMutex(NULL, TRUE, APPID);
  if (hMutex == NULL) {
    AfxMessageBox(IDS_PROMPT_APP_INIT_FAILED, MB_ICONERROR | MB_OK);
    return FALSE;
  }
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    // application already running.
    return FALSE;
  }

	AfxEnableControlContainer();


	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO:  应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("ZMVision"));

  CoInitialize(NULL);

  if (!GuiResources::GetInstance()->Init()) {
    AfxMessageBox(IDS_PROMPT_APP_INIT_FAILED, MB_OK | MB_ICONERROR);
    return FALSE;
  }

  for (int i = 1; i < __argc; i++) {
    if (_tcscmp(__targv[i], _T("-skipupdate")) == 0) {
      skipUpdate_ = true;
    }
  }

  if (WorkingParameters::GetInstance()->IsFTPInfoBlank())
  {
	  CInputNameKey inputDlg;
	  if (inputDlg.DoModal() != IDOK)
	  {
		  return FALSE;
	  }
  }

  SplashScreen splash;
  if (splash.DoModal() != IDOK) {
    return FALSE;
  }

  LoginDialog login;
  if (login.DoModal() != IDOK) {
    return FALSE;
  }

  if (!DebugLogger::GetInstance()->Init()) {
    AfxMessageBox(IDS_PROMPT_APP_INIT_FAILED, MB_OK | MB_ICONERROR);
    return FALSE;
  }

	MainDialog dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO:  在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO:  在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
	}


	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}



int Application::ExitInstance() {
  DeviceProxy::ReleaseInstance();
  LocalFileSaver::ReleaseInstance();
  WorkingParameters::ReleaseInstance();
  GuiResources::ReleaseInstance();
  DebugLogger::ReleaseInstance();
  
  CoUninitialize();

  return CWinApp::ExitInstance();
}

bool Application::Upgrade(LPCTSTR updateFilePath, LPCTSTR version) {
  HINSTANCE hInstance = AfxGetResourceHandle();
  HRSRC hResInfo = FindResource(hInstance, MAKEINTRESOURCE(IDR_APP_UPDATE), _T("EXE"));
  if (hResInfo == NULL) {
    return false;
  }  
  HGLOBAL hResData = LoadResource(hInstance, hResInfo);
  if (hResData == NULL) {
    return false;
  }
  int size = SizeofResource(hInstance, hResInfo);  
  const void *data = LockResource(hResData);
  if (data == NULL) {
    return false;
  }
  TCHAR tempFilePath[MAX_PATH];
  GetTempPath(_countof(tempFilePath), tempFilePath);
  GetTempFileName(tempFilePath, _T("~"), 0, tempFilePath);
  PathRemoveExtension(tempFilePath);
  _tcscat(tempFilePath, _T(".exe"));
  TCHAR appDir[MAX_PATH];
  GetModuleFileName(NULL, appDir, _countof(appDir));
  PathRemoveFileSpec(appDir);
  try {
    CFile file(tempFilePath, CFile::modeCreate | CFile::typeBinary | CFile::modeWrite);
    file.Write(data, size);
    file.Close();
  } catch (CException *e) {
    e->Delete();
    return false;
  }
  CString parameters;
  parameters.AppendChar(_T('\"'));
  parameters.Append(updateFilePath);
  parameters.AppendChar(_T('\"'));
  parameters.AppendChar(_T(' '));
  parameters.AppendChar(_T('\"'));
  parameters.Append(appDir);
  parameters.AppendChar(_T('\"'));
  parameters.AppendChar(_T(' '));
  parameters.AppendChar(_T('\"'));
  parameters.Append(version);
  parameters.AppendChar(_T('\"'));

  HINSTANCE instance = ShellExecute(NULL, NULL, tempFilePath, parameters, NULL, SW_SHOW); 
  if ((DWORD)instance <= 32) {
    return false;
  }
  return true;
}

