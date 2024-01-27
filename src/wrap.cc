#include "wrap.h"

void drawPixel(float x, float y, const colorRGB& col, unsigned char alpha) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, (unsigned char)alpha};
  DrawPixel(x, y, color);
}

void drawLine(float xi, float yi, float xf, float yf, const colorRGB& col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  DrawLine(xi, yi, xf, yf, color);
}
void drawLineEx(float xi, float yi, float xf, float yf, float thick, const colorRGB& col, unsigned char alpha) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, alpha};
  DrawLineEx((const Vector2){(float)xi, (float)yi}, (const Vector2){(float)xf, (float)yf}, thick, color);
}
void drawLineBezier(float xi, float yi, float xf, float yf, float thick, const colorRGB& col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  DrawLineBezier((const Vector2){(float)xi, (float)yi}, (const Vector2){(float)xf, (float)yf}, thick, color);
}
void clearBackground() { ClearBackground({0, 0, 0, 0}); }
void clearBackground(const colorRGB& col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  ClearBackground(color);
}
void drawRectangle(float x, float y, float w, float h, const colorRGB& col, unsigned char alpha) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, alpha};
  DrawRectangleV({x, y}, {w, h}, color);
}
void drawRectangleLines(float x, float y, float w, float h, float width, const colorRGB& col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  DrawRectangleLinesEx({x, y, w, h}, width, color);
}

void drawTextEx(const string& msg, const Vector2& pos, const colorRGB& col, unsigned char alpha, int size,
                const string& font) {
  drawTextEx(msg, pos.x, pos.y, col, alpha, size, font);
}
void drawTextEx(const string& msg, int x, int y, const colorRGB& col, unsigned char alpha, int size,
                const string& font) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, alpha};
  const Font& ft = ctr.getFont(font, size);
  DrawTextEx(ft, msg.c_str(), (const Vector2){static_cast<float>(x), static_cast<float>(y)}, ft.baseSize, TEXT_SPACING,
             color);
}

void drawSymbol(int sym, int size, int x, int y, const colorRGB& col, unsigned char alpha, const string& font) {
  Color color =
      (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, static_cast<unsigned char>(alpha)};
  DrawTextCodepoint(ctr.getFont(font, size), sym, (const Vector2){static_cast<float>(x), static_cast<float>(y)}, size,
                    color);
}

void drawCircle(float x, float y, float r, const colorRGB& col, float alpha) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, (unsigned char)alpha};

  DrawCircle(x, y, r, color);
}

void drawRing(const Vector2& center, float iRad, float oRad, const colorRGB& col, float alpha, float sAngle,
              float eAngle) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, (unsigned char)alpha};

  DrawRing(center, iRad, oRad, sAngle, eAngle, 1 + oRad, color);
}

void drawGradientLineH(const Vector2& a, const Vector2& b, float thick, const colorRGB& col, float alphaA,
                       float alphaB) {
  Color colorA = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, (unsigned char)(alphaA)};
  Color colorB = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, (unsigned char)alphaB};
  DrawRectangleGradientH(a.x, a.y - thick / 2.0, b.x - a.x, thick, colorB, colorA);
}

void drawTextureEx(const Texture2D& tex, const Vector2& pos, float rot, float scale) {
  DrawTextureEx(tex, pos, rot, scale, WHITE);
}

const Vector2 measureTextEx(const string& msg, int size, const string& font) {
  const Font& ft = ctr.getFont(font, size);
  return MeasureTextEx(ft, msg.c_str(), ft.baseSize, TEXT_SPACING);

  return {0.0f, 0.0f};
}
