#pragma once


// label

class Label : public CStatic
{
  DECLARE_DYNAMIC(Label)

public:
  Label();
  virtual ~Label();

  void SetBkColor(COLORREF color);
  void SetTextColor(COLORREF color);

protected:
	DECLARE_MESSAGE_MAP()

private:
  COLORREF    textColor_;
  COLORREF    bkColor_;
  int         textAlign_;

public:
  virtual void PreSubclassWindow();
  virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};


