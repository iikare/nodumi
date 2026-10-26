#include "voronoi.h"

#include <cmath>

#include "define.h"
#include "wrap.h"

using std::max;
using std::min;

namespace {

// sites are stored this many to a texture row
constexpr int site_tex_width = 1024;
// the grid aims for about this many sites per cell, and is at most this many cells across
constexpr float sites_per_cell = 3;
constexpr int max_grid = 256;

size_t pixel_hash(int64_t pixel, size_t mask) {
  uint64_t x = static_cast<uint64_t>(pixel) * 0x9E3779B97F4A7C15ull;
  x ^= x >> 29;
  return x & mask;
}

}  // namespace

void voronoiController::init() { updateTexture(); }

void voronoiController::clear_sites() {
  vertex.clear();
  color.clear();
  site_pixel.clear();
  std::fill(site_table.begin(), site_table.end(), -1);
  dots.clear();
}

void voronoiController::grow_site_table() {
  size_t size = max<size_t>(1024, site_table.size());
  while (size < 4 * (site_pixel.size() + 1)) {
    size *= 2;
  }
  site_table.assign(size, -1);
  for (size_t i = 0; i < site_pixel.size(); i++) {
    size_t slot = pixel_hash(site_pixel[i], size - 1);
    while (site_table[slot] != -1) {
      slot = (slot + 1) & (size - 1);
    }
    site_table[slot] = i;
  }
}

int voronoiController::add_site(float x, float y, const colorRGB& col) {
  if (site_table.size() < 2 * (site_pixel.size() + 1)) {
    grow_site_table();
  }
  const int64_t pixel = (static_cast<int64_t>(std::lround(x)) << 32) | static_cast<uint32_t>(std::lround(y));
  const Vector2 site = {x / ctr.getWidth(), 1 - y / ctr.getHeight()};

  const size_t mask = site_table.size() - 1;
  size_t slot = pixel_hash(pixel, mask);
  while (site_table[slot] != -1) {
    const int existing = site_table[slot];
    if (site_pixel[existing] == pixel) {
      vertex[existing] = site;
      color[existing] = col;
      return existing;
    }
    slot = (slot + 1) & mask;
  }
  site_table[slot] = vertex.size();
  site_pixel.push_back(pixel);
  vertex.push_back(site);
  color.push_back(col);
  return vertex.size() - 1;
}

void voronoiController::add_dot(int site, float x, float y, float radius, const colorRGB& col) {
  dots.push_back({{x, y}, radius, col, site, true});
}

void voronoiController::fit_texture(Texture2D& data_tex, int width, int height) {
  if (data_tex.id != 0 && data_tex.width == width && data_tex.height >= height) {
    return;
  }
  if (data_tex.id != 0) {
    rlUnloadTexture(data_tex.id);
  }

  // rows grow in powers of two, so a few more sites don't mean a new texture every frame
  int rows = 1;
  while (rows < height) {
    rows *= 2;
  }
  data_tex.id = rlLoadTexture(nullptr, width, rows, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, 1);
  data_tex.width = width;
  data_tex.height = rows;
  data_tex.mipmaps = 1;
  data_tex.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
}

void voronoiController::upload_sites() {
  const int count = vertex.size();

  // the grid covers the screen and every site, in square cells of a few sites each
  float x0 = 0;
  float x1 = 1;
  float y0 = 0;
  float y1 = 1;
  for (const auto& site : vertex) {
    x0 = min(x0, site.x);
    x1 = max(x1, site.x);
    y0 = min(y0, site.y);
    y1 = max(y1, site.y);
  }
  const float span_x = x1 - x0;
  const float span_y = y1 - y0;
  float cell = std::sqrt(span_x * span_y * sites_per_cell / max(count, 1));
  const int grid_w = std::clamp(static_cast<int>(std::ceil(span_x / cell)), 1, max_grid);
  const int grid_h = std::clamp(static_cast<int>(std::ceil(span_y / cell)), 1, max_grid);
  cell = max(span_x / grid_w, span_y / grid_h);

  // counting sort by cell, so each cell's sites sit next to each other
  cell_of.resize(count);
  cell_fill.assign(grid_w * grid_h, 0);
  for (int i = 0; i < count; i++) {
    const int cx = std::clamp(static_cast<int>((vertex[i].x - x0) / cell), 0, grid_w - 1);
    const int cy = std::clamp(static_cast<int>((vertex[i].y - y0) / cell), 0, grid_h - 1);
    cell_of[i] = cy * grid_w + cx;
    cell_fill[cell_of[i]]++;
  }

  cell_buffer.assign(grid_w * grid_h * 4, 0);
  int start = 0;
  for (int c = 0; c < grid_w * grid_h; c++) {
    cell_buffer[4 * c] = start;
    cell_buffer[4 * c + 1] = cell_fill[c];
    start += cell_fill[c];
    // from here on, the next free slot in the cell
    cell_fill[c] = cell_buffer[4 * c];
  }

  // each cell's search starts at the ring of the nearest occupied cell, found with a forward and a backward
  // chessboard distance pass
  const float far = grid_w + grid_h;
  for (int c = 0; c < grid_w * grid_h; c++) {
    cell_buffer[4 * c + 2] = cell_buffer[4 * c + 1] > 0 ? 0 : far;
  }
  const auto relax = [&](int x, int y, int nx, int ny) {
    if (nx >= 0 && nx < grid_w && ny >= 0 && ny < grid_h) {
      float& ring = cell_buffer[4 * (y * grid_w + x) + 2];
      ring = min(ring, cell_buffer[4 * (ny * grid_w + nx) + 2] + 1);
    }
  };
  for (int y = 0; y < grid_h; y++) {
    for (int x = 0; x < grid_w; x++) {
      relax(x, y, x - 1, y);
      relax(x, y, x - 1, y - 1);
      relax(x, y, x, y - 1);
      relax(x, y, x + 1, y - 1);
    }
  }
  for (int y = grid_h - 1; y >= 0; y--) {
    for (int x = grid_w - 1; x >= 0; x--) {
      relax(x, y, x + 1, y);
      relax(x, y, x + 1, y + 1);
      relax(x, y, x, y + 1);
      relax(x, y, x - 1, y + 1);
    }
  }

  const int rows = max(1, (count + site_tex_width - 1) / site_tex_width);
  site_buffer.assign(rows * site_tex_width * 4, 0);
  color_buffer.assign(rows * site_tex_width * 4, 0);
  for (int i = 0; i < count; i++) {
    const int slot = cell_fill[cell_of[i]]++;
    site_buffer[4 * slot] = vertex[i].x;
    site_buffer[4 * slot + 1] = vertex[i].y;
    color_buffer[4 * slot] = color[i].r / 255.0f;
    color_buffer[4 * slot + 1] = color[i].g / 255.0f;
    color_buffer[4 * slot + 2] = color[i].b / 255.0f;
  }

  fit_texture(site_tex, site_tex_width, rows);
  fit_texture(color_tex, site_tex_width, rows);
  fit_texture(cell_tex, max_grid, max_grid);
  constexpr int format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
  rlUpdateTexture(site_tex.id, 0, 0, site_tex_width, rows, format, site_buffer.data());
  rlUpdateTexture(color_tex.id, 0, 0, site_tex_width, rows, format, color_buffer.data());
  rlUpdateTexture(cell_tex.id, 0, 0, grid_w, grid_h, format, cell_buffer.data());

  ctr.setShaderValue("SH_VORONOI", "site_tex_width", site_tex_width);
  ctr.setShaderValue("SH_VORONOI", "grid_width", grid_w);
  ctr.setShaderValue("SH_VORONOI", "grid_height", grid_h);
  ctr.setShaderValue("SH_VORONOI", "grid_origin", Vector2{x0, y0});
  ctr.setShaderValue("SH_VORONOI", "cell_size", cell);
  ctr.setShaderValue("SH_VORONOI", "screen_size",
                     Vector2{static_cast<float>(ctr.getWidth()), static_cast<float>(ctr.getHeight())});
}

