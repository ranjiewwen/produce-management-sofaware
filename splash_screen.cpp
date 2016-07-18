// splash_screen.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "splash_screen.h"
#include "graphics.h"
#include "gui_resources.h"
#include "thread_inl.h"
#include "working_parameters.h"
#include "working_parameters_inl.h"
#include "error.h"
#include "application.h"
#include "ftp_client.h"

#define TIMER_PROGRESS    1

#define WM_INIT_EVENT     WM_USER + 100

#define WM_DOWNLOAD_UPDATE    0
#define WM_INIT_DONE          1
#define WM_APP_UPDATED        2

// SplashScreen dialog

IMPLEMENT_DYNAMIC(SplashScreen, CDialog)

SplashScreen::SplashScreen(CWnd* pParent /*=NULL*/)
	: CDialog(SplashScreen::IDD, pParent)
  , showProgress_(false)
  , progress_(0)
  , canOffline_(false)
{
}

SplashScreen::~SplashScreen() {
}

void SplashScreen::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDRETRY, onlineButton_);
  DDX_Control(pDX, IDC_BUTTON_OFFLINE, offlineButton_);
}


BEGIN_MESSAGE_MAP(SplashScreen, CDialog)
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
  ON_WM_TIMER()
  ON_MESSAGE(WM_INIT_EVENT, &SplashScreen::OnInitEvent)
  ON_WM_SIZE()
  ON_BN_CLICKED(IDRETRY, &SplashScreen::OnBnClickedOnline)
  ON_BN_CLICKED(IDC_BUTTON_OFFLINE, &SplashScreen::OnBnClickedOffline)
END_MESSAGE_MAP()

