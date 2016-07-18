// page_upgrade.cpp : implementation file
//

#include "stdafx.h"
#include "page_upgrade.h"
#include "thread_inl.h"
#include "device_proxy.h"
#include "working_parameters.h"
#include "working_parameters_inl.h"
#include "main_dialog.h"
#include "progress_tracker_inl.h"
#include "common.h"
#include "debug_logger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_UPGRADE_PROGRESS         WM_USER + 100
#define WM_UPGRADE_STEP             WM_USER + 101
#define WM_UPGRADE_STEP_COMPLETED   WM_USER + 102
#define WM_UPGRADE_COMPLETED        WM_USER + 103

#define UPGRADE_UPLOAD              0
#define UPGRADE_UPDATE_STATE        1
#define UPGRADE_RESTART             2

// PageUpgrade dialog
IMPLEMENT_DYNAMIC(PageUpgrade, Page)

PageUpgrade::PageUpgrade(CWnd* pParent /*=NULL*/)
	: Page(PageUpgrade::IDD, pParent) 
{
}

PageUpgrade::~PageUpgrade() {
}

void PageUpgrade::DoDataExchange(CDataExchange* pDX) {
	Page::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_PROGRESS, progressBar_);
  DDX_Control(pDX, IDC_LIST_INFO, taskListView_);
}

void PageUpgrade::OnEnter(int leavePageId) {
  for (int i = 0; i < taskListView_.GetCount(); i++) {
    taskListView_.SetState(i, -1);
  }
  
  progressBar_.SetPos(0);

  SetHeaderInfo(HEADER_STYLE_INFO, IDS_PROMPT_UPGRADE, 0);
  SetHeaderButtonText(IDC_BUTTON_START, IDS_BUTTON_START);
}

bool PageUpgrade::OnLeave(int enterPageId) {
  if (enterPageId != IDD_PAGE_CONNECT) {
    if (upgradeThread_.IsAlive()) {
      return false;
    }
  }
  return true;
}

BEGIN_MESSAGE_MAP(PageUpgrade, Page)
  ON_WM_SIZE()
  ON_BN_CLICKED(IDC_BUTTON_START, &PageUpgrade::OnBnClickedButtonStart)
  ON_MESSAGE(WM_UPGRADE_PROGRESS, &PageUpgrade::OnUpgradeProgress)
  ON_MESSAGE(WM_UPGRADE_STEP, &PageUpgrade::OnUpgradeStep)
  ON_MESSAGE(WM_UPGRADE_STEP_COMPLETED, &PageUpgrade::OnUpgradeStepCompleted)
  ON_MESSAGE(WM_UPGRADE_COMPLETED, &PageUpgrade::OnUpgradeCompleted)
END_MESSAGE_MAP()

