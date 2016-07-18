#pragma once

#include "singleton.h"

class GuiResources {
public:
  enum {
    FONT_NORMAL,
    FONT_NORMAL_BOLD,
    FONT_BIG,
    FONT_SMALL,
    FONT_BIG_BOLD,
    FONT_COUNT,
  }; 

  bool Init();
  CFont *GetFont(int type);

private:
  CFont   fonts_[FONT_COUNT];

  bool CreateFonts();

  DECLARE_SINGLETON(GuiResources)
};

