// page_connect.cpp : implementation file
//

#include "stdafx.h"
#include "page_ir_calibration_start.h"
#include "gui_resources.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(PageIRCalibrationStart, Page)

PageIRCalibrationStart::PageIRCalibrationStart(CWnd* pParent /*=NULL*/)
	: Page(PageIRCalibrationStart::IDD, pParent)
{

}

PageIRCalibrationStart::~PageIRCalibrationStart()
{
}

void PageIRCalibrationStart::DoDataExchange(CDataExchange* pDX)
{
  Page::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_STATIC_TIPS, labelTips_);
  DDX_Control(pDX, IDC_BUTTON_START, startButton_);
}


BEGIN_MESSAGE_MAP(PageIRCalibrationStart, Page)
  ON_WM_SIZE()
  ON_BN_CLICKED(IDC_BUTTON_START, &PageIRCalibrationStart::OnBnClickedButtonStart)
END_MESSAGE_MAP()
// PageIRCalibrationStart message handlers


BOOL PageIRCalibrationStart::OnInitDialog() {
  Page::OnInitDialog();

  // TODO:  Add extra initialization here
  labelTips_.SetFont(GuiResources::GetInstance()->GetFont(GuiResources::FONT_BIG_BOLD));
  labelTips_.SetBkColor(GetBgColor());
  labelTips_.SetTextColor(RGB(54, 133, 214));
  
  ButtonStyle buttonStyle;
  buttonStyle.bgColor = RGB(66, 142, 220);
  buttonStyle.textColor = RGB(255, 255, 255);
  startButton_.SetStyle(BUTTON_STATE_NORMAL, buttonStyle);
  startButton_.SetStyle(BUTTON_STATE_OVER, buttonStyle);
  startButton_.SetStyle(BUTTON_STATE_DOWN, buttonStyle);

  layout_.Init(m_hWnd);
  layout_.AddDlgItem(IDC_STATIC_TIPS, AnchorLayout::CENTER, AnchorLayout::CENTER);
  layout_.AddDlgItem(IDC_BUTTON_START, AnchorLayout::CENTER, AnchorLayout::CENTER);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}


void PageIRCalibrationStart::OnSize(UINT nType, int cx, int cy) {
  Page::OnSize(nType, cx, cy);

  if (::IsWindow(m_hWnd)) {
    layout_.RecalcLayout();
  }
}

void PageIRCalibrationStart::OnBnClickedButtonStart() {
  ShowPage(IDD_PAGE_IR_CALIBRATION_DO);
}
