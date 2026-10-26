#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

#include "build_target.h"
#include "color.h"
#include "data.h"
#include "log.h"

using std::vector;

class voronoiController {
 public:
  void init();

  void unloadData() { unload(); }
  void update();

  // starts this frame's sites and dots, then takes one site per note at its dot's screen position, where a
  // later note on the same pixel replaces the earlier one, as its dot is drawn over it and two sites in one
  // spot would turn their whole cell into border, and returns the site's index
  void clear_sites();
  int add_site(float x, float y, const colorRGB& col);

  // adds a note's dot, centered on the site add_site gave back for it
  void add_dot(int site, float x, float y, float radius, const colorRGB& col);

  // draws the cells of this frame's sites, below voro_y
  void render(int voro_y);

  // draws the dots in note order, leaving out any that later dots on the same pixel cover
  void draw_dots();

  // this frame's sites, from 0 to 1 across the screen with y up, and their colors
  vector<Vector2> vertex;
  vector<colorRGB> color;

  // a white texture across the window, which the shader is drawn over
  Texture2D tex;

 private:
  void unload();

  void updateTexture();

  // sorts the sites into a grid and uploads them, so each pixel only searches the cells near it
  void upload_sites();
  void fit_texture(Texture2D& data_tex, int width, int height);

  Texture2D site_tex = {};
  Texture2D color_tex = {};
  Texture2D cell_tex = {};

  vector<float> site_buffer;
  vector<float> color_buffer;
  vector<float> cell_buffer;
  vector<int> cell_of;
  vector<int> cell_fill;

  // each site's pixel, and an open addressing table from pixels to sites, kept between frames
  vector<int64_t> site_pixel;
  vector<int> site_table;
  void grow_site_table();

  struct dot_info {
    Vector2 center;
    float radius;
    colorRGB color;
    int site;
    bool shown;
  };
  vector<dot_info> dots;

  // for each site, a disc around its last note's dot that the dots of later notes already hide completely
  struct dot_cover {
    Vector2 center;
    float radius;
  };
  vector<dot_cover> site_cover;
};
