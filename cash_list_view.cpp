// CashListView.cpp : implementation file
//

#include "stdafx.h"
#include "cash_list_view.h"
#include "graphics.h"
#include "gui_resources.h"
#include "run_cash_monitor.h"
#include "run_cash_monitor_inl.h"
#include "resource.h"
#include "local_file_saver.h"
#include <Windows.h>
// CashListView

IMPLEMENT_DYNAMIC(CashListView, CStatic)

CashListView::CashListView()
: monitor_(NULL)
, selectedItem_(-1)
, topItem_(0)
, itemCount_(0) {
	
}

char* CashListView::szdir;
TCHAR CashListView::szPath[];
CashListView::~CashListView() {
}

void CashListView::SetMonitor(RunCashMonitor *monitor) {
	monitor_ = monitor;
}

void CashListView::Update() {
	if (monitor_ != NULL) {
		int count = monitor_->Freeze();
		if (count != itemCount_) {
			itemCount_ = count;
		}
		monitor_->Unfreeze();
		int selected = count - 1;
		EnsureItemVisiable(selected);
		SelectItemAndNotify(selected);
	}

	Invalidate(FALSE);
}

void CashListView::SelectItem(int index) {
	selectedItem_ = index;

	Invalidate(FALSE);
}

int CashListView::GetSelected() const {
	return selectedItem_;
}

