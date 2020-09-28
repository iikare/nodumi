#include "wrap.h"

void drawLine(int xi, int yi, int xf, int yf, colorRGB col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  DrawLine(xi, yi, xf, yf, color);
}
void clearBackground(colorRGB col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  ClearBackground(color);
}
void drawRectangle(int x, int y, int w, int h, colorRGB col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  DrawRectangle(x, y, w, h, color);
}
void drawLineEx(int xi, int yi, int xf, int yf, float thick, colorRGB col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  DrawLineEx((Vector2){(float)xi, (float)yi}, (Vector2){(float)xf, (float)yf}, thick, color);
}

void drawTextEx(Font ft, string msg, int x, int y, colorRGB col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  DrawTextEx(ft, msg.c_str(), (Vector2){static_cast<float>(x), static_cast<float>(y)}, ft.baseSize, 0.5, color);
}

void drawCircle(int x, int y, float r, colorRGB col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  DrawCircle(x, y, r, color);
}

void drawCircleLines(int x, int y, float r, colorRGB col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};
  DrawCircleLines(x, y, r, color);
}
