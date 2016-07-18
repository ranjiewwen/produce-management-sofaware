#include "stdafx.h"
#include "graphics.h"

void DrawWaveform(CDC *dc, const CRect &rect, const short *value, 
    int minValue, int maxValue, int count, CPen *pen) {
  ASSERT(dc != NULL);
  //ASSERT(minValue < maxValue);
  if (minValue >= maxValue) {
    return;
  }
  ASSERT(pen != NULL);

  if (count == 0) {
    return;
  }
  int height = rect.Height();
  int width = rect.Width();
  int range = maxValue - minValue;
  CPen *oldPen = dc->SelectObject(pen);
  dc->MoveTo(rect.left, rect.bottom - (value[0] - minValue) * height / range);
  for (int i = 1; i < count; i++) {
    dc->LineTo(rect.left + i * width / count, 
        rect.bottom - (value[i] - minValue) * height / range);
  }
  dc->SelectObject(oldPen);
}

// BufferredPaintDC
BufferredPaintDC::BufferredPaintDC(CWnd *pWnd)
  : paintDC_(pWnd) {
  paintRect_.SetRectEmpty();
  //paintDC_.GetClipBox(&paintRect_);
  if (paintRect_.IsRectEmpty()) {
    pWnd->GetClientRect(&paintRect_);
  }
  CreateCompatibleDC(&paintDC_);
  memBitmap_.CreateCompatibleBitmap(&paintDC_, paintRect_.Width(), paintRect_.Height());
  oldBitmap_ = SelectObject(&memBitmap_);
  SetViewportOrg(paintRect_.left, paintRect_.top);
}

BufferredPaintDC::~BufferredPaintDC() {
  paintDC_.BitBlt(paintRect_.left, paintRect_.top, 
      paintRect_.Width(), paintRect_.Height(), this, 0, 0, SRCCOPY);
  SelectObject(oldBitmap_);
}