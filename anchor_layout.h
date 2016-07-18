#pragma once

#include <list>

struct Anchor {
  float x, y;
};

class AnchorLayout {
public:
  static const Anchor TOP_LEFT;
  static const Anchor TOP_RIGHT;
  static const Anchor BOTTOM_RIGHT;
  static const Anchor BOTTOM_LEFT;
  static const Anchor CENTER;
  static const Anchor TOP_CENTER;

  AnchorLayout();
  virtual ~AnchorLayout();

  void Init(HWND hwndParent);
  void AddDlgItem(int id, const Anchor &anchor0, const Anchor &anchor1);
  void AddAnchor(HWND hwndChild, const Anchor &anchor0, const Anchor &anchor1);
  void RecalcLayout();

private:
  struct Child {
    HWND    hwnd;
    CRect   baseRect;
    Anchor  anchor0;
    Anchor  anchor1;
  };
  typedef std::list<Child> Children;

  HWND        hwndParent_;
  CRect       baseRect_;
  Children    children_;
};

