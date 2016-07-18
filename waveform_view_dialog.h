#pragma once

#include "resource.h"
#include "bitmap.h"
#include "anchor_layout.h"
#include "label.h"
#include "button.h"

class RunCashMonitor;

// WaveformViewDialog dialog
class WaveformViewDialog : public CDialog
{
	DECLARE_DYNAMIC(WaveformViewDialog)

public:
	WaveformViewDialog(RunCashMonitor *monitor, int cashIndex, int channel, CWnd* pParent = NULL);   // standard constructor
	virtual ~WaveformViewDialog();

  int GetSelectedCashIndex() const;

// Dialog Data
	enum { IDD = IDD_WAVEFORM_VIEW };

protected:
  int               cashIndex_;
  int               channel_;
  AnchorLayout      layout_;
  Label             labelTitle_;
  Button            closeButton_;
  RunCashMonitor    *monitor_;

  void SelectCash(int index);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

  afx_msg void OnPaint();
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnBnClickedButtonClose();
  afx_msg void OnSize(UINT nType, int cx, int cy);

public:
  virtual BOOL OnInitDialog();
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  virtual BOOL PreTranslateMessage(MSG* pMsg);
};
