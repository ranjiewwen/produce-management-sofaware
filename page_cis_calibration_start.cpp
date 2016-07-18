// Page.cpp : implementation file
//

#include "stdafx.h"
#include "page_cis_calibration_start.h"
#include "gui_resources.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Page dialog

IMPLEMENT_DYNAMIC(PageCISCalibrationStart, Page)

PageCISCalibrationStart::PageCISCalibrationStart(CWnd* pParent /*=NULL*/)
	: Page(PageCISCalibrationStart::IDD, pParent)
{

}

PageCISCalibrationStart::~PageCISCalibrationStart()
{
}

void PageCISCalibrationStart::DoDataExchange(CDataExchange* pDX)
{
  Page::DoDataExchange(pDX);

  DDX_Control(pDX, IDC_STATIC_TIPS, labelTips_);
  DDX_Control(pDX, IDC_BUTTON_START, startButton_);
}


BEGIN_MESSAGE_MAP(PageCISCalibrationStart, Page)
  ON_WM_SIZE()
  ON_BN_CLICKED(IDC_BUTTON_START, &PageCISCalibrationStart::OnBnClickedButtonStart)
END_MESSAGE_MAP()

// PageCISCalibrationStart message handlers

BOOL PageCISCalibrationStart::OnInitDialog() {
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


void PageCISCalibrationStart::OnSize(UINT nType, int cx, int cy) {
  Page::OnSize(nType, cx, cy);

  if (::IsWindow(m_hWnd)) {
    layout_.RecalcLayout();
  }
}

void PageCISCalibrationStart::OnBnClickedButtonStart() {
  ShowPage(IDD_PAGE_CIS_CALIBRATION_DO);
}

