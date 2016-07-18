#pragma once

enum {
  HEADER_STYLE_INFO,
  HEADER_STYLE_ERROR,
  HEADER_STYLE_OK,
};

struct HeaderButton {
  int id;
  int textResId;
};

class Button;

class Page : public CDialog {
	DECLARE_DYNAMIC(Page)

public:
  Page(UINT templateId, CWnd* pParent = NULL);   // standard constructor
	virtual ~Page();

  virtual void OnEnter(int leavePageId);
  virtual bool OnLeave(int enterPageId);

  COLORREF GetBgColor() { return RGB(255, 255, 255); }

  void SetHeaderText(int textResId);
  void SetHeaderSubText(int textResId);
  void SetHeaderButtons(HeaderButton *buttons, int count);
  void SetHeaderInfo(int style, int textResId, int subTextResId);
  void SetHeaderInfo(int style, LPCTSTR text, LPCTSTR subText);
  void SetHeaderButtonText(int id, int textResId);
  void ShowHeaderButton(int id, bool visibility);
  void ShowHeader(bool visibility);

  bool ShowPage(int pageId);
  bool GotoStep(int step);
  void EnableConnectionCheck(bool enabled);

protected:
  struct Header {
    int           icon;
    CString       text;
    CString       subText;
    COLORREF      bgColor;
    COLORREF      textColor;
    Button        *buttons[3];
    bool          visible;
    int           height;
  };
  enum {
    ICON_INFO,
    ICON_OK,
    ICON_ERROR,  
  };

  Header              header_;
  static CImageList   imageList_;

  void DrawHeader(CDC &dc);
  virtual void DrawClient(CDC &dc, const CRect &rect);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  virtual BOOL OnInitDialog();
  void LayoutHeaderButtons();
  virtual void OnOK();
  virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

  afx_msg void OnPaint();
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnSize(UINT nType, int cx, int cy);
};
