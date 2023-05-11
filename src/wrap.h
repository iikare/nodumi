#pragma once

#include "build_target.h"

#include <string>
#include "cie2k.h"
#include "define.h"
#include "color.h"
#include "data.h"

using std::string;

void clearBackground(const colorRGB& col);
void drawRectangle(float x, float y, float w, float h, const colorRGB& col, unsigned char alpha = 255);
void drawRectangleLines(float x, float y, float w, float h, float width, const colorRGB& col);
void drawLine(float xi, float yi, float xf, float yf, const colorRGB& col);
void drawLineEx(float xi, float yi, float xf, float yf, float thick, const colorRGB& col);
void drawLineBezier(float xi, float yi, float xf, float yf, float thick, const colorRGB& col);

void drawTextEx(const string& msg, const Vector2& pos, const colorRGB& col, 
                unsigned char alpha = 255, int size = 14, const string& font = DEFAULT_FONT);
void drawTextEx(const string& msg, int x, int y, const colorRGB& col, 
                unsigned char alpha = 255, int size = 14, const string& font = DEFAULT_FONT);

void drawSymbol(int sym, int size, int x, int y, const colorRGB& col, unsigned char alpha = 255, const string& font = GLYPH_FONT);

void drawRing(const Vector2& center, float iRad, float oRad, const colorRGB& col, 
              float alpha = 255, float startAngle = 0, float endAngle = 360); 

void drawGradientLineH(const Vector2& a, const Vector2& b, float thick, const colorRGB& col, float alphaA, float alphaB);

void drawTextureEx(const Texture2D& tex, const Vector2& pos, float rot = 0.0f, float scale = 1.0f);

const Vector2 measureTextEx(const string& msg, int size = 14, const string& font = DEFAULT_FONT);

template<class T>
auto deltaE(T a, T b) {
  auto t = static_cast<cie2k::TYPE>(ctr.option.get(optionType::OPTION_CIE_FUNCTION));
  switch (t) {
    case cie2k::TYPE::CIE_00:
      return cie2k::deltaE<T, cie2k::TYPE::CIE_00>(a, b);
    case cie2k::TYPE::CIE_94:
      return cie2k::deltaE<T, cie2k::TYPE::CIE_94>(a, b);
    case cie2k::TYPE::CIE_76:
      return cie2k::deltaE<T, cie2k::TYPE::CIE_76>(a, b);
    default:
      return 0.0;
  }
}