BEGIN_MESSAGE_MAP(CashListView, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_GETDLGCODE()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

// CashListView message handlers
void CashListView::OnPaint() {
	CRect clientRect;
	GetClientRect(&clientRect);
	int clientWidth = clientRect.Width();

	BufferredPaintDC dc(this);

	dc.FillSolidRect(clientRect, RGB(255, 255, 255));

	CFont *oldFont = dc.SelectObject(GuiResources::GetInstance()->GetFont(GuiResources::FONT_NORMAL));
	dc.SetTextColor(RGB(0, 0, 0));
	dc.SetBkMode(TRANSPARENT);

	CString text[6];
	int width[6];
	text[0] = _T("序号"); // __XXXX__
	width[0] = 8 * clientWidth / 49;
	text[1] = _T("面额"); // __XXX__
	width[1] = 7 * clientWidth / 49;
	text[2] = _T("版本"); // __XX__
	width[2] = 6 * clientWidth / 49;
	text[3] = _T("结果"); // __XXX__
	width[3] = 7 * clientWidth / 49;
	text[4] = _T("方向"); // __X__
	width[4] = 5 * clientWidth / 49;
	text[5] = _T("冠字号"); // __XXXXXXXXXXXX__  
	width[5] = 16 * clientWidth / 49;

	int y = clientRect.top;
	CRect itemRect(clientRect.left, y, clientRect.right, y + itemHeight_);
	dc.FillSolidRect(itemRect, RGB(248, 248, 248));
	DrawRow(&dc, y, 0, text, width);

	if (monitor_ != NULL) {
		int count = monitor_->Freeze();
		for (int i = topItem_; i < count; i++) {
			y += itemHeight_;
			if (y > clientRect.bottom) {
				break;
			}
			itemRect.OffsetRect(0, itemHeight_);

			if (i == selectedItem_) {
				dc.FillSolidRect(itemRect, GetFocus() == this ? RGB(54, 133, 214) : RGB(128, 128, 128));
				dc.SetTextColor(RGB(255, 255, 255));
			}
			else {
				if (((i + 1) & 1) == 0) {
					dc.FillSolidRect(itemRect, RGB(248, 248, 248));
				}
				dc.SetTextColor(RGB(0, 0, 0));
			}

			Cash *cash = monitor_->GetCash(i);
			if (cash->Valid(Cash::VALID_INFO)) {
				text[0].Format(_T("%d"), cash->Id());
				if (cash->GetDenomination() <= 0) {
					text[1] = _T("?");
				}
				else {
					text[1].Format(_T("%d"), cash->GetDenomination());
				}
				if (cash->GetVersion() <= 0) {
					text[2] = _T("?");
				}
				else {
					text[2].Format(_T("%02d"), cash->GetVersion());
				}
				text[3].Format(_T("%03d"), cash->GetError());
				if (cash->GetDirection() <= 0) {
					text[4] = _T("?");
				}
				else {
					text[4].Format(_T("%d"), cash->GetDirection());
				}
				text[5] = cash->GetSN();
				DrawRow(&dc, y, i + 1, text, width);
			}
		}
		monitor_->Unfreeze();
	}

	dc.SelectObject(oldFont);
}

void CashListView::DrawRow(CDC *dc, int y, int index, CString text[6], int width[6]) {
	CRect clientRect;
	GetClientRect(&clientRect);
	CRect rect(clientRect.left, y, clientRect.right, y + itemHeight_);
	rect.right = rect.left;
	for (int i = 0; i < 6; i++) {
		rect.left = rect.right;
		rect.right += width[i];
		dc->DrawText(text[i], &rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_END_ELLIPSIS);
		dc->Draw3dRect(rect, RGB(236, 236, 236), RGB(236, 236, 236));
	}
}

void CashListView::PreSubclassWindow() {
	CWnd::PreSubclassWindow();

	CDC *dc = GetDC();
	CFont *oldFont = dc->SelectObject(GuiResources::GetInstance()->GetFont(GuiResources::FONT_NORMAL));
	itemHeight_ = dc->GetTextExtent(_T("Y")).cy * 2;
	dc->SelectObject(oldFont);
	ReleaseDC(dc);
}

BOOL CashListView::OnEraseBkgnd(CDC* pDC) {
	return TRUE;
}

void CashListView::EnsureItemVisiable(int index) {
	int newTopItem = topItem_;

	CRect clientRect;
	GetClientRect(&clientRect);

	if (index < topItem_) {
		newTopItem = __max(index, 0);
	}
	else {
		int count = 1 + (index - topItem_) + 1;
		int dist = count * itemHeight_ - clientRect.Height();
		if (dist > 0) {
			newTopItem = topItem_ + (dist + itemHeight_ - 1) / itemHeight_;
		}
	}

	SetTopItem(newTopItem);
}

void CashListView::OnLButtonDown(UINT nFlags, CPoint point) {
	SetFocus();

	CRect clientRect;
	GetClientRect(&clientRect);
	clientRect.top += itemHeight_;
	int index = (point.y - clientRect.top) / itemHeight_ + topItem_;
	if (index >= 0 && index < itemCount_) {
		EnsureItemVisiable(index);
		SelectItemAndNotify(index);
		UpdateWindow();
	}

	CWnd::OnLButtonDown(nFlags, point);
}

//int CALLBACK CashListView::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM pData)
//{
//	TCHAR curr[MAX_PATH];
//	switch (uMsg) {
//	case BFFM_INITIALIZED: // WParam is TRUE since you are passing a path. // It would be FALSE if you were passing a pidl.
//		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)szdir);
//		break;
//	case BFFM_SELCHANGED: // Set the status window to the currently selected path. 
//		if (SHGetPathFromIDList((LPITEMIDLIST)lParam, curr))
//		{
//			::SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)curr);
//		}
//		break;
//	} return 0;
//}

int CALLBACK CashListView::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{
	case BFFM_INITIALIZED:    //初始化消息
		////设置默认路径为lpData如'D:\'
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)szPath/*lpData*/);
		//设置选择文件夹对话框的标题  
		::SetWindowText(hwnd, TEXT("请先设置个工作目录"));
		break;
	case BFFM_SELCHANGED:    //选择路径变化，
	{
	 char curr[MAX_PATH];
	 SHGetPathFromIDList((LPCITEMIDLIST)lParam, (LPWSTR)curr);
	 //在STATUSTEXT区域显示当前路径  
	 ::SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)curr);
	}
		break;
	default:
		break;
	}
	return 0;
}
void CashListView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	SetFocus();

	CRect clientRect;
	GetClientRect(&clientRect);
	clientRect.top += itemHeight_;
	int index = (point.y - clientRect.top) / itemHeight_ + topItem_;
	//	if (index >= 0 && index < itemCount_) 
	{
		EnsureItemVisiable(index);
		SelectItemAndNotify(index);
		UpdateWindow();

		//	if (monitor_ != NULL)
		{
			CMenu menu;
			menu.LoadMenu(IDR_POPUP_SAVE_WAVE);
			CMenu *contextMenu = menu.GetSubMenu(0);
			ClientToScreen(&point);
			int choice = contextMenu->TrackPopupMenu(TPM_RETURNCMD, point.x, point.y, this);
			/*if (choice == ID_SAVE_WAVE)
			{			
				BROWSEINFO bi;
				bi.hwndOwner = NULL;
				bi.pidlRoot = NULL; 
				bi.pszDisplayName = NULL;
				bi.lpszTitle = _T("请选择用于保存的文件夹");
				bi.ulFlags = 0;
				bi.lpfn = 0;
				bi.iImage = 0;
				LPCITEMIDLIST pidl = SHBrowseForFolder(&bi);
				if (pidl)
				{
					TCHAR folderName[MAX_PATH];
					SHGetPathFromIDList(pidl, folderName);
					
					int count = monitor_->Freeze();
					Cash *cash = monitor_->GetCash(index);
					if (cash->Valid(Cash::VALID_ADC))
					{
						if (LocalFileSaver::GetInstance()->SaveADCData(cash, folderName))
						{
							AfxMessageBox(IDS_PROMPT_SAVE_WAVE_SUCCESS, MB_ICONINFORMATION | MB_OK);
						}
						else
						{
							AfxMessageBox(IDS_PROMPT_SAVE_WAVE_FAILED, MB_ICONINFORMATION | MB_OK);
						}
					}
					else
					{
						AfxMessageBox(IDS_PROMPT_INVALID_WAVE, MB_ICONWARNING | MB_OK);
					}
					monitor_->Unfreeze();
				}
			}*/

			if (choice == ID_SAVE_WAVE)
			{
				filePath=AfxGetApp()->GetProfileString(_T("Recently"), _T("lastPath"), _T(""));
				_tcscpy(szPath, filePath);  //CString转为TCHAR数组
			
				TCHAR folderName[MAX_PATH];			
				LPCITEMIDLIST pidl = NULL;
				BROWSEINFO bi;
				bi.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
				bi.pidlRoot = NULL;
				bi.pszDisplayName = folderName;
				bi.lpszTitle = _T("请选择用于保存的文件夹");
				bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;;
				bi.lpfn = BrowseCallbackProc;    //回调函数  
				bi.iImage = 0;
				bi.lParam =0 /*long(&szPath)*/;  //设置默认路径，传给回掉函数的参数

				pidl = SHBrowseForFolder(&bi);
				if (pidl)
				{		
					SHGetPathFromIDList(pidl, szPath);
					//filePath = szPath;  //先定义成数组名是常量，不可以修改
					AfxGetApp()->WriteProfileString(_T("Recently"), _T("lastPath"), /*filePath*/szPath);

					/*int count = monitor_->Freeze();
					Cash *cash = monitor_->GetCash(index);
					if (cash->Valid(Cash::VALID_ADC))
					{
					if (LocalFileSaver::GetInstance()->SaveADCData(cash, folderName))
					{
					AfxMessageBox(IDS_PROMPT_SAVE_WAVE_SUCCESS, MB_ICONINFORMATION | MB_OK);
					}
					else
					{
					AfxMessageBox(IDS_PROMPT_SAVE_WAVE_FAILED, MB_ICONINFORMATION | MB_OK);
					}
					}
					else
					{
					AfxMessageBox(IDS_PROMPT_INVALID_WAVE, MB_ICONWARNING | MB_OK);
					}
					monitor_->Unfreeze();*/
				}
			}

		}
	}
	CWnd::OnRButtonDown(nFlags, point);
}



