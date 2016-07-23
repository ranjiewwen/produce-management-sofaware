#pragma once

#include "resource.h"
#include "anchor_layout.h"
#include "label.h"
#include "button.h"
#include "cash_list_view.h"

// RunCashDialog dialog
class RunCashMonitor;

class RunCashDialog : public CDialog
{
	DECLARE_DYNAMIC(RunCashDialog)

public:
	RunCashDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~RunCashDialog();

    virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_RUN_CASH };

protected:
  HICON             m_hIcon;
  RunCashMonitor    *monitor_;
  AnchorLayout      layout_;
  Label             labelTitle_;
  Button            closeButton_;
  int               currentCashIndex_;
  CashListView      cashListView_;

  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  void DrawWaveform(CDC *dc, const CRect &rect, const short *codes, const short *values, int minValue, int maxValue, int count);
  
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnPaint();
  afx_msg void OnClose();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnBnClickedButtonClose();
  afx_msg LRESULT OnUpdate(WPARAM, LPARAM);
  afx_msg void OnLbnSelchangeListCash();
  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
public:
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg  void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
 
};