// SplashScreen message handlers
BOOL SplashScreen::OnInitDialog() {
  CDialog::OnInitDialog();

  background_.LoadFromResource(IDB_SPLASH_SCREEN);
  marquee_.LoadFromResource(IDB_MARQUEE);

  ButtonStyle normalStyle(RGB(25,60,100), RGB(255, 255, 255), RGB(102,137,176));
  onlineButton_.SetStyle(BUTTON_STATE_NORMAL, normalStyle);
  onlineButton_.SetStyle(BUTTON_STATE_OVER, normalStyle);
  onlineButton_.SetStyle(BUTTON_STATE_DOWN, normalStyle);
  offlineButton_.SetStyle(BUTTON_STATE_NORMAL, normalStyle);
  offlineButton_.SetStyle(BUTTON_STATE_OVER, normalStyle);
  offlineButton_.SetStyle(BUTTON_STATE_DOWN, normalStyle);

  SetWindowPos(NULL, 0, 0,
	  background_.GetWidth(), background_.GetHeight(),
	  SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

  CenterWindow();

  canOffline_ = WorkingParameters::GetInstance()->LoadXML();

  bool selectOffline = false;
  if (WorkingParameters::GetInstance()->LoadConfig())
  {
	  VariableConfigBlock *config = WorkingParameters::GetInstance()->GetCurrentConfigParameters();
	  if (config->GetIntParameter(_T("SelectOffline"), 0) == 1)
	  {
		  // 允许选择离线登录
		  selectOffline = true;
	  }
  }

  if (selectOffline)
  {
	  FtpClient client;
	  if (!client.Initial())
	  {
		  // 网络连接初始化失败 lux
		  errorCode_ = GetLastError();
		  ::PostMessage(m_hWnd, WM_INIT_EVENT, WM_INIT_DONE, FALSE);
		  return TRUE;
	  }

	  // 在可以优先选择离线登录模式的情况下，显示在线和离线登录选项
	  ShowButtons(SELECT);
  }
  else
  {
	  tips_ = _T("正在加载配置...");
	  ShowProgress(true);

	  initThread_.Start(new MethodRunnable<SplashScreen>(this, &SplashScreen::InitAsync));
  }

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void SplashScreen::ShowProgress(bool show) {
  progress_ =0;  
  if (show) {
    SetTimer(TIMER_PROGRESS, 20, NULL);
  } else {
    KillTimer(TIMER_PROGRESS);
  }
  showProgress_ = show;
  Invalidate(FALSE);
}

BOOL SplashScreen::OnEraseBkgnd(CDC* pDC) {
  return TRUE;//CDialog::OnEraseBkgnd(pDC);
}


void SplashScreen::OnOK() {
  // TODO: Add your specialized code here and/or call the base class

  //CDialog::OnOK();
}


void SplashScreen::OnCancel() {
  if (initThread_.IsAlive()) {
    return;
  }

  CDialog::OnCancel();
}

void SplashScreen::OnBnClickedOffline() {
  if (initThread_.IsAlive()) {
    return;
  }

  if (canOffline_) {
    CDialog::OnOK();
  } else {
    CDialog::OnCancel();
  }
}

void SplashScreen::OnPaint() {
  BufferredPaintDC dc(this);
  
  if (background_.IsValid()) {
    background_.Draw(dc.GetSafeHdc(), 0, 0);
  }
  CRect clientRect;
  GetClientRect(&clientRect);
  CFont *oldFont = dc.SelectObject(GuiResources::GetInstance()->GetFont(GuiResources::FONT_BIG));
  CSize tipsSize = dc.GetTextExtent(tips_);
  if (icon_.IsValid()) {
    tipsSize.cx += icon_.GetWidth() + 8;
  }
  int x = clientRect.left + (clientRect.Width() - tipsSize.cx) / 2;
  int y = (clientRect.top + clientRect.bottom) / 2 - tipsSize.cy;
  if (icon_.IsValid()) {
    icon_.Draw(dc.GetSafeHdc(), x, y + (tipsSize.cy - icon_.GetHeight()) / 2);
    x += icon_.GetWidth() + 8;
  }
  dc.SetTextColor(RGB(255, 255, 255));
  dc.SetBkMode(TRANSPARENT);
  dc.TextOut(x, y, tips_);
  dc.SelectObject(oldFont);
  y += tipsSize.cy + 32;
  if (showProgress_) {
    CRect progressRect(0, y, marquee_.GetWidth() * 2, y + marquee_.GetHeight());    
    progressRect.OffsetRect((clientRect.Width() - progressRect.Width()) / 2, 0);
    dc.FillSolidRect(progressRect, RGB(92,149,195));
    CRgn clipRgn;
    clipRgn.CreateRectRgn(progressRect.left, progressRect.top, 
        progressRect.right, progressRect.bottom);
    dc.SelectClipRgn(&clipRgn);
    x = progressRect.left + progress_ - marquee_.GetWidth();
    marquee_.Draw(dc.GetSafeHdc(), x, y);
    dc.SelectClipRgn(NULL);
    progress_ += 4;
    if (progress_ > progressRect.Width() + marquee_.GetWidth()) {
      progress_ = 0;
    }
    y = progressRect.bottom + 32;
  }
}

void SplashScreen::OnTimer(UINT_PTR nIDEvent) {
  Invalidate(FALSE);

  CDialog::OnTimer(nIDEvent);
}

DWORD SplashScreen::InitAsync() {
	CoInitialize(NULL);

  if (!WorkingParameters::GetInstance()->LoadFromServer()) {
    errorCode_ = GetLastError();
    ::PostMessage(m_hWnd, WM_INIT_EVENT, WM_INIT_DONE, FALSE);
    CoUninitialize();
    return 0;
  }

  // 下载可变配置文件 lux
  // 但是第一次是下载不到可变配置文件的，因为只有上传记录以后，服务器才会有终端记录
  // 所以即使下载失败，也不能强制退出
  WorkingParameters::GetInstance()->LoadConfigFromServer();

  if (WorkingParameters::GetInstance()->IsAppUpdated()) {
    ::PostMessage(m_hWnd, WM_INIT_EVENT, WM_APP_UPDATED, 0);
    CoUninitialize();
    return 0;
  }

  CoUninitialize();

  ::PostMessage(m_hWnd, WM_INIT_EVENT, WM_INIT_DONE, TRUE);

  return 0;
}

LRESULT SplashScreen::OnInitEvent(WPARAM wParam, LPARAM lParam) {
  int code = (int)wParam;
  
  if (code == WM_INIT_DONE) {
    initThread_.WaitForExit(INFINITE);

    BOOL success = (BOOL)lParam;
    if (success) {
      EndDialog(IDOK);
    } else {
      icon_.LoadFromResource(canOffline_ ? IDB_ALERT : IDB_ERROR);
      tips_ = FormatErrorMessage(errorCode_);
      ShowProgress(false);
      ShowButtons(RETRY);
      Invalidate(FALSE);
    }
  } else if (code == WM_APP_UPDATED) {
    initThread_.WaitForExit(INFINITE);

    if (!theApp.Upgrade(WorkingParameters::GetInstance()->GetAppUpdateFilePath(),
        WorkingParameters::GetInstance()->GetLastAppVersion())) {
      AfxMessageBox(IDS_PROMPT_UPGRADE_FAILED, MB_ICONERROR | MB_OK);
      EndDialog(IDOK);
    }

    EndDialog(IDCANCEL);
  } else if (code == WM_DOWNLOAD_UPDATE) {
    tips_ = _T("正在下载更新...");
  }

  return 0;
}

void SplashScreen::ShowButtons(ButtonState state) 
{
	CRect clientRect;
	GetClientRect(&clientRect);
	int y = (clientRect.top + clientRect.bottom) / 2 + 40;
	const CSize buttonSize = { 128, 48 };
	const int padding = 12;
	CRect leftButtonRect(0, y, buttonSize.cx, y + buttonSize.cy);
	leftButtonRect.OffsetRect((clientRect.Width() - 2 * buttonSize.cx - padding) / 2, 0);
	CRect rightButtonRect(0, y, buttonSize.cx, y + buttonSize.cy);
	rightButtonRect.OffsetRect(leftButtonRect.right + padding, 0);

	switch (state)
	{
	case SELECT:
		offlineButton_.SetWindowText(canOffline_ ? _T("离线") : _T("退出"));
		onlineButton_.SetWindowText(_T("在线"));
		onlineButton_.SetWindowPos(NULL, leftButtonRect.left, leftButtonRect.top, leftButtonRect.Width(), leftButtonRect.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		offlineButton_.SetWindowPos(NULL, rightButtonRect.left, rightButtonRect.top, rightButtonRect.Width(), rightButtonRect.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		break;
	case RETRY:
		offlineButton_.SetWindowText(canOffline_ ? _T("离线") : _T("退出"));
		onlineButton_.SetWindowText(_T("重试"));
		onlineButton_.SetWindowPos(NULL, leftButtonRect.left, leftButtonRect.top, leftButtonRect.Width(), leftButtonRect.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		offlineButton_.SetWindowPos(NULL, rightButtonRect.left, rightButtonRect.top, rightButtonRect.Width(), rightButtonRect.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		break;
	case UNSHOWN:
		onlineButton_.ShowWindow(SW_HIDE);
		offlineButton_.ShowWindow(SW_HIDE);
		break;
	default:
		ASSERT(0);
		break;
	}
}

void SplashScreen::OnSize(UINT nType, int cx, int cy) {
  CDialog::OnSize(nType, cx, cy);
}


void SplashScreen::OnBnClickedOnline() {
  if (initThread_.IsAlive()) {
    return;
  }

  tips_ = _T("正在加载配置...");
  icon_.Destroy();
  ShowProgress(true);
  ShowButtons(UNSHOWN);
  Invalidate(FALSE);

  initThread_.Start(new MethodRunnable<SplashScreen>(this, &SplashScreen::InitAsync));
}