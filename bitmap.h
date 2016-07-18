#pragma once

class Bitmap {
public:
  Bitmap();
  Bitmap(const Bitmap &other);
  virtual ~Bitmap();

  bool Create(int width, int height, int bpp, const RGBQUAD *palette);
  bool CreateGrayBitmap(int width, int height);
  void Destroy();
  bool IsValid() const;
  void Assign(const Bitmap &other);
  Bitmap &operator=(const Bitmap &other);
  bool Draw(HDC hdc, int x, int y);
  bool Draw(HDC hdc, const CRect &rect);
  bool Draw(HDC hdc, int srcX, int srcY, int cx, int cy, int dstX, int dstY);
  bool PreMultiplyAlpha();
  bool LoadFromResource(int resId);
  int GetWidth() const;
  int GetHeight() const;
  int GetBitsPerPixel() const;
  bool SetWithAlpha();
  void *GetBits() const;
  bool SaveToFile(LPCTSTR fileName);
  int GetBytesPerLine() const;

private:
  struct PixelBuffer {
    int width;
    int height;
    int bpp;
    void *bits;
    int refCount;
    bool withAlpha;
    HANDLE handle;
  };
  PixelBuffer   *buffer_;  
};
