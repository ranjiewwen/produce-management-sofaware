#pragma once

// NavigationView
class NavigationView : public CListBox
{
	DECLARE_DYNAMIC(NavigationView)

public:
  NavigationView();
  virtual ~NavigationView();

  void AddItem(int pos, LPCTSTR text, bool enabled);
  void AddItem(int pos, int textResId, bool enabled);
  void DisableAllItems();
  void EnableItem(int pos, bool enabled);
  void SetCurSel(int index);

protected:
  struct ItemData {
    CString   text;
    bool      enabled;
  };
  CImageList  imageList_;

  virtual void PreSubclassWindow();

public:
  DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void DeleteItem(LPDELETEITEMSTRUCT /*lpDeleteItemStruct*/);
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
};


