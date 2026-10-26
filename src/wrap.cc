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

namespace {

// cubic ease in and out, as raylib's beziers use
float ease_cubic_in_out(float t, float b, float c, float d) {
  t /= 0.5f * d;
  if (t < 1) {
    return 0.5f * c * t * t * t + b;
  }
  t -= 2;
  return 0.5f * c * (t * t * t + 2.0f) + b;
}

// draws raylib's triangle strip points as quads, the primitive its rings and rectangles use, so lines
// mixed with them stay in one draw call instead of starting a new one at every switch
void emit_quad_strip(const Vector2* points, int count, Color color) {
  const Texture2D tex = GetShapesTexture();
  const Rectangle rect = GetShapesTextureRectangle();
  const float u0 = rect.x / tex.width;
  const float v0 = rect.y / tex.height;
  const float u1 = (rect.x + rect.width) / tex.width;
  const float v1 = (rect.y + rect.height) / tex.height;

  rlSetTexture(tex.id);
  rlBegin(RL_QUADS);
  rlNormal3f(0.0f, 0.0f, 1.0f);
  rlColor4ub(color.r, color.g, color.b, color.a);
  for (int i = 2; i + 1 < count; i += 2) {
    // walking the edge of the strip's two triangles keeps their winding
    rlTexCoord2f(u0, v0);
    rlVertex2f(points[i - 2].x, points[i - 2].y);
    rlTexCoord2f(u0, v1);
    rlVertex2f(points[i - 1].x, points[i - 1].y);
    rlTexCoord2f(u1, v1);
    rlVertex2f(points[i + 1].x, points[i + 1].y);
    rlTexCoord2f(u1, v0);
    rlVertex2f(points[i].x, points[i].y);
  }
  rlEnd();
  rlSetTexture(0);
}

// emit_quad_strip with the edges softened the way draw_aa_strip does the sheet: each rung is pulled in by
// half the fringe and a one pixel fringe fades out past it, since the render texture has no msaa
void emit_smooth_quad_strip(const Vector2* points, int count, Color color) {
  constexpr int max_rungs = 25;
  constexpr float feather = 1.0f;
  const int rungs = std::min(count / 2, max_rungs);
  if (rungs < 2) {
    return;
  }

  Vector2 core_a[max_rungs];
  Vector2 core_b[max_rungs];
  Vector2 fringe_a[max_rungs];
  Vector2 fringe_b[max_rungs];
  for (int i = 0; i < rungs; i++) {
    const Vector2& a = points[2 * i];
    const Vector2& b = points[2 * i + 1];
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float len = std::sqrt(dx * dx + dy * dy);
    const float shrink = len > 0 ? std::min(feather / 2, len * 0.4f) / len : 0;
    const float out = len > 0 ? feather / len : 0;
    core_a[i] = {a.x - dx * shrink, a.y - dy * shrink};
    core_b[i] = {b.x + dx * shrink, b.y + dy * shrink};
    fringe_a[i] = {core_a[i].x + dx * out, core_a[i].y + dy * out};
    fringe_b[i] = {core_b[i].x - dx * out, core_b[i].y - dy * out};
  }

  const Texture2D tex = GetShapesTexture();
  const Rectangle rect = GetShapesTextureRectangle();
  const float u0 = rect.x / tex.width;
  const float v0 = rect.y / tex.height;
  const float u1 = (rect.x + rect.width) / tex.width;
  const float v1 = (rect.y + rect.height) / tex.height;
  const auto vertex = [&](const Vector2& v, unsigned char alpha, float u, float t) {
    rlColor4ub(color.r, color.g, color.b, alpha);
    rlTexCoord2f(u, t);
    rlVertex2f(v.x, v.y);
  };

  rlSetTexture(tex.id);
  rlBegin(RL_QUADS);
  rlNormal3f(0.0f, 0.0f, 1.0f);
  for (int i = 1; i < rungs; i++) {
    // fringe, core, fringe, each walked from the a side to the b side like emit_quad_strip's quads
    vertex(fringe_a[i - 1], 0, u0, v0);
    vertex(core_a[i - 1], color.a, u0, v1);
    vertex(core_a[i], color.a, u1, v1);
    vertex(fringe_a[i], 0, u1, v0);

    vertex(core_a[i - 1], color.a, u0, v0);
    vertex(core_b[i - 1], color.a, u0, v1);
    vertex(core_b[i], color.a, u1, v1);
    vertex(core_a[i], color.a, u1, v0);

    vertex(core_b[i - 1], color.a, u0, v0);
    vertex(fringe_b[i - 1], 0, u0, v1);
    vertex(fringe_b[i], 0, u1, v1);
    vertex(core_b[i], color.a, u1, v0);
  }
  rlEnd();
  rlSetTexture(0);
}

}  // namespace

