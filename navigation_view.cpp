// navigation_view.cpp : implementation file
//

#include "stdafx.h"
#include "navigation_view.h"
#include "resource.h"
#include "gui_resources.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// NavigationView

IMPLEMENT_DYNAMIC(NavigationView, CListBox)

NavigationView::NavigationView() {
}

NavigationView::~NavigationView() {
}

void NavigationView::AddItem(int pos, LPCTSTR text, bool enabled) {
  ItemData *item = new ItemData;
  item->text = text;
  item->enabled = enabled;
  int index = InsertString(pos, _T(""));
  ASSERT(index == pos);
  SetItemData(index, (DWORD_PTR)item);
}

void NavigationView::AddItem(int pos, int textResId, bool enabled) {
  CString text;
  text.LoadString(textResId);
  AddItem(pos, text, enabled);
}

void NavigationView::DisableAllItems() {
  int count = GetCount();
  for (int i = 0; i < count; i++) {
    ItemData *item = (ItemData *)GetItemData(i);
    item->enabled = false;
  }
  Invalidate(FALSE);
}

void NavigationView::EnableItem(int pos, bool enabled) {
  ItemData *item = (ItemData *)GetItemData(pos);
  if (item->enabled != enabled) {
    item->enabled = enabled;
    CRect rect;
    GetItemRect(pos, &rect);
    InvalidateRect(rect);
  }
}

BEGIN_MESSAGE_MAP(NavigationView, CListBox)
  ON_WM_ERASEBKGND()
  ON_WM_DRAWITEM_REFLECT()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_DELETEITEM_REFLECT()
  ON_WM_KEYDOWN()
  ON_WM_KEYUP()
END_MESSAGE_MAP()

// NavigationView message handlers
void NavigationView::PreSubclassWindow() {
  CBitmap bitmap;
  bitmap.LoadBitmap(IDB_NAVIGATION_NUMBER);
  imageList_.Create(40, 40, ILC_COLOR32 | ILC_MASK, 14, 0);
  imageList_.Add(&bitmap, RGB(255, 0, 255));

  SetItemHeight(0, 58);

  CListBox::PreSubclassWindow();
}

BOOL NavigationView::OnEraseBkgnd(CDC* pDC) {
  CRect rect;
  GetClientRect(&rect);
  pDC->FillSolidRect(rect, RGB(65, 71, 77));
  return TRUE;
}

void NavigationView::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
  ItemData *item = (ItemData *)lpDrawItemStruct->itemData;
  if (item == NULL) {
    return;
  }
  CDC *dc = CDC::FromHandle(lpDrawItemStruct->hDC);  
  CRect rectItem(lpDrawItemStruct->rcItem);
  dc->FillSolidRect(rectItem, RGB(65, 71, 77));
  int index = lpDrawItemStruct->itemID;
  int curSel = GetCurSel();
  CFont *oldFont = dc->SelectObject(GuiResources::GetInstance()->GetFont(GuiResources::FONT_BIG));
  if (index == curSel) {
    dc->FillSolidRect(rectItem, RGB(43, 47, 52));

    POINT triangle[3];
    triangle[0].x = rectItem.right - 8;
    triangle[0].y = (rectItem.top + rectItem.bottom) / 2;
    triangle[1].x = rectItem.right;
    triangle[1].y = triangle[0].y - 8;
    triangle[2].x = rectItem.right;
    triangle[2].y = triangle[0].y + 8;
    CBrush brush;
    brush.CreateSolidBrush(RGB(255, 255, 255));
    CBrush *oldBrush = dc->SelectObject(&brush);
    CGdiObject *oldPen = dc->SelectStockObject(NULL_PEN);
    dc->Polygon(triangle, 3);
    dc->SelectObject(oldBrush);
    dc->SelectObject(oldPen);

    dc->SelectObject(GuiResources::GetInstance()->GetFont(GuiResources::FONT_BIG_BOLD));
  } else {
    dc->FillSolidRect(rectItem, RGB(65, 71, 77));
  }

  int x = rectItem.left + 8;
  int y = rectItem.top + (rectItem.Height() - 40) / 2;
  int iconIndex = 0;
  if (item->enabled) {
    if (index == curSel) {
      iconIndex = index;
    } else {
      iconIndex = 14;      
    }
    dc->SetTextColor(RGB(255, 255, 255));
  } else {
    iconIndex = index + 7;
    dc->SetTextColor(RGB(139, 140, 140));
  }

  imageList_.Draw(dc, iconIndex, CPoint(x, y), ILD_NORMAL);

  CRect rectText(rectItem);
  rectText.left = x + 40 + 6;
  dc->DrawText(item->text, rectText, DT_SINGLELINE | DT_VCENTER);

  dc->SelectObject(oldFont);
}

void NavigationView::SetCurSel(int index) {
  ItemData *item = (ItemData *)GetItemData(index);
  if (item->enabled) {
    int oldSel = GetCurSel();
    if (index != oldSel) {      
      CListBox::SetCurSel(index);
      if (oldSel >= 0) {
        CRect rectItem;
        GetItemRect(oldSel, &rectItem);
        InvalidateRect(rectItem);
      }
    }
  }
}

void NavigationView::OnLButtonDown(UINT nFlags, CPoint point) {
  BOOL outSide = TRUE;
  UINT index = ItemFromPoint(point, outSide);
  if (index != LB_ERR && !outSide) {
    int oldSel = GetCurSel();
    SetCurSel(index);
    int curSel = GetCurSel();
    if (oldSel != curSel) {
      CWnd *parent = GetParent();
      if (parent != NULL) {
        parent->SendMessage(WM_COMMAND, MAKELPARAM(GetDlgCtrlID(), LBN_SELCHANGE), (LPARAM)m_hWnd);
      }
    }
  }
  return;
}

void NavigationView::OnLButtonUp(UINT nFlags, CPoint point) {
  return;
}

void NavigationView::DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct) {
  ItemData *item = (ItemData *)lpDeleteItemStruct->itemData;
  if (item != NULL && item != (ItemData *)-1) {
    delete item;
  }
}


void NavigationView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
  
}


void NavigationView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
  
}
