#include "stdafx.h"
#include "check_point_list_box.h"
#include "gui_resources.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CheckPointListBox, CListBox)

CheckPointListBox::CheckPointListBox()
  : imageList_(NULL)
  , itemHeight_(52)
  , spacing_(10) {
}

CheckPointListBox::~CheckPointListBox() {
}

void CheckPointListBox::SetItemHeight(int height) {
  itemHeight_ = height;

  CListBox::SetItemHeight(0, height);
}

void CheckPointListBox::SetSpacing(int spacing) {
  spacing_ = spacing;
}

void CheckPointListBox::AddItem(LPCTSTR text, int icon, int state) {
  ItemData *item = new ItemData;
  item->icon = icon;
  item->text = text;
  item->state = state;
  int index = CListBox::AddString(_T(""));
  SetItemData(index, (DWORD_PTR)item);
}

void CheckPointListBox::SetState(int index, int state) {
  ItemData *data = (ItemData *)GetItemData(index);
  if (data != NULL && data != (ItemData *)-1) {
    data->state = state;
    CRect rect;
    GetItemRect(index, &rect);
    InvalidateRect(rect, FALSE);
  }
}

void CheckPointListBox::SetImageList(CImageList *imageList) {
  imageList_ = imageList;

  Invalidate(FALSE);
}

void CheckPointListBox::DeleteAllItems() {
  while (GetCount() > 0) {
    DeleteString(0);
  }
}

BEGIN_MESSAGE_MAP(CheckPointListBox, CListBox)
  ON_WM_ERASEBKGND()
  ON_WM_DRAWITEM_REFLECT()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_DELETEITEM_REFLECT()
  ON_WM_KEYDOWN()
  ON_WM_KEYUP()
END_MESSAGE_MAP()

// CheckPointListBox message handlers
void CheckPointListBox::PreSubclassWindow() {
  CListBox::SetItemHeight(0, itemHeight_);

  CListBox::PreSubclassWindow();
}

BOOL CheckPointListBox::OnEraseBkgnd(CDC* pDC) {
  CRect rect;
  GetClientRect(&rect);
  pDC->FillSolidRect(rect, RGB(255, 255, 255));
  return TRUE;
}

void CheckPointListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {  
  ItemData *data = (ItemData *)lpDrawItemStruct->itemData;
  if (data == NULL || data == (ItemData *)-1) {
    return;
  }
  CDC *dc = CDC::FromHandle(lpDrawItemStruct->hDC);
  CRect rectItem(lpDrawItemStruct->rcItem);
  dc->FillSolidRect(rectItem, RGB(255, 255, 255));
  rectItem.DeflateRect(32, 0, 32, 0);
  int index = lpDrawItemStruct->itemID;
  CFont *oldFont = dc->SelectObject(GuiResources::GetInstance()->GetFont(GuiResources::FONT_BIG));  
  
  dc->SetBkMode(TRANSPARENT);

  int x = rectItem.right - 32 - 4;
  int y = rectItem.top + (rectItem.Height() - 32) / 2;
  if (imageList_ != NULL && data->state != -1) {
    imageList_->Draw(dc, data->state, CPoint(x, y), ILD_NORMAL);
  }

  dc->SetTextColor(RGB(0,0,0));

  if (imageList_ != NULL && data->icon != -1) {
    imageList_->Draw(dc, data->icon, CPoint(rectItem.left + 10, y), ILD_NORMAL);
  }

  CRect rectText(rectItem);
  rectText.left = rectItem.left + 32 + spacing_;
  dc->DrawText(data->text, rectText, DT_SINGLELINE | DT_VCENTER);

  dc->SelectObject(oldFont);

  dc->Draw3dRect(CRect(rectText.left, rectItem.bottom - 1, rectItem.right, rectItem.bottom),
      RGB(221,221,221), RGB(221,221,221));
}

void CheckPointListBox::OnLButtonDown(UINT nFlags, CPoint point) {
  return;
}

void CheckPointListBox::OnLButtonUp(UINT nFlags, CPoint point) {
  return;
}

void CheckPointListBox::DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct) {
  ItemData *item = (ItemData *)lpDeleteItemStruct->itemData;
  if (item != NULL && item != (ItemData *)-1) {
    delete item;
  } 
}


void CheckPointListBox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {  
}


void CheckPointListBox::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {  
}