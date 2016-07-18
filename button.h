#pragma once

#include "bitmap.h"

// Button
enum ButtonState {
  BUTTON_STATE_NORMAL,
  BUTTON_STATE_DOWN,
  BUTTON_STATE_OVER,
};

class ButtonStyle {
public:
  COLORREF  borderColor;
  COLORREF  bgColor;
  COLORREF  textColor;
  Bitmap    bitmap;

  ButtonStyle() : borderColor(CLR_NONE), bgColor(RGB(255, 255, 255)), textColor(RGB(0, 0, 0)) {}

  ButtonStyle(COLORREF _bgColor, COLORREF _textColor, const Bitmap &_bitmap, COLORREF _borderColor = CLR_NONE)
  : borderColor(_borderColor), bgColor(_bgColor), textColor(_textColor), bitmap(_bitmap) {}

  ButtonStyle(COLORREF _bgColor, COLORREF _textColor, COLORREF _borderColor = CLR_NONE)
  : borderColor(_borderColor), bgColor(_bgColor), textColor(_textColor) {}
};

class Button : public CButton {
	DECLARE_DYNAMIC(Button)

public:
	Button();
	virtual ~Button();

  void SetStyle(ButtonState state, const ButtonStyle &style);

protected:  
  ButtonStyle styles_[3];
  bool        mouseOn_;

	DECLARE_MESSAGE_MAP()

  virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
public:
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnMouseLeave();
};


