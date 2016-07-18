// Page.cpp : implementation file
//

#include "stdafx.h"
#include "page.h"
#include "gui_resources.h"
#include "main_dialog.h"
#include "graphics.h"

#ifdef _DBEUG
#define new DEBUG_NEW
#endif

// Page dialog

IMPLEMENT_DYNAMIC(Page, CDialog)

CImageList Page::imageList_;

Page::Page(UINT templateId, CWnd* pParent /*=NULL*/)
	: CDialog(templateId, pParent) {
  header_.icon = NULL;
  header_.bgColor = RGB(255, 255, 255);
  header_.textColor = RGB(0, 0, 0);
  ZeroMemory(header_.buttons, sizeof(header_.buttons));
  header_.visible = false;
  header_.height = 64;
}

Page::~Page() {
}

void Page::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
}

void Page::OnEnter(int leavePageId) {
}

bool Page::OnLeave(int enterPageId) {
  return true;
}

void Page::SetHeaderButtons(HeaderButton *buttons, int count) {
  ASSERT(count < _countof(header_.buttons));

  for (int i = 0; i < _countof(header_.buttons); i++) {
    if (header_.buttons[i] != NULL) {
      header_.buttons[i]->DestroyWindow();
      delete header_.buttons[i]; 
      header_.buttons[i] = NULL;
    }    
  }

  ButtonStyle buttonStyle;
  buttonStyle.bgColor = RGB(54, 133, 214);
  buttonStyle.textColor = RGB(255, 255, 255);
  
  for (int i = 0; i < count; i++) {
    CString caption;
    caption.LoadString(buttons[i].textResId);
    Button *button = new Button();
    button->Create(caption, WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, CRect(0, 0, 64, 32), this, buttons[i].id);
    button->SetFont(GuiResources::GetInstance()->GetFont(GuiResources::FONT_NORMAL));
    button->SetStyle(BUTTON_STATE_NORMAL, buttonStyle);
    button->SetStyle(BUTTON_STATE_OVER, buttonStyle);
    button->SetStyle(BUTTON_STATE_DOWN, buttonStyle);
    header_.buttons[i] = button;
  }

  LayoutHeaderButtons();
}

void Page::SetHeaderInfo(int style, int textResId, int subTextResId) {
  CString text, subText;

  text.LoadString(textResId);
  subText.LoadString(subTextResId);

  SetHeaderInfo(style, text, subText);
}

void Page::SetHeaderInfo(int style, LPCTSTR text, LPCTSTR subText) {
  header_.text = text;
  header_.subText = subText;

  switch (style) {
    case HEADER_STYLE_ERROR:
      header_.icon = ICON_ERROR;
      header_.bgColor = RGB(255, 238, 238);
      header_.textColor = RGB(227, 0, 0);
      break;
    case HEADER_STYLE_OK:
      header_.icon = ICON_OK;
      header_.bgColor = RGB(233, 255, 224);
      header_.textColor = RGB(38, 179, 0);
      break;
    default:
      header_.icon = ICON_INFO;
      header_.bgColor = RGB(238, 238, 238);
      header_.textColor = RGB(54, 133, 214);
      break;
  }  

  Invalidate(FALSE);
}

void Page::SetHeaderButtonText(int id, int textResId) {
  CString text;
  text.LoadString(textResId);
  SetDlgItemText(id, text);
}

void Page::ShowHeaderButton(int id, bool visibility) {
  CWnd *button = GetDlgItem(id);
  if (button != NULL) {
    button->ShowWindow(visibility ? SW_SHOW : SW_HIDE);
  }
}

void Page::ShowHeader(bool visibility) {
  header_.visible = visibility;

  Invalidate(FALSE);
}

bool Page::ShowPage(int pageId) {
  MainDialog *mainDialog = (MainDialog *)GetParent();
  ASSERT(mainDialog != NULL);
  return mainDialog->ShowPage(pageId);
}

bool Page::GotoStep(int step) {
  MainDialog *mainDialog = (MainDialog *)GetParent();
  ASSERT(mainDialog != NULL);
  return mainDialog->GotoStep(step);
}