// PageUpgrade message handlers
BOOL PageUpgrade::OnInitDialog() {
  Page::OnInitDialog();

  CBitmap bitmap;
  bitmap.LoadBitmap(IDB_STATUS);
  imageList_.Create(32, 32, ILC_COLOR32 | ILC_MASK, 8, 0);
  imageList_.Add(&bitmap, RGB(255, 0, 255));
  taskListView_.SetImageList(&imageList_);
  taskListView_.SetSpacing(32);

  layout_.Init(m_hWnd);
  layout_.AddDlgItem(IDC_PROGRESS, AnchorLayout::TOP_LEFT, AnchorLayout::TOP_RIGHT);
  layout_.AddDlgItem(IDC_LIST_INFO, AnchorLayout::TOP_LEFT, AnchorLayout::BOTTOM_RIGHT);

  progressTracker_.AttachWindow(m_hWnd);
  progressTracker_.SetRange(0, 100);
  progressBar_.SetRange(0, 100);
  progressBar_.SetPos(0);

  CString text;
  text.LoadString(IDS_UPGRADE_UPLOAD);
  taskListView_.AddItem(text, 4, -1);
  text.LoadString(IDS_UPGRADE_UPDATE_STATE);
  taskListView_.AddItem(text, 5, -1);
  text.LoadString(IDS_UPGRADE_RESTART);
  taskListView_.AddItem(text, 6, -1);

  HeaderButton buttons = { IDC_BUTTON_START, IDS_BUTTON_START };
  SetHeaderButtons(&buttons, 1);
  ShowHeader(true);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void PageUpgrade::OnSize(UINT nType, int cx, int cy) {
  Page::OnSize(nType, cx, cy);

  if (::IsWindow(m_hWnd)) {
    layout_.RecalcLayout();
  }
}

void PageUpgrade::OnBnClickedButtonStart() {
  if (upgradeThread_.IsAlive()) {
    return;
  }

  EnableConnectionCheck(false);

  SetHeaderInfo(HEADER_STYLE_INFO, IDS_PROMPT_UPGADE_PROCESSING, 0);

  upgradeThread_.Start(new MethodRunnable<PageUpgrade>(this, &PageUpgrade::UpgradeAsync));
}

PageUpgrade::UpgradeProgressTracker::UpgradeProgressTracker()
  : hWnd_(NULL) {
}
void PageUpgrade::UpgradeProgressTracker::AttachWindow(HWND hWnd) {
  hWnd_ = hWnd;
}
void PageUpgrade::UpgradeProgressTracker::UpdateProgress(int pos) {
  if (hWnd_ != NULL) {
    ::PostMessage(hWnd_, WM_UPGRADE_PROGRESS, pos, 0);
  }
}

DWORD PageUpgrade::UpgradeAsync() {
  // upload upgrade file.
  DeviceProxy *proxy = DeviceProxy::GetInstance();
  ParameterBlock *modelBlock = WorkingParameters::GetInstance()->GetCurrentModelParameters();
  ASSERT(modelBlock != NULL && !modelBlock->IsNull());
  ParameterBlock firmwareBlock = modelBlock->SelectBlock(_T("Firmware"));
  CString firmwareVersion = firmwareBlock.GetStringParameter(_T("version"), NULL);
  TCHAR appDataPath[MAX_PATH];
  GetAppDataPath(appDataPath);
  PathAddBackslash(appDataPath);
  _tcscat(appDataPath, _T("config\\"));
  CString firmwareFilePath = appDataPath;
  firmwareFilePath.AppendFormat(_T("firmware_%d.dat"), modelBlock->GetIntParameter(_T("id"), -1));

  ::PostMessage(m_hWnd, WM_UPGRADE_STEP, UPGRADE_UPLOAD, 0);
  if (!proxy->Upgrade(firmwareFilePath, firmwareVersion, &progressTracker_)) {
    int error = GetLastError();
    
    DebugLogger::GetInstance()->Upgrade(firmwareVersion, error);

    ::PostMessage(m_hWnd, WM_UPGRADE_STEP_COMPLETED, UPGRADE_UPLOAD, FALSE);
    ::PostMessage(m_hWnd, WM_UPGRADE_COMPLETED, FALSE, error);

    return -1;
  } else {
    DebugLogger::GetInstance()->Upgrade(firmwareVersion, 0);
  }

  ::PostMessage(m_hWnd, WM_UPGRADE_STEP_COMPLETED, UPGRADE_UPLOAD, TRUE);  
  // update debug state.
  ::PostMessage(m_hWnd, WM_UPGRADE_STEP, UPGRADE_UPDATE_STATE, 0);
  if (!proxy->UpdateDebugState(DEBUG_UPGRADE_DONE, 0)) {    
    int error = GetLastError();
    ::PostMessage(m_hWnd, WM_UPGRADE_STEP_COMPLETED, UPGRADE_UPDATE_STATE, FALSE);
    ::PostMessage(m_hWnd, WM_UPGRADE_COMPLETED, FALSE, error);
    return -1;
  }
  ::PostMessage(m_hWnd, WM_UPGRADE_STEP_COMPLETED, UPGRADE_UPDATE_STATE, TRUE);
  // restart device.
  ::PostMessage(m_hWnd, WM_UPGRADE_STEP, UPGRADE_RESTART, 0);
  if (!proxy->Restart()) {
    int error = GetLastError();
    ::PostMessage(m_hWnd, WM_UPGRADE_STEP_COMPLETED, UPGRADE_RESTART, FALSE);
    ::PostMessage(m_hWnd, WM_UPGRADE_COMPLETED, FALSE, error);
    return -1;
  }
  // wait for restart.
  while (1) {
    if (!proxy->Echo()) {
      // device is shutdown.
      break;
    }
    Sleep(500);
  }
  
  Sleep(2000);

  ::PostMessage(m_hWnd, WM_UPGRADE_STEP_COMPLETED, UPGRADE_RESTART, TRUE);
  ::PostMessage(m_hWnd, WM_UPGRADE_COMPLETED, TRUE, 0);

  proxy->Disconnect();

  //CString deviceIP = parameters->GetDeviceIP();
  //int devicePort = parameters->GetDevicePort();
  //while (!upgradeThread_.IsInterrupted()) {
  //  // device is started.
  //  if (proxy->Connect(deviceIP, devicePort)) {
  //    break;
  //  }
  //  Sleep(500);
  //}

  //::PostMessage(m_hWnd, WM_UPGRADE_STEP_COMPLETED, UPGRADE_RESTART, TRUE);
  //::PostMessage(m_hWnd, WM_UPGRADE_COMPLETED, TRUE, 0);

  return 0;
}

LRESULT PageUpgrade::OnUpgradeCompleted(WPARAM wParam, LPARAM lParam) {
  upgradeThread_.WaitForExit(INFINITE);

  if (!(BOOL)wParam) {
    int error = (int)lParam;
    
    SetHeaderInfo(HEADER_STYLE_ERROR, IDS_PROMPT_UPGADE_FAILED, 0);
    SetHeaderButtonText(IDC_BUTTON_START, IDS_BUTTON_RETRY);
  } else {
    //GotoStep(STEP_IR_CALIBRATION);
  }

  EnableConnectionCheck(true);

  return 0;
}

LRESULT PageUpgrade::OnUpgradeStep(WPARAM wParam, LPARAM lParam) {  
  taskListView_.SetState((int)wParam, 3);

  return 0;
}

LRESULT PageUpgrade::OnUpgradeStepCompleted(WPARAM wParam, LPARAM lParam) {
  int step = (int)wParam;
  BOOL success = (BOOL)lParam;
  taskListView_.SetState(step, success ? 1 : 2);
  return 0;
}

LRESULT PageUpgrade::OnUpgradeProgress(WPARAM wParam, LPARAM lParam) {
  progressBar_.SetPos((int)wParam);
  return 0;
}