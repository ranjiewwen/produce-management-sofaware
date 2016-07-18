#include "stdafx.h"
#include "gui_resources.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SINGLETON(GuiResources)

bool GuiResources::Init(){
  if (!CreateFonts()) {
    return false;
  }
  return true;
}

CFont *GuiResources::GetFont(int type) {
  ASSERT(type >= 0 && type < FONT_COUNT);
  return &fonts_[type];
}

bool GuiResources::CreateFonts() {
  HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
  if (hFont == NULL) {
    return false;
  }
  LOGFONT lf;
  if (GetObject(hFont, sizeof(lf), &lf) != sizeof(lf)) {
    return false;
  }
  
  // normal fonts
  if (!fonts_[FONT_NORMAL].CreateFontIndirect(&lf)) {
    return false;
  }
  lf.lfWeight = FW_BOLD;
  if (!fonts_[FONT_NORMAL_BOLD].CreateFontIndirect(&lf)) {
    return false;
  }

  LONG normalSize = lf.lfHeight;
  
  // small fonts
  lf.lfHeight = normalSize * 3 / 5;
  lf.lfWeight = FW_NORMAL;
  if (!fonts_[FONT_SMALL].CreateFontIndirect(&lf)) {
    return false;
  }

  // big fonts
  lf.lfHeight = normalSize * 5 / 3;
  lf.lfWeight = FW_NORMAL;
  _tcsncpy(lf.lfFaceName, _T("Î¢ÈíÑÅºÚ"), _countof(lf.lfFaceName));
  if (!fonts_[FONT_BIG].CreateFontIndirect(&lf)) {
    return false;
  }
  lf.lfWeight = FW_BOLD;
  if (!fonts_[FONT_BIG_BOLD].CreateFontIndirect(&lf)) {
    return false;
  }

  lf.lfWeight = FW_BOLD;

  return true;
}