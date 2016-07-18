// page_finish.cpp : implementation file
//

#include "stdafx.h"
#include "page_finish.h"
#include "gui_resources.h"
#include "device_proxy.h"
#include "device_proxy_inl.h"
#include "gui_resources.h"
#include "debug_logger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// PageFinish dialog

IMPLEMENT_DYNAMIC(PageFinish, Page)

PageFinish::PageFinish(CWnd* pParent /*=NULL*/)
	: Page(PageFinish::IDD, pParent) {
}

PageFinish::~PageFinish() {
}

void PageFinish::DoDataExchange(CDataExchange* pDX) {
	Page::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(PageFinish, Page)
  ON_WM_SIZE()
  ON_BN_CLICKED(IDC_BUTTON_OK, &PageFinish::OnBnClickedButtonOk)
END_MESSAGE_MAP()


// PageFinish message handlers
BOOL PageFinish::OnInitDialog() {
  Page::OnInitDialog();

  // TODO:  Add extra initialization here
  icon_.LoadFromResource(IDB_DEBUG_FINISH);

  SetHeaderInfo(HEADER_STYLE_INFO, IDS_PROMPT_DEBUG_FINISH, 0);
  HeaderButton buttons = { IDC_BUTTON_OK, IDS_BUTTON_OK };
  SetHeaderButtons(&buttons, 1);
  ShowHeader(true);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void PageFinish::OnSize(UINT nType, int cx, int cy) {
  Page::OnSize(nType, cx, cy);

  // TODO: Add your message handler code here
}

void PageFinish::OnBnClickedButtonOk() {
  DebugLogger::GetInstance()->EndDebug();

  DeviceProxy::GetInstance()->DisableDebug();
}

void PageFinish::DrawClient(CDC &dc, const CRect &rect) {
  CFont *oldFont = dc.SelectObject(GuiResources::GetInstance()->GetFont(GuiResources::FONT_BIG_BOLD));
  CString text;
  text.LoadString(IDS_PROMPT_DEBUG_FINISH);
  CSize textSize = dc.GetTextExtent(text);
  int contentWidth = 48 + icon_.GetWidth() + 16 + textSize.cx + 48;
  int contentHeight = 24 + icon_.GetHeight() + 24;
  int x = rect.left + (rect.Width() - contentWidth) / 2;
  int y = rect.top + (rect.Height() - contentHeight) / 2;
  CRect contentRect(x, y, x + contentWidth, y + contentHeight);
  dc.FillSolidRect(contentRect, RGB(244,244,244));
  dc.Draw3dRect(contentRect, RGB(227,227,227), RGB(227,227,227));
  icon_.Draw(dc.GetSafeHdc(), x + 48, y + 24);
  dc.SetTextColor(RGB(0, 0, 0));
  dc.SetBkMode(TRANSPARENT);
  dc.TextOut(x + 48 + icon_.GetWidth() + 16, rect.top + (rect.Height() - textSize.cy) / 2, text);
  dc.SelectObject(oldFont);
}