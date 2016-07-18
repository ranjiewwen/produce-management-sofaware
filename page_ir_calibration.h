#pragma once

#include "page.h"
#include "resource.h"
#include "button.h"
#include "label.h"
#include "anchor_layout.h"
#include "ir_calibration.h"
#include "thread.h"
#include "bitmap.h"

// class IRStateListView
class IRStateListView : public CStatic {
public:
  IRStateListView();
  virtual ~IRStateListView();

  void SetStates(const IRState *states);

private:
  const IRState   *states_;

  void DrawCell(CDC &dc, const CRect &rect, const CString &text, COLORREF bgColor, COLORREF foreColor);

  DECLARE_MESSAGE_MAP()

  afx_msg void OnPaint();
  afx_msg BOOL OnEraseBkgnd(CDC *pDC);
};

// PageIRCalibration dialog
class PageIRCalibration : public Page {
	DECLARE_DYNAMIC(PageIRCalibration)

public:
	PageIRCalibration(CWnd* pParent = NULL);   // standard constructor
	virtual ~PageIRCalibration();

// Dialog Data
	enum { IDD = IDD_PAGE_IR_CALIBRATION };

  virtual void OnEnter(int leavePageId);
  virtual bool OnLeave(int enterPageId);

protected:
  enum {
    STEP_FIRST,
    STEP_NEXT,
  };
  IRStateListView resultView_;
  AnchorLayout    layout_;
  IRCalibration   calibration_;
  Thread          thread_;
  bool            adjustSuccessed_;
  int             adjustError_;
  int             nextStep_;
  Bitmap          redCircle_;
  Bitmap          greenCircle_;
  Bitmap          sketchMap_;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  void StartAdjust();
  bool Adjust();
  DWORD AdjustAsync();
  void ShowAdjustResult();
  virtual void DrawClient(CDC &dc, const CRect &rect);

	afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg LRESULT OnAdjustDone(WPARAM, LPARAM);
  afx_msg void OnBnClickedButtonStep();

  DECLARE_MESSAGE_MAP()
};
