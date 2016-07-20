// login_dialog.cpp : implementation file
//

#include "stdafx.h"
#include "login_dialog.h"
#include "gui_resources.h"
#include "thread_inl.h"
#include "working_parameters.h"
#include "error.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// LoginDialog
IMPLEMENT_DYNAMIC(LoginDialog, CDialog)

LoginDialog::LoginDialog()
: CDialog(IDD_LOGIN) {
}

LoginDialog::~LoginDialog() {
}

// LoginDialog message handlers
BOOL LoginDialog::OnInitDialog() {
  CDialog::OnInitDialog();

  // close button style
  ButtonStyle buttonStyle;
  buttonStyle.bgColor = RGB(54, 133, 214);
  buttonStyle.textColor = RGB(255, 255, 255);
  buttonStyle.bitmap.LoadFromResource(IDB_BUTTON_CLOSE);
  buttonStyle.bitmap.SetWithAlpha();
  buttonStyle.bitmap.PreMultiplyAlpha();
  closeButton_.SetStyle(BUTTON_STATE_NORMAL, buttonStyle);
  buttonStyle.bgColor = RGB(255, 64, 64);
  closeButton_.SetStyle(BUTTON_STATE_OVER, buttonStyle);
  buttonStyle.bgColor = RGB(186, 0, 0);
  closeButton_.SetStyle(BUTTON_STATE_DOWN, buttonStyle);

  // login button style
  buttonStyle.bgColor = RGB(54, 133, 214);
  buttonStyle.textColor = RGB(255, 255, 255);
  buttonStyle.bitmap.Destroy();
  loginButton_.SetStyle(BUTTON_STATE_NORMAL, buttonStyle);
  loginButton_.SetStyle(BUTTON_STATE_OVER, buttonStyle);
  loginButton_.SetStyle(BUTTON_STATE_DOWN, buttonStyle);

  userNameLabel_.SetBkColor(RGB(255, 255, 255));
  passwordLabel_.SetBkColor(RGB(255, 255, 255));

  header_.LoadFromResource(IDB_LOGIN_HEADER);

  SetDlgItemText(IDC_EDIT_USER_NAME, 
      AfxGetApp()->GetProfileString(_T("Recently"), _T("User"), _T("")));

  pButton = (CButton*)GetDlgItem(IDC_CHECK_REMPSWD);
  //下一次登录的时候，检查注册表里的值，如果记录了用户名密码，说明当前处于记住密码的状态，那么就把注册表里记录的数据填在用户名密码里
  CString temp=AfxGetApp()->GetProfileString(_T("Recently"), _T("PassWord"), _T(""));
  //if (temp!=L"")
  if (_tcscmp(temp, TEXT("")) != 0)  //比较相等返回0，然后就是大于小于0；
  {	  
	  pButton->SetCheck(1);
	  SetDlgItemText(IDC_EDIT_PASSWORD,
		  AfxGetApp()->GetProfileString(_T("Recently"), _T("PassWord"), _T("")));
  }
  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void LoginDialog::DoDataExchange(CDataExchange* pDX) {
  DDX_Control(pDX, IDCANCEL, closeButton_);
  DDX_Control(pDX, IDOK, loginButton_);
  DDX_Control(pDX, IDC_STATIC_USER_NAME, userNameLabel_);
  DDX_Control(pDX, IDC_STATIC_PASSWORD, passwordLabel_);
  DDX_Control(pDX, IDC_CHECK_REMPSWD, pBtn);
  CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(LoginDialog, CDialog)
  ON_WM_PAINT()
  ON_WM_ERASEBKGND()
  ON_BN_CLICKED(IDC_CHECK_REMPSWD, &LoginDialog::OnBnClickedCheckRempswd)
END_MESSAGE_MAP()

void LoginDialog::OnPaint() {
  CPaintDC dc(this); // device context for painting
  
  CRect clientRect;
  GetClientRect(&clientRect);
  dc.FillSolidRect(clientRect, RGB(255, 255, 255));

  CRect headerRect(clientRect);
  headerRect.bottom = headerRect.top + 96;
  dc.SetStretchBltMode(HALFTONE);
  header_.Draw(dc.GetSafeHdc(), headerRect);

  CString title;
  title.LoadString(AFX_IDS_APP_TITLE);
  CFont *font = GuiResources::GetInstance()->GetFont(GuiResources::FONT_BIG_BOLD);
  CFont *oldFont = dc.SelectObject(font);
  dc.SetBkMode(TRANSPARENT);
  dc.SetTextColor(RGB(255, 255, 255));
  dc.DrawText(title, headerRect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
  dc.SelectObject(oldFont);

  dc.Draw3dRect(clientRect, RGB(49, 90, 133), RGB(49, 90, 133));
}


BOOL LoginDialog::OnEraseBkgnd(CDC* pDC) {
  return TRUE;
}

void LoginDialog::OnOK() {
  GetDlgItemText(IDC_EDIT_USER_NAME, userName_);
  GetDlgItemText(IDC_EDIT_PASSWORD, password_);
  
  int permission = WorkingParameters::GetInstance()->Login(userName_, password_);
  if ((permission != 3) && (permission != 2)) {
    AfxMessageBox(IDS_PROMPT_INVALID_USER_NAME_PASSWORD);
    return;
  } 
  AfxGetApp()->WriteProfileString(_T("Recently"), _T("User"), userName_);

  //勾选以后，如果登录成功，就把当前的用户名密码记录在注册表里
  //如果没有勾选，登录成功，就把注册表里记录的数据清空
 
  isRemPswd = pButton->GetCheck();
  if (isRemPswd)
  {
	  AfxGetApp()->WriteProfileString(_T("Recently"), _T("PassWord"), password_);
  }
  if (!isRemPswd)
  {
	  //清注册表
	  AfxGetApp()->WriteProfileString(_T("Recently"), _T("PassWord"), L"");
  }
   CDialog::OnOK();
}


void LoginDialog::OnCancel() {
  CDialog::OnCancel();
}

void LoginDialog::OnBnClickedCheckRempswd()
{
	// TODO:  在此添加控件通知处理程序代码
	//if (pButton->GetCheck())
	//{
	//	isRemPswd = true;
	//}
	//if (!pButton->GetCheck())
	//{
	//	isRemPswd = false;
	//}
}
