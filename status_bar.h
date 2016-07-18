#pragma once

#include <list>
#include "bitmap.h"

// StatusBar

class StatusBar : public CWnd {
	DECLARE_DYNAMIC(StatusBar)

public:
  enum {
    PANEL_ALIGN_LEFT,
    PANEL_ALIGN_RIGHT,
  };

	StatusBar();
	virtual ~StatusBar();

  BOOL Create(CWnd *parent, int place);
  
  void AddPanel(int id, LPCTSTR text, int icon, int align);
  void SetPanel(int id, LPCTSTR text, int icon);

protected:
  struct Panel {
    int id;
    CString text;
    Bitmap icon;
    int align;
  };
  typedef std::list<Panel> Panels;

  Panels  panels_;

	DECLARE_MESSAGE_MAP()

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnPaint();
};


