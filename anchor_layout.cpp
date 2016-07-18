#include "stdafx.h"
#include "anchor_layout.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const Anchor AnchorLayout::TOP_LEFT = {0.f, 0.f};
const Anchor AnchorLayout::TOP_RIGHT = { 1.f, 0.f };
const Anchor AnchorLayout::BOTTOM_LEFT = { 0.f, 1.f };
const Anchor AnchorLayout::BOTTOM_RIGHT = { 1.f, 1.f };
const Anchor AnchorLayout::CENTER = { 0.5f, 0.5f };
const Anchor AnchorLayout::TOP_CENTER = { 0.5f, 0.0f };

void ScreenToClient(HWND hwnd, CRect *rect) {
  ASSERT(rect != NULL);
  CPoint topLeft = rect->TopLeft();
  ::ScreenToClient(hwnd, &topLeft);
  CPoint bottomRight = rect->BottomRight();
  ::ScreenToClient(hwnd, &bottomRight);
  rect->SetRect(topLeft, bottomRight);
  rect->NormalizeRect();
}

AnchorLayout::AnchorLayout()
  : hwndParent_(NULL)
  , baseRect_(0, 0, 0, 0) {
}

AnchorLayout::~AnchorLayout() {
}

void AnchorLayout::Init(HWND hwndParent) {
  ASSERT(hwndParent != NULL);

  hwndParent_ = hwndParent;
  ::GetClientRect(hwndParent, &baseRect_);
}

void AnchorLayout::AddDlgItem(int id, const Anchor &anchor0, const Anchor &anchor1) {
  HWND hwndChild = ::GetDlgItem(hwndParent_, id);
  ASSERT(hwndChild != NULL);
  AddAnchor(hwndChild, anchor0, anchor1);
}

void AnchorLayout::AddAnchor(HWND hwndChild, const Anchor &anchor0, const Anchor &anchor1) {
  Child child;
  child.hwnd = hwndChild;
  child.anchor0 = anchor0;
  child.anchor1 = anchor1;
  CRect rect;
  ::GetWindowRect(hwndChild, &rect);
  ::ScreenToClient(hwndParent_, &rect);
  child.baseRect = rect;
  children_.push_back(child);
}

void AnchorLayout::RecalcLayout() {
  if (children_.empty()) {
    return;
  }
  CRect newRect;
  ::GetClientRect(hwndParent_, newRect);
  HDWP hdwp = ::BeginDeferWindowPos((int)children_.size());
  if (hdwp != NULL) {
    int scaleX = newRect.Width() - baseRect_.Width();
    int scaleY = newRect.Height() - baseRect_.Height();
    int offsetX = newRect.left - baseRect_.left;
    int offsetY = newRect.top - baseRect_.top;
    for (Children::const_iterator i = children_.begin(); i != children_.end(); ++i) {
      int left, top, right, bottom;
      left = (*i).baseRect.left + (int)((*i).anchor0.x * scaleX) + offsetX;
      top = (*i).baseRect.top + (int)((*i).anchor0.y * scaleY) + offsetY;
      right = (*i).baseRect.right + (int)((*i).anchor1.x * scaleX) + offsetX;
      bottom = (*i).baseRect.bottom + (int)((*i).anchor1.y * scaleY) + offsetY;
      ::DeferWindowPos(hdwp, (*i).hwnd, NULL, left, top, right - left, bottom - top, SWP_NOACTIVATE | SWP_NOZORDER);
    }
    ::EndDeferWindowPos(hdwp);
  }  
}