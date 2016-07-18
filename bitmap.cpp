#include "stdafx.h"
#include "bitmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Bitmap::Bitmap() : buffer_(NULL) {
}

Bitmap::Bitmap(const Bitmap &other) : buffer_(NULL) {
  Assign(other);
}

Bitmap::~Bitmap() {
  Destroy();
}

bool Bitmap::Create(int width, int height, int bpp, const RGBQUAD *palette) {
  char biBuffer[sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD)];
  BITMAPINFO *bi = (BITMAPINFO *)biBuffer;
  ZeroMemory(bi, sizeof(BITMAPINFO));
  bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi->bmiHeader.biWidth = width;
  bi->bmiHeader.biHeight = height;
  bi->bmiHeader.biBitCount = bpp;
  bi->bmiHeader.biPlanes = 1;
  bi->bmiHeader.biCompression = BI_RGB;
  if (bpp <= 8) {
    ASSERT(palette != NULL);
    CopyMemory(bi->bmiColors, palette, (1 << bpp) * sizeof(RGBQUAD));
  }
  void *bits = NULL;
  HBITMAP hBitmap = CreateDIBSection(NULL, bi, DIB_RGB_COLORS, &bits, NULL, 0);
  if (hBitmap == NULL) {
    return false;
  }

  Destroy();

  buffer_ = (PixelBuffer *)malloc(sizeof(PixelBuffer));
  buffer_->width = width;
  buffer_->height = height;
  buffer_->bpp = bpp;
  buffer_->bits = bits;
  buffer_->refCount = 1;
  buffer_->handle = hBitmap;
  buffer_->withAlpha = false;

  return true;
}

bool Bitmap::CreateGrayBitmap(int width, int height) {
  RGBQUAD palette[256];
  for (int i = 0; i < 256; i++) {
    palette[i].rgbRed = i;
    palette[i].rgbGreen = i;
    palette[i].rgbBlue = i;
    palette[i].rgbReserved = 0;
  }
  return Create(width, height, 8, palette);
}

void Bitmap::Destroy() {
  if (buffer_ != NULL) {
    buffer_->refCount--;
    if (buffer_->refCount == 0) {
      DeleteObject(buffer_->handle);
      free(buffer_);
    }
    buffer_ = NULL;
  }
}

bool Bitmap::IsValid() const {
  return buffer_ != NULL;
}

void Bitmap::Assign(const Bitmap &other) {
  if (&other == this) {
    return;
  }

  Destroy();
  
  buffer_ = other.buffer_;
  if (buffer_ != NULL) {
    buffer_->refCount++;
  }
}

Bitmap &Bitmap::operator=(const Bitmap &other) {
  Assign(other);
  return *this;
}

bool Bitmap::Draw(HDC hdc, int x, int y) {
  if (buffer_ == NULL) {
    SetLastError(ERROR_INVALID_STATE);
    return false;
  }
  HDC hBitmapDC = CreateCompatibleDC(hdc);
  if (hBitmapDC == NULL) {
    return false;
  }
  bool ret = false;
  HBITMAP oldBitmap = (HBITMAP)SelectObject(hBitmapDC, buffer_->handle);
  if (oldBitmap != NULL) {
    if (buffer_->withAlpha) {
      BLENDFUNCTION bf;
      bf.BlendOp = AC_SRC_OVER;
      bf.BlendFlags = 0;
      bf.SourceConstantAlpha = 255;
      bf.AlphaFormat = AC_SRC_ALPHA;
      ret = !!AlphaBlend(hdc, x, y, buffer_->width, buffer_->height,
          hBitmapDC, 0, 0, buffer_->width, buffer_->height, bf);
    } else {
      ret = !!BitBlt(hdc, x, y, buffer_->width, buffer_->height, hBitmapDC, 0, 0, SRCCOPY);
    }
    SelectObject(hBitmapDC, oldBitmap);
  }
  DeleteDC(hBitmapDC);

  return ret;
}

bool Bitmap::Draw(HDC hdc, const CRect &rect) {
  if (buffer_ == NULL) {
    SetLastError(ERROR_INVALID_STATE);
    return false;
  }
  HDC hBitmapDC = CreateCompatibleDC(hdc);
  if (hBitmapDC == NULL) {
    return false;
  }
  bool ret = false;
  HBITMAP oldBitmap = (HBITMAP)SelectObject(hBitmapDC, buffer_->handle);
  if (oldBitmap != NULL) {
    /*double scaleX = (double)rect.Width() / buffer_->width;
    double scaleY = (double)rect.Height() / buffer_->height;
    double scale = __min(scaleX, scaleY);
    int width = (int)(buffer_->width * scale);
    int height = (int)(buffer_->height * scale);
    int left = rect.left + (rect.Width() - width) / 2;
    int top = rect.top + (rect.Height() - height) / 2;*/
    int width = rect.Width();
    int height = rect.Height();
    int left = rect.left;
    int top = rect.top;
    if (buffer_->withAlpha) {
      BLENDFUNCTION bf;
      bf.BlendOp = AC_SRC_OVER;
      bf.BlendFlags = 0;
      bf.SourceConstantAlpha = 255;
      bf.AlphaFormat = AC_SRC_ALPHA;
      ret = !!AlphaBlend(hdc, left, top, width, height,
        hBitmapDC, 0, 0, buffer_->width, buffer_->height, bf);
    }
    else {
      ret = !!StretchBlt(hdc, left, top, width, height, hBitmapDC, 
        0, 0, buffer_->width, buffer_->height, SRCCOPY);
    }
    SelectObject(hBitmapDC, oldBitmap);
  }
  DeleteDC(hBitmapDC);

  return ret;
}

