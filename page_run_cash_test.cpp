// page_run_cash_test.cpp : implementation file
//

#include "stdafx.h"
#include "page_run_cash_test.h"
#include "run_cash_monitor.h"
#include "run_cash_monitor_inl.h"
#include "device_proxy.h"
#include "device_proxy_inl.h"
#include "main_dialog.h"
#include "observer_inl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_CASH_INFO  WM_USER + 101

// PageRunCashTest dialog

IMPLEMENT_DYNAMIC(PageRunCashTest, CDialog)

PageRunCashTest::PageRunCashTest(CWnd* pParent /*=NULL*/)
	: Page(PageRunCashTest::IDD, pParent)
  , monitor_(NULL) {
}

PageRunCashTest::~PageRunCashTest() {
}

void PageRunCashTest::DoDataExchange(CDataExchange* pDX) {
	Page::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_LIST_CHECK_POINTS, checkPointList_);
}

#define ANY   -5

struct CheckPoint {
  TCHAR desc[256];
  int denomination;
  int version;
  int direction;
  int targetCount;
  int count;
};

static CheckPoint s_CheckPoints[] = {
  { _T("05版100元[面向1]1张"), 100, 05, 1, 1, 0 },
  { _T("05版100元[面向2]1张"), 100, 05, 2, 1, 0 },
  { _T("05版100元[面向3]1张"), 100, 05, 3, 1, 0 },
  { _T("05版100元[面向4]1张"), 100, 05, 4, 1, 0 },
};

void PageRunCashTest::OnEnter(int leavePageId) {
  checkPointList_.DeleteAllItems();

  for (int i = 0; i < _countof(s_CheckPoints); i++) {
    s_CheckPoints[i].count = 0;

    checkPointList_.AddItem(s_CheckPoints[i].desc, 7, 3);
  }

  SetHeaderInfo(HEADER_STYLE_INFO, IDS_PROMPT_RUN_CASH, 0);
  ShowHeaderButton(IDC_BUTTON_STEP, false);

  monitor_ = RunCashMonitor::GetInstance();
  monitor_->AddObserver(_T("PageRunCashTest::OnCashInfo"), 
      RunCashMonitor::SUBJECT_CASH_INFO, &PageRunCashTest::OnCashInfo, this);
}

bool PageRunCashTest::OnLeave(int enterPageId) {
  monitor_->RemoveObserver(_T("PageRunCashTest::OnCashInfo"));
  monitor_->Release();
  monitor_ = NULL;

  return true;
}

void PageRunCashTest::OnCashInfo(int subject) {
  int currentCount = monitor_->Freeze();
  ASSERT(currentCount > 0);
  Cash *cash = monitor_->GetCash(currentCount - 1);
  if (cash->Valid(Cash::VALID_INFO)) {
    for (int j = 0; j < _countof(s_CheckPoints); j++) {
      if (((s_CheckPoints[j].denomination == ANY) || (s_CheckPoints[j].denomination == cash->GetDenomination())) &&
          ((s_CheckPoints[j].version == ANY) || (s_CheckPoints[j].version == cash->GetVersion())) &&
          ((s_CheckPoints[j].direction == ANY) || (s_CheckPoints[j].direction == cash->GetDirection()))) {
        s_CheckPoints[j].count++;
      }
    }
    ::PostMessage(m_hWnd, WM_CASH_INFO, 0, 0);
  }
  monitor_->Unfreeze();
}

void PageRunCashTest::UpdateCheckState() {
  bool allDone = true;
  for (int i = 0; i < _countof(s_CheckPoints); i++) {
    if (s_CheckPoints[i].count < s_CheckPoints[i].targetCount) {
      allDone = false;
      checkPointList_.SetState(i, 3);
    } else {
      checkPointList_.SetState(i, 1);
    }
  }
  if (allDone) {
    DeviceProxy *device = DeviceProxy::GetInstance();
    // how to deal error return??
    device->UpdateDebugState(DEBUG_RUN_CASH_TEST_DONE, DEBUG_RUN_CASH_TEST_DONE);

    SetHeaderInfo(HEADER_STYLE_OK, IDS_PROMPT_RUN_CASH_FINISHED, 0);
    ShowHeaderButton(IDC_BUTTON_STEP, true);
  }
}

BEGIN_MESSAGE_MAP(PageRunCashTest, Page)
  ON_WM_SIZE()
  ON_MESSAGE(WM_CASH_INFO, &PageRunCashTest::OnCashInfo)
  ON_BN_CLICKED(IDC_BUTTON_STEP, &PageRunCashTest::OnBnClickedButtonStep)
END_MESSAGE_MAP()

// PageRunCashTest message handlers
BOOL PageRunCashTest::OnInitDialog() {
  Page::OnInitDialog();

  CBitmap bitmap;
  bitmap.LoadBitmap(IDB_STATUS);
  imageList_.Create(32, 32, ILC_COLOR32 | ILC_MASK, 8, 0);
  imageList_.Add(&bitmap, RGB(255, 0, 255));
  checkPointList_.SetImageList(&imageList_);

  layout_.Init(m_hWnd);
  layout_.AddDlgItem(IDC_LIST_CHECK_POINTS, AnchorLayout::TOP_LEFT, AnchorLayout::BOTTOM_RIGHT);
  
  HeaderButton buttons = { IDC_BUTTON_STEP, IDS_BUTTON_CONTINUE };
  SetHeaderButtons(&buttons, 1);
  ShowHeader(true);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void PageRunCashTest::OnSize(UINT nType, int cx, int cy) {
  Page::OnSize(nType, cx, cy);

  // TODO: Add your message handler code here
  if (::IsWindow(m_hWnd)) {
    layout_.RecalcLayout();
  }
}

LRESULT PageRunCashTest::OnCashInfo(WPARAM wParam, LPARAM lParam) {
  UpdateCheckState();

  return 0;
}

void PageRunCashTest::OnBnClickedButtonStep() {
  GotoStep(STEP_FINISH);
}
