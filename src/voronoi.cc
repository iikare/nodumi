#include "voronoi.h"

#include "define.h"
#include "wrap.h"

void voronoiController::init() {
  updateBuffer();
  updateTexture();
}

void voronoiController::resample(int voro_y) {
  vector<Vector2>& voronoi_vertex_data = vertex;
  vector<colorRGB>& voronoi_color_data = color;

  vector<Vector2> voronoi_vertex_resampled;
  vector<colorRGB> voronoi_color_resampled;

  if (vertex.size() > VORONOI_MAX_POINTS) {
    voronoi_vertex_resampled.resize(VORONOI_MAX_POINTS);
    voronoi_color_resampled.resize(VORONOI_MAX_POINTS);

    // sample points
    double sampleRatio = vertex.size() / static_cast<double>(VORONOI_MAX_POINTS);
    for (auto i = 0; i < VORONOI_MAX_POINTS; ++i) {
      voronoi_vertex_resampled[i] = vertex[int(i * sampleRatio)];
      voronoi_color_resampled[i] = color[int(i * sampleRatio)];
    }

    voronoi_vertex_data = voronoi_vertex_resampled;
    voronoi_color_data = voronoi_color_resampled;
  }

  // logQ(voronoi_vertex_data[0], voronoi_vertex_data[1], voronoi_vertex_data[2], voronoi_vertex_data[3]);

  int voroSize = min(static_cast<int>(vertex.size()), VORONOI_MAX_POINTS);
  float render_bound = static_cast<float>(voro_y) / ctr.getHeight();

  ctr.setShaderValue("SH_VORONOI", "vertex_count", voroSize);
  ctr.setShaderValue("SH_VORONOI", "vertex_data", voronoi_vertex_data, voroSize);
  ctr.setShaderValue("SH_VORONOI", "vertex_color", voronoi_color_data, voroSize);
  ctr.setShaderValue("SH_VORONOI", "render_bound", render_bound);

  vertex.clear();
  color.clear();
}

void voronoiController::update() {
  unload();
  updateBuffer();
  updateTexture();
}

void voronoiController::updateBuffer() { voro_buffer = LoadRenderTexture(ctr.getWidth(), ctr.getHeight()); }
void voronoiController::updateTexture() {
  Image i = GenImageColor(ctr.getWidth(), ctr.getHeight(), WHITE);
  tex = LoadTextureFromImage(i);
  UnloadImage(i);
}

void voronoiController::unload() {
  UnloadRenderTexture(voro_buffer);
  UnloadTexture(tex);
}

void voronoiController::render() {
  ctr.pushTextureMode("FB_VORONOI", &voro_buffer);

  ctr.beginShaderMode("SH_VORONOI");
  drawTextureEx(tex, {0, 0});  // static_cast<float>(ctr.menuHeight)});
  // drawTextureEx(tex, {0, static_cast<float>(ctr.menuHeight)});
  ctr.endShaderMode();

  ctr.popTextureMode();

  ctr.beginShaderMode("SH_FXAA");
  // DrawTextureEx(voro_buffer.texture, { 0, ctr.menuHeight }, 360.0f, 1.0f,
  // WHITE);
  DrawTextureRec(voro_buffer.texture, {0, 0, float(voro_buffer.texture.width), float(-voro_buffer.texture.height)},
                 {0, 0}, WHITE);
  ctr.endShaderMode();
}
