// Label.cpp : implementation file
//

#include "stdafx.h"
#include "Label.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Label

IMPLEMENT_DYNAMIC(Label, CStatic)

Label::Label()
  : bkColor_(RGB(255, 255, 255))
  , textColor_(RGB(0, 0, 0))
  , textAlign_(DT_TOP | DT_LEFT) {
}

Label::~Label() {
}

void Label::SetBkColor(COLORREF color) {
  bkColor_ = color;
}

void Label::SetTextColor(COLORREF color) {
  textColor_ = color;
}

BEGIN_MESSAGE_MAP(Label, CStatic)
END_MESSAGE_MAP()


void Label::PreSubclassWindow() {
  DWORD style = GetStyle();
  if ((style & SS_TYPEMASK) == SS_CENTER) {
    textAlign_ = DT_TOP | DT_CENTER;
  }
  ModifyStyle(0, SS_OWNERDRAW);

  CStatic::PreSubclassWindow();
}

void Label::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
  CDC *dc = CDC::FromHandle(lpDrawItemStruct->hDC);

  CRect rect(lpDrawItemStruct->rcItem);
  dc->FillSolidRect(rect, bkColor_);

  CString text;
  GetWindowText(text);
  dc->SetBkMode(TRANSPARENT);
  dc->SetTextColor(textColor_);
  CFont *font = GetFont();
  CFont *oldFont = dc->SelectObject(font);
  UINT format = textAlign_;
  dc->DrawText(text, rect, format);
  if (oldFont != NULL) {
    dc->SelectObject(oldFont);
  }
}