bool Bitmap::Draw(HDC hdc, int srcX, int srcY, int cx, int cy, int dstX, int dstY) {
  if (buffer_ == NULL) {
    SetLastError(ERROR_INVALID_STATE);
    return false;
  }
  HDC hBitmapDC = CreateCompatibleDC(hdc);
  if (hBitmapDC == NULL) {
    return false;
  }
  bool ret = false;
  HBITMAP oldBitmap = (HBITMAP)SelectObject(hBitmapDC, buffer_->handle);
  if (oldBitmap != NULL) {
    if (buffer_->withAlpha) {
      BLENDFUNCTION bf;
      bf.BlendOp = AC_SRC_OVER;
      bf.BlendFlags = 0;
      bf.SourceConstantAlpha = 255;
      bf.AlphaFormat = AC_SRC_ALPHA;
      ret = !!AlphaBlend(hdc, dstX, dstY, cx, cy,
          hBitmapDC, srcX, srcY, cx, cy, bf);
    } else {
      ret = !!BitBlt(hdc, dstX, dstY, cx, cy, hBitmapDC, srcX, srcY, SRCCOPY);
    }
    SelectObject(hBitmapDC, oldBitmap);
  }
  DeleteDC(hBitmapDC);

  return ret;
}

bool Bitmap::PreMultiplyAlpha() {
  if (buffer_ == NULL || buffer_->bpp != 32) {
    SetLastError(ERROR_INVALID_STATE);
    return false;
  }
  unsigned char *p = (unsigned char *)buffer_->bits;
  for (int y = 0; y < buffer_->height; y++) {
    for (int x = 0; x < buffer_->width; x++) {
      unsigned char a = p[3];
      p[1] = p[1] * a / 255;
      p[2] = p[2] * a / 255;
      p[0] = p[0] * a / 255;
      p += 4;
    }
  }
  return true;
}

bool Bitmap::SetWithAlpha() {
  if (buffer_ == NULL || buffer_->bpp != 32) {
    SetLastError(ERROR_INVALID_STATE);
    return false;
  }
  buffer_->withAlpha = true;
  return true;
}

bool Bitmap::LoadFromResource(int resId) {
  HRSRC hResInfo = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(resId), RT_BITMAP);
  if (hResInfo == NULL) {
    return false;
  }
  int size = SizeofResource(AfxGetResourceHandle(), hResInfo);
  if (size < sizeof(BITMAPINFOHEADER)) {
    SetLastError(ERROR_INVALID_DATA);
    return false;
  }
  HGLOBAL hResData = LoadResource(AfxGetResourceHandle(), hResInfo);
  if (hResData == NULL) {
    return false;
  }  
  BITMAPINFOHEADER *bih = (BITMAPINFOHEADER *)LockResource(hResData);
  if (bih == NULL) {
    return false;
  }
  if (!Create(bih->biWidth, bih->biHeight, bih->biBitCount, NULL)) {
    UnlockResource(hResData);
    return false;
  }
  char *pixels = (char *)(bih + 1);
  if (bih->biBitCount <= 8) {
    pixels += (1 << bih->biBitCount) * sizeof(RGBQUAD);
  }
  memcpy(buffer_->bits, pixels, (bih->biBitCount * bih->biWidth + 31) / 32 * 4 * bih->biHeight);

  UnlockResource(hResData);

  if (bih->biBitCount == 32) {
    SetWithAlpha();
    PreMultiplyAlpha();
  }  

  return true;
}

int Bitmap::GetWidth() const {
  return buffer_ != NULL ? buffer_->width : 0;
}

int Bitmap::GetHeight() const {
  return buffer_ != NULL ? buffer_->height : 0;
}

int Bitmap::GetBitsPerPixel() const {
  return buffer_ != NULL ? buffer_->bpp : 0;
}

void *Bitmap::GetBits() const {
  return buffer_ != NULL ? buffer_->bits : NULL;
}

bool Bitmap::SaveToFile(LPCTSTR fileName) {
  try {
    CFile file(fileName, CFile::typeBinary | CFile::modeCreate | CFile::modeWrite);
    
    int paletteSize = 0;
    if (GetBitsPerPixel() <= 8) {
      paletteSize = (1 << GetBitsPerPixel());
    }
    BITMAPFILEHEADER bfh;
    ZeroMemory(&bfh, sizeof(bfh));
    bfh.bfType = 0x4D42;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + paletteSize * sizeof(RGBQUAD);
    int imageSize = GetBytesPerLine() * GetHeight();
    bfh.bfSize = bfh.bfOffBits + imageSize;
    file.Write(&bfh, sizeof(bfh));

    BITMAPINFOHEADER bih;
    ZeroMemory(&bih, sizeof(bih));
    bih.biSize = sizeof(bih);
    bih.biBitCount = GetBitsPerPixel();
    bih.biCompression = BI_RGB;
    bih.biPlanes = 1;
    bih.biWidth = GetWidth();
    bih.biHeight = GetHeight();
    file.Write(&bih, sizeof(bih));

    if (paletteSize > 0) {
      RGBQUAD *palette = new RGBQUAD[paletteSize];
      for (int i = 0; i < paletteSize; i++) {
        palette[i].rgbBlue = i;
        palette[i].rgbGreen = i;
        palette[i].rgbRed = i;
        palette[i].rgbReserved = 0;;
      }
      file.Write(palette, paletteSize * sizeof(RGBQUAD));
      delete[] palette;
    }

    file.Write(GetBits(), imageSize);

  } catch (CFileException *e) {
    e->Delete();
    return false;
  }

  return true;
}

int Bitmap::GetBytesPerLine() const {
  return (GetWidth() * GetBitsPerPixel() + 31) / 32 * 4;
}