void voronoiController::update() {
  unload();
  updateTexture();
}

void voronoiController::updateTexture() {
  Image i = GenImageColor(ctr.getWidth(), ctr.getHeight(), WHITE);
  tex = LoadTextureFromImage(i);
  UnloadImage(i);
}

void voronoiController::unload() {
  UnloadTexture(tex);

  // the site textures come back on the next render
  for (Texture2D* data_tex : {&site_tex, &color_tex, &cell_tex}) {
    if (data_tex->id != 0) {
      rlUnloadTexture(data_tex->id);
    }
    *data_tex = {};
  }
}

void voronoiController::render(int voro_y) {
  upload_sites();
  ctr.setShaderValue("SH_VORONOI", "render_bound", static_cast<float>(voro_y) / ctr.getHeight());

  // the shader smooths its own edges, so it draws straight into the frame
  ctr.beginShaderMode("SH_VORONOI");
  // samplers have to be set again after every EndShaderMode
  ctr.setShaderTexture("SH_VORONOI", "site_data", site_tex);
  ctr.setShaderTexture("SH_VORONOI", "site_color", color_tex);
  ctr.setShaderTexture("SH_VORONOI", "cell_data", cell_tex);
  drawTextureEx(tex, {0, 0});
  ctr.endShaderMode();
}

void voronoiController::draw_dots() {
  // walking back from the last note, a dot is hidden once the later dots on its pixel cover all of it
  site_cover.assign(vertex.size(), {{0, 0}, -1});
  for (int i = static_cast<int>(dots.size()) - 1; i >= 0; i--) {
    dot_info& dot = dots[i];
    dot_cover& cover = site_cover[dot.site];
    if (cover.radius < 0) {
      cover = {dot.center, dot.radius};
      continue;
    }
    const float offset = std::hypot(dot.center.x - cover.center.x, dot.center.y - cover.center.y);
    dot.shown = dot.radius + offset > cover.radius;
    if (dot.shown) {
      cover.radius = max(cover.radius, dot.radius - offset);
    }
  }

  // one quad per dot, a pixel wider than it all round, which the shader cuts into a smooth circle
  ctr.beginShaderMode("SH_DISC");
  rlSetTexture(rlGetTextureIdDefault());
  rlBegin(RL_QUADS);
  for (const dot_info& dot : dots) {
    if (!dot.shown) {
      continue;
    }
    const float half = dot.radius + 1;
    const float reach = half / dot.radius;
    rlColor4ub(static_cast<unsigned char>(dot.color.r), static_cast<unsigned char>(dot.color.g),
               static_cast<unsigned char>(dot.color.b), 255);
    rlTexCoord2f(-reach, -reach);
    rlVertex2f(dot.center.x - half, dot.center.y - half);
    rlTexCoord2f(-reach, reach);
    rlVertex2f(dot.center.x - half, dot.center.y + half);
    rlTexCoord2f(reach, reach);
    rlVertex2f(dot.center.x + half, dot.center.y + half);
    rlTexCoord2f(reach, -reach);
    rlVertex2f(dot.center.x + half, dot.center.y - half);
  }
  rlEnd();
  rlSetTexture(0);
  ctr.endShaderMode();
}
