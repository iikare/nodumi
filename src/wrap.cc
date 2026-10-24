#include "wrap.h"

#include <algorithm>
#include <cmath>
#include <utility>


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
namespace {

void emit_triangle(Vector2 p, Vector2 q, Vector2 r, unsigned char ap, unsigned char aq, unsigned char ar,
                   const colorRGB& col) {
  // raylib culls back faces, so keep the winding it draws
  const float cross = (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
  if (cross > 0) {
    std::swap(q, r);
    std::swap(aq, ar);
  }
  rlColor4ub(col.r, col.g, col.b, ap);
  rlVertex2f(p.x, p.y);
  rlColor4ub(col.r, col.g, col.b, aq);
  rlVertex2f(q.x, q.y);
  rlColor4ub(col.r, col.g, col.b, ar);
  rlVertex2f(r.x, r.y);
}

// unit normal of an edge polyline at point i, facing away from the opposite edge
Vector2 find_edge_normal(const Vector2* edge, const Vector2* other, int i, int count) {
  const Vector2& prev = edge[i > 0 ? i - 1 : 0];
  const Vector2& next = edge[i + 1 < count ? i + 1 : count - 1];
  Vector2 normal = {prev.y - next.y, next.x - prev.x};
  const float len = std::sqrt(normal.x * normal.x + normal.y * normal.y);
  if (len == 0) {
    return {0, 0};
  }
  normal = {normal.x / len, normal.y / len};

  // where the two edges meet, fall back to the neighbouring rung to tell outwards
  int j = i;
  if (edge[j].x == other[j].x && edge[j].y == other[j].y) {
    j = i > 0 ? i - 1 : i + 1 < count ? i + 1 : i;
  }
  if (normal.x * (edge[j].x - other[j].x) + normal.y * (edge[j].y - other[j].y) < 0) {
    normal = {-normal.x, -normal.y};
  }
  return normal;
}

}  // namespace

void draw_aa_strip(const Vector2* a, const Vector2* b, int count, const colorRGB& col, unsigned char alpha) {
  // the sheet is drawn into a render texture without msaa, so long edges get a soft fringe instead
  constexpr int max_count = 32;
  constexpr float feather = 1.0f;
  if (count > max_count) {
    count = max_count;
  }
  if (count < 2) {
    return;
  }

  // pull both edges in by half the fringe so the shape keeps its weight
  Vector2 core_a[max_count];
  Vector2 core_b[max_count];
  for (int i = 0; i < count; ++i) {
    const float dx = a[i].x - b[i].x;
    const float dy = a[i].y - b[i].y;
    const float len = std::sqrt(dx * dx + dy * dy);
    const float shrink = len > 0 ? std::min(feather / 2, len * 0.4f) / len : 0;
    core_a[i] = {a[i].x - dx * shrink, a[i].y - dy * shrink};
    core_b[i] = {b[i].x + dx * shrink, b[i].y + dy * shrink};
  }

  Vector2 fringe_a[max_count];
  Vector2 fringe_b[max_count];
  for (int i = 0; i < count; ++i) {
    const Vector2 na = find_edge_normal(a, b, i, count);
    const Vector2 nb = find_edge_normal(b, a, i, count);
    fringe_a[i] = {core_a[i].x + na.x * feather, core_a[i].y + na.y * feather};
    fringe_b[i] = {core_b[i].x + nb.x * feather, core_b[i].y + nb.y * feather};
  }

  rlCheckRenderBatchLimit(18 * (count - 1));
  rlBegin(RL_TRIANGLES);
  for (int i = 0; i + 1 < count; ++i) {
    emit_triangle(core_a[i], core_b[i], core_a[i + 1], alpha, alpha, alpha, col);
    emit_triangle(core_b[i], core_b[i + 1], core_a[i + 1], alpha, alpha, alpha, col);
    emit_triangle(core_a[i], fringe_a[i], core_a[i + 1], alpha, 0, alpha, col);
    emit_triangle(fringe_a[i], fringe_a[i + 1], core_a[i + 1], 0, 0, alpha, col);
    emit_triangle(core_b[i], fringe_b[i], core_b[i + 1], alpha, 0, alpha, col);
    emit_triangle(fringe_b[i], fringe_b[i + 1], core_b[i + 1], 0, 0, alpha, col);
  }
  rlEnd();
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

void drawTexturePro(const Texture2D& tex, const rect& source, const rect& dest, const Vector2& origin, float rot) {
  Rectangle source_r = {static_cast<float>(source.x), static_cast<float>(source.y), static_cast<float>(source.width),
                        static_cast<float>(source.height)};
  Rectangle dest_r = {static_cast<float>(dest.x), static_cast<float>(dest.y), static_cast<float>(dest.width),
                      static_cast<float>(dest.height)};
  DrawTexturePro(tex, source_r, dest_r, origin, rot, WHITE);
}

const Vector2 measureTextEx(const string& msg, int size, const string& font) {
  const Font& ft = ctr.getFont(font, size);
  return MeasureTextEx(ft, msg.c_str(), ft.baseSize, TEXT_SPACING);

  return {0.0f, 0.0f};
}

ostream& operator<<(ostream& out, const Vector2& vec) {
  out << "{" << vec.x << ", " << vec.y << "} (vec2)";
  return out;
}

ostream& operator<<(ostream& out, const Vector3& vec) {
  out << "{" << vec.x << ", " << vec.y << ", " << vec.z << "} (vec3)";
  return out;
}
