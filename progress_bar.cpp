// progress_bar.cpp : implementation file
//

#include "stdafx.h"
#include "progress_bar.h"
#include "gui_resources.h"

// ProgressBar

IMPLEMENT_DYNAMIC(ProgressBar, CStatic)

ProgressBar::ProgressBar()
  : minPos_(0), maxPos_(100)
  , pos_(0) {
}

ProgressBar::~ProgressBar() {
}

void ProgressBar::SetRange(int minPos, int maxPos) {
  ASSERT(maxPos > minPos);
  minPos_ = minPos;
  maxPos_ = maxPos;
}

void ProgressBar::SetPos(int pos) {
  if (pos < minPos_) {
    pos = minPos_;
  }
  if (pos > maxPos_) {
    pos = maxPos_;
  }
  pos_ = pos;

  if (::IsWindow(m_hWnd)) {
    Invalidate();
  }
}

BEGIN_MESSAGE_MAP(ProgressBar, CStatic)
  ON_WM_PAINT()
  ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void ProgressBar::OnPaint() {
  CPaintDC dc(this);
  CRect rectClient;
  GetClientRect(&rectClient);
  dc.FillSolidRect(rectClient, RGB(232, 232, 232));
  CRect rectProgress(rectClient);
  rectProgress.right = rectClient.left + rectClient.Width() * (pos_ - minPos_) / (maxPos_ - minPos_);
  dc.FillSolidRect(rectProgress, RGB(54, 133, 214));
  CGdiObject *oldFont = dc.SelectObject(GuiResources::GetInstance()->GetFont(GuiResources::FONT_NORMAL));
  dc.SetTextColor(RGB(0, 0, 0));
  dc.SetBkMode(TRANSPARENT);
  CString progress;
  progress.Format(_T("%d%%"), (pos_ - minPos_) * 100 / (maxPos_ - minPos_));
  dc.DrawText(progress, rectClient, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
  dc.SelectObject(oldFont);
}

BOOL ProgressBar::OnEraseBkgnd(CDC *pDC) {
  return TRUE;
}


// ProgressBar message handlers


