#pragma once

#include <string>

#include "build_target.h"
#include "cie2k.h"
#include "color.h"
#include "data.h"
#include "define.h"

using std::string;

void clearBackground();
void clearBackground(const colorRGB& col);

void drawPixel(float x, float y, const colorRGB& col,
               unsigned char alpha = 255);

void drawRectangle(float x, float y, float w, float h, const colorRGB& col,
                   unsigned char alpha = 255);
void drawRectangleLines(float x, float y, float w, float h, float width,
                        const colorRGB& col);
void drawLine(float xi, float yi, float xf, float yf, const colorRGB& col);
void drawLineEx(float xi, float yi, float xf, float yf, float thick,
                const colorRGB& col, unsigned char alpha = 255);
void drawLineBezier(float xi, float yi, float xf, float yf, float thick,
                    const colorRGB& col);

void drawTextEx(const string& msg, const Vector2& pos, const colorRGB& col,
                unsigned char alpha = 255, int size = 14,
                const string& font = DEFAULT_FONT);
void drawTextEx(const string& msg, int x, int y, const colorRGB& col,
                unsigned char alpha = 255, int size = 14,
                const string& font = DEFAULT_FONT);

void drawSymbol(int sym, int size, int x, int y, const colorRGB& col,
                unsigned char alpha = 255, const string& font = GLYPH_FONT);

void drawCircle(float x, float y, float r, const colorRGB& col,
                float alpha = 255);

void drawRing(const Vector2& center, float iRad, float oRad,
              const colorRGB& col, float alpha = 255, float startAngle = 0,
              float endAngle = 360);

void drawGradientLineH(const Vector2& a, const Vector2& b, float thick,
                       const colorRGB& col, float alphaA, float alphaB);

void drawTextureEx(const Texture2D& tex, const Vector2& pos, float rot = 0.0f,
                   float scale = 1.0f);

const Vector2 measureTextEx(const string& msg, int size = 14,
                            const string& font = DEFAULT_FONT);

template <class T>
bool isKeyPressed(T key) {
  return IsKeyPressed(key);
}
template <class T, class... U>
bool isKeyPressed(T key, U... keys) {
  return IsKeyPressed(key) || isKeyPressed(keys...);
}

template <class T>
bool isKeyDown(T key) {
  return IsKeyDown(key);
}
template <class T, class... U>
bool isKeyDown(T key, U... keys) {
  return IsKeyDown(key) || isKeyDown(keys...);
}

template <class T>
auto deltaE(const T& a, const T& b) {
  auto t = static_cast<cie2k::TYPE>(ctr.option.get(OPTION::CIE_FUNCTION));
  switch (t) {
    using enum cie2k::TYPE;
    case CIE_00:
      return cie2k::deltaE<T, CIE_00>(a, b);
    case CIE_94:
      return cie2k::deltaE<T, CIE_94>(a, b);
    case CIE_76:
      return cie2k::deltaE<T, CIE_76>(a, b);
    default:
      return 0.0;
  }
}
