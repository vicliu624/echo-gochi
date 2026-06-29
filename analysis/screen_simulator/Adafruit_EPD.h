#pragma once

#include "Arduino.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "glcdfont.c"

constexpr uint16_t EPD_BLACK = 0;
constexpr uint16_t EPD_WHITE = 1;

class Adafruit_EPD {
 public:
  enum class Update_Mode {
    FULL_REFRESH,
    PARTIAL_REFRESH,
    FAST_REFRESH,
  };
};

class Adafruit_SSD1681 {
 public:
  Adafruit_SSD1681(int16_t width, int16_t height, int8_t dc = -1,
                   int8_t rst = -1, int8_t cs = -1, int8_t sram = -1,
                   int8_t busy = -1, void* spi = nullptr)
      : nativeWidth_(width),
        nativeHeight_(height),
        width_(width),
        height_(height),
        pixels_(static_cast<size_t>(width) * static_cast<size_t>(height),
                EPD_WHITE) {
    (void)dc;
    (void)rst;
    (void)cs;
    (void)sram;
    (void)busy;
    (void)spi;
  }

  bool begin(bool reset = true) {
    (void)reset;
    return true;
  }

  void clearBuffer() { fillScreen(EPD_WHITE); }
  void display(bool sleep = false) { (void)sleep; }
  void display(Adafruit_EPD::Update_Mode mode, bool sleep = false) {
    (void)mode;
    (void)sleep;
  }
  void powerDown() {}
  void setBlackBuffer(int8_t index, bool inverted) {
    (void)index;
    (void)inverted;
  }
  void setColorBuffer(int8_t index, bool inverted) {
    (void)index;
    (void)inverted;
  }
  void setRAMValueBaseMap(uint8_t whiteValue, uint8_t blackValue) {
    (void)whiteValue;
    (void)blackValue;
  }

  int16_t width() const { return width_; }
  int16_t height() const { return height_; }

  void setRotation(uint8_t rotation) {
    rotation_ = rotation & 3;
    if (rotation_ & 1) {
      width_ = nativeHeight_;
      height_ = nativeWidth_;
    } else {
      width_ = nativeWidth_;
      height_ = nativeHeight_;
    }
    pixels_.assign(static_cast<size_t>(width_) * static_cast<size_t>(height_),
                   EPD_WHITE);
  }

  void fillScreen(uint16_t color) {
    std::fill(pixels_.begin(), pixels_.end(), normalizeColor(color));
  }

