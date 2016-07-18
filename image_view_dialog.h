#pragma once

#include "resource.h"
#include "bitmap.h"
#include "anchor_layout.h"
#include "label.h"
#include "button.h"

class RunCashMonitor;

// ImageViewDialog dialog
class ImageViewDialog : public CDialog
{
	DECLARE_DYNAMIC(ImageViewDialog)

public:
	ImageViewDialog(RunCashMonitor *monitor, int cashIndex, CWnd* pParent = NULL);   // standard constructor
	virtual ~ImageViewDialog();

  int GetSelectedCashIndex() const;

// Dialog Data
	enum { IDD = IDD_IMAGE_VIEW };

protected:
  int               cashIndex_;
  Bitmap            topImage_;
  Bitmap            bottomImage_;
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

  void DrawImage(CDC *dc, Bitmap *image, const CRect &rect);

public:
  virtual BOOL OnInitDialog();
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  virtual BOOL PreTranslateMessage(MSG* pMsg);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
};
