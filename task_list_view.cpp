// task_list_view.cpp : implementation file
//

#include "stdafx.h"
#include "task_list_view.h"
#include "gui_resources.h"
#include "resource.h"

// TaskListView

IMPLEMENT_DYNAMIC(TaskListView, CListBox)

TaskListView::TaskListView() {
  dotPen_.CreatePen(PS_DOT, 1, RGB(222, 222, 222));
}

TaskListView::~TaskListView() {
}

int TaskListView::AddTask(LPCTSTR name, TaskStatus status) {
  TaskItem *item = new TaskItem;
  item->name = name;
  item->status = status;
  int index = AddString(_T(""));
  SetItemData(index, (DWORD_PTR)item);
  return index;
}

void TaskListView::UpdateTaskStatus(int index, TaskStatus status) {
  CRect rectItem;
  GetItemRect(index, &rectItem);
  TaskItem *item = (TaskItem *)GetItemData(index);
  item->status = status;
  InvalidateRect(rectItem, FALSE);
}

void TaskListView::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) {
  CDC *dc = CDC::FromHandle(lpDrawItemStruct->hDC);
  CRect rectItem(lpDrawItemStruct->rcItem);
  dc->FillSolidRect(rectItem, RGB(248, 248, 248));
  TaskItem *item = (TaskItem *)lpDrawItemStruct->itemData;
  if (item == NULL || item == (TaskItem *)-1) {
    return;
  }
  CFont *font = GuiResources::GetInstance()->GetFont(GuiResources::FONT_NORMAL);
  CFont *oldFont = dc->SelectObject(font);  
  dc->SetTextColor(RGB(0, 0, 0));
  dc->DrawText(item->name, rectItem, DT_SINGLELINE | DT_VCENTER);
  int left = 0, right = rectItem.right;
  CSize textSize = dc->GetTextExtent(item->name);
  left += textSize.cx + 4;
  if (item->status == STATUS_PROCESSING) {
    CString processing;
    processing.LoadString(IDS_PROCESSING);
    dc->DrawText(processing, rectItem, DT_SINGLELINE | DT_VCENTER | DT_RIGHT);
    textSize = dc->GetTextExtent(processing);
    right -= textSize.cx + 4;
  } else if (item->status == STATUS_COMPLETED) {

  } else if (item->status == STATUS_ERROR) {

  }
  dc->SelectObject(oldFont);

  if (left < right) {
    int centerY = (rectItem.top + rectItem.bottom) / 2;
    CPen *oldPen = dc->SelectObject(&dotPen_);
    dc->MoveTo(left, centerY);
    dc->LineTo(right, centerY);
    dc->SelectObject(oldPen);
  }
}

BEGIN_MESSAGE_MAP(TaskListView, CListBox)
  ON_WM_DELETEITEM_REFLECT()
  ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// TaskListView message handlers
void TaskListView::DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct) {
  if (lpDeleteItemStruct->itemData != NULL && lpDeleteItemStruct->itemData != -1) {
    delete (TaskItem *)lpDeleteItemStruct->itemData;
  }
}

BOOL TaskListView::OnEraseBkgnd(CDC* pDC) {
  return TRUE;
}

void TaskListView::PreSubclassWindow() {
  CListBox::PreSubclassWindow();

  CFont *font = GuiResources::GetInstance()->GetFont(GuiResources::FONT_NORMAL);
  CDC *dc = GetDC();
  CFont *oldFont = dc->SelectObject(font);
  TEXTMETRIC textMetrics;
  dc->GetTextMetrics(&textMetrics);
  dc->SelectObject(oldFont);
  ReleaseDC(dc);

  SetItemHeight(0, textMetrics.tmAscent + textMetrics.tmDescent + 12);
}
