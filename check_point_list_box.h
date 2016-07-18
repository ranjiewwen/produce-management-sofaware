#pragma once

// CheckPointListBox
class CheckPointListBox : public CListBox
{
	DECLARE_DYNAMIC(CheckPointListBox)

public:
  CheckPointListBox();
  virtual ~CheckPointListBox();

  void AddItem(LPCTSTR text, int icon, int state);
  void SetState(int index, int state);
  void DeleteAllItems();
  void SetImageList(CImageList *imageList);
  void SetItemHeight(int height);
  void SetSpacing(int spacing);

protected:
  struct ItemData {
    int       icon;
    CString   text;
    int       state;
  };
  CImageList  *imageList_;
  int         itemHeight_;
  int         spacing_;

  virtual void PreSubclassWindow();

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void DeleteItem(LPDELETEITEMSTRUCT /*lpDeleteItemStruct*/);
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

  DECLARE_MESSAGE_MAP()
};