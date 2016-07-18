// input_sn_dialog.cpp : implementation file
//

#include "stdafx.h"
#include "input_sn_dialog.h"
#include "gui_resources.h"
#include "device_proxy.h"
#include "device_proxy_inl.h"

#define WM_DEVICE_DISCONNECTED  WM_USER + 101

// InputSNDialog dialog

IMPLEMENT_DYNAMIC(InputSNDialog, CDialog)

InputSNDialog::InputSNDialog(CWnd* pParent /*=NULL*/)
	: CDialog(InputSNDialog::IDD, pParent) {
}

InputSNDialog::~InputSNDialog() {
}

void InputSNDialog::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_EDIT_SN, snEdit_);
  DDX_Control(pDX, IDC_BUTTON_OK, ok_);
  DDX_Control(pDX, IDC_STATIC_TIPS, tips_);
}

void InputSNDialog::OnOK() {  
}

void InputSNDialog::OnCancel() {
}

BEGIN_MESSAGE_MAP(InputSNDialog, CDialog)
  ON_BN_CLICKED(IDC_BUTTON_OK, &InputSNDialog::OnBnClickedButtonOk)
  ON_WM_PAINT()
  ON_WM_DESTROY()
  ON_MESSAGE(WM_DEVICE_DISCONNECTED, &InputSNDialog::OnDeviceDisconnected)
END_MESSAGE_MAP()


// InputSNDialog message handlers
BOOL InputSNDialog::OnInitDialog() {
  CDialog::OnInitDialog();

  GetDlgItem(IDC_EDIT_SN)->SetFont(GuiResources::GetInstance()->GetFont(GuiResources::FONT_BIG_BOLD));
  tips_.SetFont(GuiResources::GetInstance()->GetFont(GuiResources::FONT_NORMAL));
  tips_.SetTextColor(RGB(0,0,0));
  tips_.SetBkColor(RGB(238,238,238));
  
  ButtonStyle buttonStyle;
  buttonStyle.bgColor = RGB(54, 133, 214);
  buttonStyle.textColor = RGB(255, 255, 255);
  buttonStyle.bitmap.Destroy();
  ok_.SetStyle(BUTTON_STATE_NORMAL, buttonStyle);
  ok_.SetStyle(BUTTON_STATE_OVER, buttonStyle);
  ok_.SetStyle(BUTTON_STATE_DOWN, buttonStyle);

  DeviceProxy::GetInstance()->AddObserver(_T("InputSNDialog::OnDeviceDisconnected"), m_hWnd, DeviceProxy::SUBJECT_DISCONNECTED, WM_DEVICE_DISCONNECTED);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void InputSNDialog::OnBnClickedButtonOk() {
  CString sn;
  GetDlgItemText(IDC_EDIT_SN, sn);
  sn.Trim();
  if (sn.IsEmpty()) {
    return;
  }

  if (!DeviceProxy::GetInstance()->SetSerialNumber(sn)) {
    return;
  }

  EndDialog(IDOK);
}

BEGIN_MESSAGE_MAP(SerialNumberEdit, CEdit)
  ON_WM_KEYDOWN()
  ON_WM_KEYUP()
END_MESSAGE_MAP()


void SerialNumberEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
  // TODO: Add your message handler code here and/or call default 
  if (nChar == VK_RETURN) {
    return;
  }
  CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}


void SerialNumberEdit::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
  // TODO: Add your message handler code here and/or call default
  if (nChar == VK_RETURN) {
    SetSel(0, -1);
    return;
  }
  CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}


void InputSNDialog::OnPaint() {
  CPaintDC dc(this); // device context for painting
  // TODO: Add your message handler code here
  // Do not call CDialog::OnPaint() for painting messages
  CRect clientRect;
  GetClientRect(&clientRect);
  dc.FillSolidRect(clientRect, RGB(238,238,238));
  dc.Draw3dRect(clientRect, RGB(198,198,198), RGB(198,198,198));
}

LRESULT InputSNDialog::OnDeviceDisconnected(WPARAM, LPARAM) {
  EndDialog(IDCANCEL);
  return 0;
}


void InputSNDialog::OnDestroy() {
  CDialog::OnDestroy();

  DeviceProxy::GetInstance()->RemoveObserver(_T("InputSNDialog::OnDeviceDisconnected"));
}
