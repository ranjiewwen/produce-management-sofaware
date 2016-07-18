#pragma once

// class BufferredPaintDC
class BufferredPaintDC : public CDC {
public:
  BufferredPaintDC(CWnd *pWnd);
  virtual ~BufferredPaintDC();

private:
  CPaintDC    paintDC_;
  CRect       paintRect_;
  CBitmap     memBitmap_;
  CBitmap     *oldBitmap_;
};

void DrawWaveform(CDC *dc, const CRect &rect, const short *value, 
    int minValue, int maxValue, int count, CPen *pen);