// StatusBar.cpp : implementation file
//

#include "stdafx.h"
#include "status_bar.h"
#include "graphics.h"
#include "gui_resources.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// StatusBar
IMPLEMENT_DYNAMIC(StatusBar, CWnd)

StatusBar::StatusBar() {
}

StatusBar::~StatusBar() {
}

BOOL StatusBar::Create(CWnd *parent, int place) {
  ASSERT(parent != NULL);
  CRect rect(0, 0, 0, 0);
  CWnd *placeWnd = parent->GetDlgItem(place);
  if (placeWnd != NULL) {
    placeWnd->GetWindowRect(&rect);
    parent->ScreenToClient(&rect);
  }
  return CWnd::Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, parent, IDC_STATIC);
}

void StatusBar::AddPanel(int id, LPCTSTR text, int icon, int align) {
  Panel panel;
  panel.id = id;
  panel.text = text;
  if (icon != 0) {
    panel.icon.LoadFromResource(icon);
  }
  panel.align = align;
  panels_.push_back(panel);

  Invalidate(FALSE);
}

void StatusBar::SetPanel(int id, LPCTSTR text, int icon) {
  for (Panels::iterator i = panels_.begin(); i != panels_.end(); ++i) {
    if ((*i).id == id) {
      (*i).text = text;
      if (icon != 0) {
        (*i).icon.LoadFromResource(icon);
      } else {
        (*i).icon.Destroy();
      }      
      Invalidate(FALSE);
      return;
    }
  }
}

BEGIN_MESSAGE_MAP(StatusBar, CWnd)
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
END_MESSAGE_MAP()



// StatusBar message handlers
BOOL StatusBar::OnEraseBkgnd(CDC* pDC) {
  return TRUE;
}

void StatusBar::OnPaint() {
  BufferredPaintDC dc(this);

  CRect rectClient;
  GetClientRect(&rectClient);
  dc.FillSolidRect(&rectClient, RGB(240, 240, 240));
  
  CFont *oldFont = dc.SelectObject(GuiResources::GetInstance()->GetFont(GuiResources::FONT_NORMAL));
  dc.SetTextColor(RGB(161,161,161));
  dc.SetBkMode(TRANSPARENT);

  int left = rectClient.left + 16;
  int right = rectClient.right - 16;
  for (Panels::iterator i = panels_.begin(); i != panels_.end(); ++i) {
    CSize textSize = dc.GetTextExtent((*i).text);
    CRect panelRect;
    panelRect.top = rectClient.top;
    panelRect.bottom = rectClient.bottom;
    if ((*i).align == PANEL_ALIGN_LEFT) {
      panelRect.left = left;
      panelRect.right = left + textSize.cx;
      if ((*i).icon.IsValid()) {
        panelRect.right += (*i).icon.GetWidth() + 4;
      }
      left = panelRect.right + 32;
    } else {
      panelRect.right = right;
      panelRect.left = right - textSize.cx;
      if ((*i).icon.IsValid()) {
        panelRect.left -= (*i).icon.GetWidth() + 4;
      }
      right = panelRect.left - 32;
    }
    if ((*i).icon.IsValid()) {
      int y = panelRect.top + (panelRect.Height() - (*i).icon.GetHeight()) / 2;
      (*i).icon.Draw(dc.GetSafeHdc(), panelRect.left, y);
      panelRect.left += (*i).icon.GetWidth() + 4;
    }
    dc.DrawText((*i).text, panelRect, DT_SINGLELINE | DT_VCENTER);
  }

  dc.SelectObject(oldFont);

  dc.FillSolidRect(rectClient.left, rectClient.top, rectClient.Width(), 2, RGB(220, 220, 220));
}