void Page::EnableConnectionCheck(bool enabled) {
  MainDialog *mainDialog = (MainDialog *)GetParent();
  ASSERT(mainDialog != NULL);
  mainDialog->EnableConnectionCheck(enabled);
}

void Page::OnOK() {
}

void Page::OnCancel() {
}

BEGIN_MESSAGE_MAP(Page, CDialog)
  ON_WM_PAINT()
  ON_WM_ERASEBKGND()
  ON_WM_SIZE()
END_MESSAGE_MAP()

// Page message handlers
void Page::OnPaint() {
  BufferredPaintDC dc(this);
  
  CRect rectClient;
  GetClientRect(&rectClient);
  dc.FillSolidRect(rectClient, GetBgColor());

  if (header_.visible) {
    DrawHeader(dc);
    rectClient.top += header_.height;
  }

  DrawClient(dc, rectClient);
}

void Page::DrawHeader(CDC &dc) {
  CRect rect;
  GetClientRect(&rect);
  rect.bottom = rect.top + header_.height;

  dc.FillSolidRect(rect, header_.bgColor);

  int x = rect.left + 40;
  int y = rect.top + (rect.Height() - 32) / 2;
  imageList_.Draw(&dc, header_.icon, CPoint(x, y), ILD_NORMAL);
  x += 32 + 8;

  dc.SetTextColor(header_.textColor);
  dc.SetBkMode(TRANSPARENT);

  CFont *oldFont = dc.SelectObject(GuiResources::GetInstance()->GetFont(GuiResources::FONT_BIG));
  if (header_.subText.IsEmpty()) {
    CRect rectText(rect);
    rectText.left = x;
    dc.DrawText(header_.text, &rectText, DT_SINGLELINE | DT_VCENTER);
  } else {
    int textHeight = dc.GetTextExtent(header_.text).cy;
    dc.SelectObject(GuiResources::GetInstance()->GetFont(GuiResources::FONT_NORMAL));
    int subTextHeight = dc.GetTextExtent(header_.subText).cy;
    int y = rect.top + (rect.Height() - textHeight - subTextHeight) / 2;
    dc.SelectObject(GuiResources::GetInstance()->GetFont(GuiResources::FONT_BIG));
    dc.TextOut(x, y, header_.text);
    dc.SelectObject(GuiResources::GetInstance()->GetFont(GuiResources::FONT_NORMAL));
    dc.TextOut(x, y + textHeight, header_.subText);
  }
  dc.SelectObject(oldFont);
}

void Page::DrawClient(CDC &dc, const CRect &rect) {
}

BOOL Page::OnEraseBkgnd(CDC* pDC) {
  return TRUE;
}

BOOL Page::OnInitDialog() {
  CDialog::OnInitDialog();

  if (imageList_.GetSafeHandle() == NULL) {
    CBitmap bitmap;
    bitmap.LoadBitmap(IDB_STATUS);
    imageList_.Create(32, 32, ILC_COLOR32 | ILC_MASK, 3, 0);
    imageList_.Add(&bitmap, RGB(255, 0, 255));
  }

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}


void Page::OnSize(UINT nType, int cx, int cy) {
  CDialog::OnSize(nType, cx, cy);

  if (::IsWindow(m_hWnd)) {
    LayoutHeaderButtons();
  }
}

void Page::LayoutHeaderButtons() {
  CRect clientRect;
  GetClientRect(&clientRect);

  int count = 0;
  for (int i = 0; i < _countof(header_.buttons); i++) {
    if (header_.buttons[i] != NULL) {
      count++;
    }
  }

  if (count > 0) {
    HDWP hdwp = BeginDeferWindowPos(count);
    int x = clientRect.right - 40;
    for (int i = 0; i < _countof(header_.buttons); i++) {
      if (header_.buttons[i] != NULL) {
        CRect buttonRect;
        header_.buttons[i]->GetWindowRect(&buttonRect);
        x -= buttonRect.Width();
        int y = clientRect.top + (header_.height - buttonRect.Height()) / 2;
        DeferWindowPos(hdwp, header_.buttons[i]->GetSafeHwnd(), 
            NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
        x -= 8;
      }
    }
    EndDeferWindowPos(hdwp);
  }
}