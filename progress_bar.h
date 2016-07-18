#pragma once


// ProgressBar
class ProgressBar : public CStatic
{
	DECLARE_DYNAMIC(ProgressBar)

public:
	ProgressBar();
	virtual ~ProgressBar();

  void SetRange(int minPos, int maxPos);
  void SetPos(int pos);

protected:
  int   minPos_, maxPos_;
  int   pos_;

	DECLARE_MESSAGE_MAP()

  afx_msg void OnPaint();
  afx_msg BOOL OnEraseBkgnd(CDC *pDC);
};