void drawLineEx(float xi, float yi, float xf, float yf, float thick, const colorRGB& col, unsigned char alpha) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, alpha};

  // the corners raylib's DrawLineEx uses
  const float dx = xf - xi;
  const float dy = yf - yi;
  const float length = std::sqrt(dx * dx + dy * dy);
  if (!(length > 0 && thick > 0)) {
    return;
  }
  const float scale = thick / (2 * length);
  const Vector2 radius = {-scale * dy, scale * dx};
  const Vector2 strip[4] = {{xi - radius.x, yi - radius.y},
                            {xi + radius.x, yi + radius.y},
                            {xf - radius.x, yf - radius.y},
                            {xf + radius.x, yf + radius.y}};
  emit_quad_strip(strip, 4, color);
}

void draw_line_smooth(float xi, float yi, float xf, float yf, float thick, const colorRGB& col,
                      unsigned char alpha) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, alpha};

  // the same corners as drawLineEx, with soft edges
  const float dx = xf - xi;
  const float dy = yf - yi;
  const float length = std::sqrt(dx * dx + dy * dy);
  if (!(length > 0 && thick > 0)) {
    return;
  }
  const float scale = thick / (2 * length);
  const Vector2 radius = {-scale * dy, scale * dx};
  const Vector2 strip[4] = {{xi - radius.x, yi - radius.y},
                            {xi + radius.x, yi + radius.y},
                            {xf - radius.x, yf - radius.y},
                            {xf + radius.x, yf + radius.y}};
  emit_smooth_quad_strip(strip, 4, color);
}

void drawLineBezier(float xi, float yi, float xf, float yf, float thick, const colorRGB& col) {
  Color color = (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, 255};

  // raylib always cuts the curve into 24 segments, but segments a few pixels long look the same
  constexpr int max_segments = 24;
  constexpr float segment_px = 4;
  const float span_x = xf - xi;
  const float span_y = yf - yi;
  // the eased curve is steepest in the middle, at 1.5 times its average slope
  const float wanted = std::ceil((std::fabs(span_x) + 1.5f * std::fabs(span_y)) / segment_px);
  const int segments = !(wanted < max_segments) ? max_segments : std::max(1, static_cast<int>(wanted));

  // the points raylib's DrawLineBezier makes, for that many segments
  Vector2 points[2 * max_segments + 2];
  Vector2 previous = {xi, yi};
  for (int i = 1; i <= segments; i++) {
    Vector2 current;
    current.y = ease_cubic_in_out(static_cast<float>(i), yi, span_y, static_cast<float>(segments));
    current.x = previous.x + span_x / static_cast<float>(segments);

    const float dy = current.y - previous.y;
    const float dx = current.x - previous.x;
    const float size = 0.5f * thick / std::sqrt(dx * dx + dy * dy);
    if (i == 1) {
      points[0] = {previous.x + dy * size, previous.y - dx * size};
      points[1] = {previous.x - dy * size, previous.y + dx * size};
    }
    points[2 * i + 1] = {current.x - dy * size, current.y + dx * size};
    points[2 * i] = {current.x + dy * size, current.y - dx * size};

    previous = current;
  }
  emit_smooth_quad_strip(points, 2 * segments + 2, color);
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

void drawSymbolRotated(int sym, int size, float x, float y, float angle, const colorRGB& col, unsigned char alpha,
                       const string& font) {
  // turned about the top left of the glyph's line, which drawSymbol places at x and y
  Color color =
      (Color){(unsigned char)col.r, (unsigned char)col.g, (unsigned char)col.b, static_cast<unsigned char>(alpha)};
  rlPushMatrix();
  rlTranslatef(x, y, 0);
  rlRotatef(angle, 0, 0, 1);
  DrawTextCodepoint(ctr.getFont(font, size), sym, (const Vector2){0, 0}, size, color);
  rlPopMatrix();
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
