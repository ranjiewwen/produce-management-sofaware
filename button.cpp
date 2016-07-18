// button.cpp : implementation file
//

#include "stdafx.h"
#include "button.h"


// Button

IMPLEMENT_DYNAMIC(Button, CButton)

Button::Button()
: mouseOn_(false)
{

}

Button::~Button()
{
}


void Button::SetStyle(ButtonState state, const ButtonStyle &style) {
  ASSERT(state >= 0 && state < _countof(styles_));

  styles_[state] = style;
}

BEGIN_MESSAGE_MAP(Button, CButton)
  ON_WM_MOUSEMOVE()
  ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()

// Button message handlers
void Button::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
  CDC *dc = CDC::FromHandle(lpDrawItemStruct->hDC);
  CRect rectItem(lpDrawItemStruct->rcItem);
  int state = lpDrawItemStruct->itemState;
  if (state & ODS_SELECTED) {
    state = BUTTON_STATE_DOWN;
  } else if (mouseOn_) {
    state = BUTTON_STATE_OVER;
  } else {
    state = BUTTON_STATE_NORMAL;
  }

  dc->FillSolidRect(rectItem, styles_[state].bgColor);
  
  int cxContent = 0;
  if (styles_[state].bitmap.IsValid()) {
    cxContent += styles_[state].bitmap.GetWidth();
  }

  CString text;
  GetWindowText(text);
  if (!text.IsEmpty()) {
    if (cxContent != 0) {
      cxContent += 4;
    }
    cxContent += dc->GetTextExtent(text).cx;
  }

  int x = rectItem.left + (rectItem.Width() - cxContent) / 2;
  if (styles_[state].bitmap.IsValid()) {
    int y = rectItem.top + (rectItem.Height() - styles_[state].bitmap.GetHeight()) / 2;
    styles_[state].bitmap.Draw(dc->GetSafeHdc(), x, y);
    x += styles_[state].bitmap.GetWidth() + 4;
  }

  if (!text.IsEmpty()) {
    CRect rectText(rectItem);
    rectText.left = x;
    dc->SetTextColor(styles_[state].textColor);
    dc->SetBkColor(styles_[state].bgColor);
    dc->DrawText(text, &rectText, DT_SINGLELINE | DT_VCENTER);
  }

  if (styles_[state].borderColor != CLR_NONE) {
    dc->Draw3dRect(rectItem, styles_[state].borderColor, styles_[state].borderColor);
  }
}


void Button::OnMouseMove(UINT nFlags, CPoint point)
{
  if (!mouseOn_) {
    mouseOn_ = true;
    Invalidate(FALSE);
    TRACKMOUSEEVENT track;
    ZeroMemory(&track, sizeof(track));
    track.cbSize = sizeof(track);
    track.dwFlags = TME_LEAVE;
    track.hwndTrack = m_hWnd;
    _TrackMouseEvent(&track);        
  }

  CButton::OnMouseMove(nFlags, point);
}

void Button::OnMouseLeave()
{
  if (mouseOn_) {
    mouseOn_ = false;
    Invalidate(FALSE);
  }

  CButton::OnMouseLeave();
}