void CashListView::SelectItemAndNotify(int index) {
	int oldSel = GetSelected();
	SelectItem(index);
	int curSel = GetSelected();
	if (oldSel != curSel) {
		CWnd *parent = GetParent();
		if (parent != NULL) {
			parent->SendMessage(WM_COMMAND, MAKELPARAM(GetDlgCtrlID(), LBN_SELCHANGE), (LPARAM)m_hWnd);
		}
	}
}


void CashListView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	int curSel = GetSelected();

	if (nChar == VK_UP) {
		if (curSel <= 0) {
			return;
		}
		curSel--;
	}
	else if (nChar == VK_DOWN) {
		if (curSel >= itemCount_ - 1) {
			return;
		}
		curSel++;
	}

	EnsureItemVisiable(curSel);
	SelectItemAndNotify(curSel);
	UpdateWindow();

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}


BOOL CashListView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
	int scroll = zDelta / WHEEL_DELTA;

	SetTopItem(topItem_ - scroll);

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CashListView::SetTopItem(int index) {
	CRect clientRect;
	GetClientRect(&clientRect);
	clientRect.top += itemHeight_; // exclude header row.
	int pageCount = clientRect.Height() / itemHeight_;
	if (index > itemCount_ - pageCount) {
		index = itemCount_ - pageCount;
	}
	if (index < 0) {
		index = 0;
	}
	if (topItem_ != index) {
		topItem_ = index;

		Invalidate(FALSE);
	}
}

UINT CashListView::OnGetDlgCode() {
	return DLGC_WANTARROWS;
}


void CashListView::OnKillFocus(CWnd* pNewWnd) {
	CWnd::OnKillFocus(pNewWnd);

	Invalidate(FALSE);
}


void CashListView::OnSetFocus(CWnd* pOldWnd) {
	CWnd::OnSetFocus(pOldWnd);

	Invalidate(FALSE);
}