  void drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
    pixels_[static_cast<size_t>(y) * static_cast<size_t>(width_) +
            static_cast<size_t>(x)] = normalizeColor(color);
  }

  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    fillRect(x, y, w, 1, color);
  }

  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    fillRect(x, y, 1, h, color);
  }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (w < 0) {
      x += w;
      w = static_cast<int16_t>(-w);
    }
    if (h < 0) {
      y += h;
      h = static_cast<int16_t>(-h);
    }
    const int16_t x0 = std::max<int16_t>(0, x);
    const int16_t y0 = std::max<int16_t>(0, y);
    const int16_t x1 = std::min<int16_t>(width_, static_cast<int16_t>(x + w));
    const int16_t y1 = std::min<int16_t>(height_, static_cast<int16_t>(y + h));
    for (int16_t yy = y0; yy < y1; ++yy) {
      for (int16_t xx = x0; xx < x1; ++xx) drawPixel(xx, yy, color);
    }
  }

  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, static_cast<int16_t>(y + h - 1), w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(static_cast<int16_t>(x + w - 1), y, h, color);
  }

  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                uint16_t color) {
    const bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) {
      std::swap(x0, y0);
      std::swap(x1, y1);
    }
    if (x0 > x1) {
      std::swap(x0, x1);
      std::swap(y0, y1);
    }
    const int16_t dx = static_cast<int16_t>(x1 - x0);
    const int16_t dy = static_cast<int16_t>(std::abs(y1 - y0));
    int16_t err = dx / 2;
    const int16_t ystep = (y0 < y1) ? 1 : -1;
    for (; x0 <= x1; ++x0) {
      if (steep) {
        drawPixel(y0, x0, color);
      } else {
        drawPixel(x0, y0, color);
      }
      err = static_cast<int16_t>(err - dy);
      if (err < 0) {
        y0 = static_cast<int16_t>(y0 + ystep);
        err = static_cast<int16_t>(err + dx);
      }
    }
  }

  void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = static_cast<int16_t>(-2 * r);
    int16_t x = 0;
    int16_t y = r;

    drawPixel(x0, static_cast<int16_t>(y0 + r), color);
    drawPixel(x0, static_cast<int16_t>(y0 - r), color);
    drawPixel(static_cast<int16_t>(x0 + r), y0, color);
    drawPixel(static_cast<int16_t>(x0 - r), y0, color);

    while (x < y) {
      if (f >= 0) {
        --y;
        ddF_y = static_cast<int16_t>(ddF_y + 2);
        f = static_cast<int16_t>(f + ddF_y);
      }
      ++x;
      ddF_x = static_cast<int16_t>(ddF_x + 2);
      f = static_cast<int16_t>(f + ddF_x);
      drawPixel(static_cast<int16_t>(x0 + x), static_cast<int16_t>(y0 + y),
                color);
      drawPixel(static_cast<int16_t>(x0 - x), static_cast<int16_t>(y0 + y),
                color);
      drawPixel(static_cast<int16_t>(x0 + x), static_cast<int16_t>(y0 - y),
                color);
      drawPixel(static_cast<int16_t>(x0 - x), static_cast<int16_t>(y0 - y),
                color);
      drawPixel(static_cast<int16_t>(x0 + y), static_cast<int16_t>(y0 + x),
                color);
      drawPixel(static_cast<int16_t>(x0 - y), static_cast<int16_t>(y0 + x),
                color);
      drawPixel(static_cast<int16_t>(x0 + y), static_cast<int16_t>(y0 - x),
                color);
      drawPixel(static_cast<int16_t>(x0 - y), static_cast<int16_t>(y0 - x),
                color);
    }
  }

  void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    const int32_t rr = static_cast<int32_t>(r) * static_cast<int32_t>(r);
    for (int16_t y = static_cast<int16_t>(-r); y <= r; ++y) {
      for (int16_t x = static_cast<int16_t>(-r); x <= r; ++x) {
        if (static_cast<int32_t>(x) * x + static_cast<int32_t>(y) * y <= rr) {
          drawPixel(static_cast<int16_t>(x0 + x), static_cast<int16_t>(y0 + y),
                    color);
        }
      }
    }
  }

  void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                    int16_t x2, int16_t y2, uint16_t color) {
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
  }

  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                    int16_t x2, int16_t y2, uint16_t color) {
    const int16_t minX = std::max<int16_t>(0, std::min({x0, x1, x2}));
    const int16_t maxX =
        std::min<int16_t>(static_cast<int16_t>(width_ - 1),
                          std::max({x0, x1, x2}));
    const int16_t minY = std::max<int16_t>(0, std::min({y0, y1, y2}));
    const int16_t maxY =
        std::min<int16_t>(static_cast<int16_t>(height_ - 1),
                          std::max({y0, y1, y2}));
    for (int16_t y = minY; y <= maxY; ++y) {
      for (int16_t x = minX; x <= maxX; ++x) {
        if (pointInTriangle(x, y, x0, y0, x1, y1, x2, y2)) {
          drawPixel(x, y, color);
        }
      }
    }
  }

  void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,
                     uint16_t color) {
    r = clampRadius(w, h, r);
    for (int16_t yy = y; yy < y + h; ++yy) {
      for (int16_t xx = x; xx < x + w; ++xx) {
        const bool outer = insideRoundRect(xx, yy, x, y, w, h, r);
        const bool inner =
            insideRoundRect(xx, yy, static_cast<int16_t>(x + 1),
                            static_cast<int16_t>(y + 1),
                            static_cast<int16_t>(w - 2),
                            static_cast<int16_t>(h - 2),
                            static_cast<int16_t>(std::max<int16_t>(0, r - 1)));
        if (outer && !inner) drawPixel(xx, yy, color);
      }
    }
  }

  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,
                     uint16_t color) {
    r = clampRadius(w, h, r);
    for (int16_t yy = y; yy < y + h; ++yy) {
      for (int16_t xx = x; xx < x + w; ++xx) {
        if (insideRoundRect(xx, yy, x, y, w, h, r)) drawPixel(xx, yy, color);
      }
    }
  }

  void setCursor(int16_t x, int16_t y) {
    cursorX_ = x;
    cursorY_ = y;
  }

  void setTextColor(uint16_t color) { textColor_ = color; }
  void setTextSize(uint8_t size) { textSize_ = std::max<uint8_t>(1, size); }

  void print(const char* text) {
    if (!text) return;
    while (*text) print(*text++);
  }

  void print(char value) {
    if (value == '\n') {
      cursorX_ = 0;
      cursorY_ = static_cast<int16_t>(cursorY_ + textSize_ * 8);
      return;
    }
    if (value == '\r') return;
    drawChar(cursorX_, cursorY_, value, textColor_, textSize_);
    cursorX_ = static_cast<int16_t>(cursorX_ + textSize_ * 6);
  }

  void print(unsigned char value) { printNumber(static_cast<unsigned>(value)); }
  void print(int value) { printNumber(value); }
  void print(unsigned int value) { printNumber(value); }
  void print(long value) { printNumber(value); }
  void print(unsigned long value) { printNumber(value); }

  bool writePng(const char* path, int scale = 4) const {
    if (!path || scale < 1 || width_ <= 0 || height_ <= 0) return false;
    const uint32_t outWidth = static_cast<uint32_t>(width_ * scale);
    const uint32_t outHeight = static_cast<uint32_t>(height_ * scale);
    std::vector<uint8_t> raw;
    raw.reserve(static_cast<size_t>(outHeight) *
                (static_cast<size_t>(outWidth) * 3 + 1));

    for (uint32_t y = 0; y < outHeight; ++y) {
      raw.push_back(0);
      const int16_t srcY = static_cast<int16_t>(y / scale);
      for (uint32_t x = 0; x < outWidth; ++x) {
        const int16_t srcX = static_cast<int16_t>(x / scale);
        const bool white = pixelAt(srcX, srcY) != EPD_BLACK;
        const uint8_t value = white ? 255 : 0;
        raw.push_back(value);
        raw.push_back(value);
        raw.push_back(value);
      }
    }

    std::vector<uint8_t> png = pngSignature();
    std::vector<uint8_t> ihdr;
    appendU32BE(ihdr, outWidth);
    appendU32BE(ihdr, outHeight);
    ihdr.push_back(8);
    ihdr.push_back(2);
    ihdr.push_back(0);
    ihdr.push_back(0);
    ihdr.push_back(0);
    appendChunk(png, "IHDR", ihdr);

    std::vector<uint8_t> zlib;
    zlib.push_back(0x78);
    zlib.push_back(0x01);
    size_t offset = 0;
    while (offset < raw.size()) {
      const size_t remaining = raw.size() - offset;
      const uint16_t len =
          static_cast<uint16_t>(std::min<size_t>(remaining, 65535));
      const bool finalBlock = (offset + len) == raw.size();
      zlib.push_back(finalBlock ? 1 : 0);
      zlib.push_back(static_cast<uint8_t>(len & 0xFF));
      zlib.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
      const uint16_t nlen = static_cast<uint16_t>(~len);
      zlib.push_back(static_cast<uint8_t>(nlen & 0xFF));
      zlib.push_back(static_cast<uint8_t>((nlen >> 8) & 0xFF));
      zlib.insert(zlib.end(), raw.begin() + static_cast<std::ptrdiff_t>(offset),
                  raw.begin() + static_cast<std::ptrdiff_t>(offset + len));
      offset += len;
    }
    appendU32BE(zlib, adler32(raw));
    appendChunk(png, "IDAT", zlib);
    appendChunk(png, "IEND", {});

    FILE* file = std::fopen(path, "wb");
    if (!file) return false;
    const size_t written = std::fwrite(png.data(), 1, png.size(), file);
    std::fclose(file);
    return written == png.size();
  }

 private:
  static uint8_t normalizeColor(uint16_t color) {
    return color == EPD_BLACK ? EPD_BLACK : EPD_WHITE;
  }

  uint8_t pixelAt(int16_t x, int16_t y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return EPD_WHITE;
    return pixels_[static_cast<size_t>(y) * static_cast<size_t>(width_) +
                   static_cast<size_t>(x)];
  }

  void drawChar(int16_t x, int16_t y, char value, uint16_t color,
                uint8_t size) {
    unsigned char c = static_cast<unsigned char>(value);
    if (c < 32 || c > 126) c = '?';
    for (uint8_t i = 0; i < 5; ++i) {
      uint8_t line = pgm_read_byte(font + (static_cast<uint16_t>(c) * 5) + i);
      for (uint8_t j = 0; j < 8; ++j) {
        if (line & 0x1) {
          if (size == 1) {
            drawPixel(static_cast<int16_t>(x + i), static_cast<int16_t>(y + j),
                      color);
          } else {
            fillRect(static_cast<int16_t>(x + i * size),
                     static_cast<int16_t>(y + j * size), size, size, color);
          }
        }
        line >>= 1;
      }
    }
  }

  template <typename T>
  void printNumber(T value) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%ld", static_cast<long>(value));
    print(buffer);
  }

  static int32_t edge(int16_t ax, int16_t ay, int16_t bx, int16_t by,
                      int16_t px, int16_t py) {
    return static_cast<int32_t>(px - ax) * static_cast<int32_t>(by - ay) -
           static_cast<int32_t>(py - ay) * static_cast<int32_t>(bx - ax);
  }

  static bool pointInTriangle(int16_t px, int16_t py, int16_t x0, int16_t y0,
                              int16_t x1, int16_t y1, int16_t x2,
                              int16_t y2) {
    const int32_t d0 = edge(x0, y0, x1, y1, px, py);
    const int32_t d1 = edge(x1, y1, x2, y2, px, py);
    const int32_t d2 = edge(x2, y2, x0, y0, px, py);
    const bool hasNeg = (d0 < 0) || (d1 < 0) || (d2 < 0);
    const bool hasPos = (d0 > 0) || (d1 > 0) || (d2 > 0);
    return !(hasNeg && hasPos);
  }

  static int16_t clampRadius(int16_t w, int16_t h, int16_t r) {
    if (r < 0) return 0;
    return std::min<int16_t>(r, static_cast<int16_t>(std::min(w, h) / 2));
  }

  static bool insideRoundRect(int16_t px, int16_t py, int16_t x, int16_t y,
                              int16_t w, int16_t h, int16_t r) {
    if (w <= 0 || h <= 0) return false;
    if (px < x || py < y || px >= x + w || py >= y + h) return false;
    if (r <= 0) return true;
    const int16_t left = static_cast<int16_t>(x + r);
    const int16_t right = static_cast<int16_t>(x + w - r - 1);
    const int16_t top = static_cast<int16_t>(y + r);
    const int16_t bottom = static_cast<int16_t>(y + h - r - 1);
    const int16_t cx = px < left ? left : (px > right ? right : px);
    const int16_t cy = py < top ? top : (py > bottom ? bottom : py);
    const int32_t dx = static_cast<int32_t>(px - cx);
    const int32_t dy = static_cast<int32_t>(py - cy);
    return dx * dx + dy * dy <= static_cast<int32_t>(r) * r;
  }

  static std::vector<uint8_t> pngSignature() {
    return {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
  }

  static void appendU32BE(std::vector<uint8_t>& out, uint32_t value) {
    out.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(value & 0xFF));
  }

  static uint32_t crc32(const uint8_t* data, size_t size) {
    uint32_t crc = 0xFFFFFFFFU;
    for (size_t i = 0; i < size; ++i) {
      crc ^= data[i];
      for (int bit = 0; bit < 8; ++bit) {
        const uint32_t mask = static_cast<uint32_t>(-(crc & 1U));
        crc = (crc >> 1) ^ (0xEDB88320U & mask);
      }
    }
    return ~crc;
  }

  static uint32_t adler32(const std::vector<uint8_t>& data) {
    uint32_t a = 1;
    uint32_t b = 0;
    for (uint8_t value : data) {
      a = (a + value) % 65521U;
      b = (b + a) % 65521U;
    }
    return (b << 16) | a;
  }

  static void appendChunk(std::vector<uint8_t>& png, const char* type,
                          const std::vector<uint8_t>& data) {
    appendU32BE(png, static_cast<uint32_t>(data.size()));
    const size_t crcStart = png.size();
    png.push_back(static_cast<uint8_t>(type[0]));
    png.push_back(static_cast<uint8_t>(type[1]));
    png.push_back(static_cast<uint8_t>(type[2]));
    png.push_back(static_cast<uint8_t>(type[3]));
    png.insert(png.end(), data.begin(), data.end());
    const uint32_t crc = crc32(png.data() + crcStart, png.size() - crcStart);
    appendU32BE(png, crc);
  }

  int16_t nativeWidth_;
  int16_t nativeHeight_;
  int16_t width_;
  int16_t height_;
  uint8_t rotation_ = 0;
  int16_t cursorX_ = 0;
  int16_t cursorY_ = 0;
  uint8_t textSize_ = 1;
  uint16_t textColor_ = EPD_BLACK;
  std::vector<uint8_t> pixels_;
};
