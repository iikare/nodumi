#include "wrap.h"

void drawLine(int xi, int yi, int xf, int yf, colorRGB col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  DrawLine(xi, yi, xf, yf, color);
}
void clearBackground(colorRGB col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  ClearBackground(color);
}
void drawRectangle(float x, float y, float w, float h, colorRGB col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  DrawRectangleV({x, y}, {w, h}, color);
}
void drawLineEx(int xi, int yi, int xf, int yf, float thick, colorRGB col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  DrawLineEx((Vector2){(float)xi, (float)yi}, (Vector2){(float)xf, (float)yf}, thick, color);
}


void drawTextEx(Font ft, string msg, Vector2 pos, colorRGB col, unsigned char alpha) {
  drawTextEx(ft, msg, pos.x, pos.y, col, alpha);
}
void drawTextEx(Font ft, string msg, int x, int y, colorRGB col, unsigned char alpha) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, alpha};
  DrawTextEx(ft, msg.c_str(), (Vector2){static_cast<float>(x), static_cast<float>(y)}, 
             ft.baseSize, TEXT_SPACING, color);
}

void drawCircle(int x, int y, float r, colorRGB col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  DrawCircle(x, y, r, color);
}

void drawCircleLines(int x, int y, float r, colorRGB col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  DrawCircleLines(x, y, r, color);
}

void drawRing(Vector2 center, float iRad, float oRad, colorRGB col, float alpha) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, 
                        (unsigned char)col.b, (unsigned char)alpha};
  DrawRing(center, iRad, oRad, 0.0f, 360.0f, 1 + oRad, color);
}
void drawRing(Vector2 center, float iRad, float oRad, colorRGB col) {
  drawRing(center, iRad, oRad, col, 255);

}

void drawGradientLineH(Vector2 a, Vector2 b, float thick, colorRGB col, float alphaA, float alphaB) {
  Color colorA = (Color){(unsigned char)col.r, (unsigned char)col.g, 
                         (unsigned char)col.b, (unsigned char)(alphaA)};
  Color colorB = (Color){(unsigned char)col.r, (unsigned char)col.g, 
                         (unsigned char)col.b, (unsigned char)alphaB};
  DrawRectangleGradientH(a.x, a.y-thick/2.0 , b.x-a.x, thick , colorB, colorA);
}

Vector2 measureTextEx(Font ft, string msg) {
  return MeasureTextEx(ft, msg.c_str(), ft.baseSize, TEXT_SPACING);
}
