#version 330

in vec2 fragTexCoord;

// site positions, sorted by grid cell, and their colors, both laid out row by row
uniform sampler2D site_data;
uniform sampler2D site_color;
// each grid cell's first site, its site count and how many rings out the nearest occupied cell is
uniform sampler2D cell_data;
uniform int site_tex_width;
uniform int grid_width;
uniform int grid_height;
uniform vec2 grid_origin;
uniform float cell_size;
uniform vec3 bg_color;
uniform float render_bound;
// the window in pixels, which the sites span from 0 to 1
uniform vec2 screen_size;

out vec4 finalColor;

// this pixel, and the nearest and second nearest site found so far, as squared distances
vec2 st;
float min_d2;
float next_d2;
int min_index;
int next_index;
vec2 min_site;
vec2 next_site;

ivec2 site_texel(int index) {
    return ivec2(index % site_tex_width, index / site_tex_width);
}

// tests a cell's sites, unless it is off the grid or too far away to hold either of the nearest two
void visit(ivec2 cell) {
    if (cell.x < 0 || cell.y < 0 || cell.x >= grid_width || cell.y >= grid_height) {
      return;
    }
    vec2 low = grid_origin + vec2(cell) * cell_size;
    vec2 gap = max(max(low - st, st - (low + cell_size)), 0.0f);
    if (dot(gap, gap) >= next_d2) {
      return;
    }
    vec2 span = texelFetch(cell_data, cell, 0).xy;
    int first = int(span.x);
    int last = first + int(span.y);
    for (int i = first; i < last; i++) {
        vec2 site = texelFetch(site_data, site_texel(i), 0).xy;
        vec2 delta = st - site;
        float d2 = dot(delta, delta);
        if (d2 < min_d2) {
          next_d2 = min_d2;
          next_index = min_index;
          next_site = min_site;
          min_d2 = d2;
          min_index = i;
          min_site = site;
        }
        else if (d2 < next_d2) {
          next_d2 = d2;
          next_index = i;
          next_site = site;
        }
    }
}

void main() {
    st = fragTexCoord;

    if (st.y <= render_bound) {
      discard;
    }

    // flip normals
    st.y = 1.0f-st.y;

    min_d2 = 100.0f;
    next_d2 = 100.0f;
    min_index = -1;
    next_index = -1;
    min_site = st;
    next_site = st;

    vec2 cell_pos = (st - grid_origin) / cell_size;
    ivec2 cell = clamp(ivec2(floor(cell_pos)), ivec2(0), ivec2(grid_width - 1, grid_height - 1));
    // how far the pixel is from the edge of its own cell, the least distance to any cell outside it
    vec2 in_cell = cell_pos - vec2(cell);
    float edge = max(0.0f, min(min(in_cell.x, 1.0f - in_cell.x), min(in_cell.y, 1.0f - in_cell.y)));
    edge *= cell_size;

    // search rings of cells outwards, from the first one with a site in it, until no unvisited site can beat
    // the second nearest
    int max_ring = max(grid_width, grid_height);
    for (int ring = int(texelFetch(cell_data, cell, 0).z); ring <= max_ring; ring++) {
        if (ring == 0) {
          visit(cell);
        }
        else {
          // the ring is walked from the middle of its sides to the corners, so once a step's cells are past the
          // second nearest, the rest of the ring is too
          float side = float(ring - 1) * cell_size + edge;
          for (int t = 0; t <= ring; t++) {
              float along = t > 0 ? float(t - 1) * cell_size + edge : 0.0f;
              if (side * side + along * along >= next_d2) {
                break;
              }
              visit(cell + ivec2(ring, t));
              visit(cell + ivec2(-ring, t));
              if (t > 0) {
                visit(cell + ivec2(ring, -t));
                visit(cell + ivec2(-ring, -t));
              }
              if (t < ring) {
                visit(cell + ivec2(t, ring));
                visit(cell + ivec2(t, -ring));
                if (t > 0) {
                  visit(cell + ivec2(-t, ring));
                  visit(cell + ivec2(-t, -ring));
                }
              }
          }
        }
        float reach = float(ring) * cell_size + edge;
        if (next_d2 <= reach * reach) {
          break;
        }
    }

    float min_dist = sqrt(min_d2);
    float min_dist_next = sqrt(next_d2);
    // kept finite on the site itself, so the smoothing below stays defined around it
    float ratio = min(min_dist_next / max(min_dist, 1e-6f), 1000.0f);

    // color by track of nearest note
    vec3 color = min_index < 0 ? bg_color : texelFetch(site_color, site_texel(min_index), 0).rgb;
    vec3 next_color = next_index < 0 ? color : texelFetch(site_color, site_texel(next_index), 0).rgb;

    float sepRatio = 1.03;

    const float addSep = 0.07f;
    float addRatio = 1.0-clamp(min_dist*100, 0.0f, 1.0f);
    sepRatio += addSep*addRatio;

    float alpha = 1.0f;
    float darkScale = 1.0f;

    // edges are smoothed from exact distances rather than screen derivatives, which change with how the 2x2
    // pixel blocks fall on the cells and made thin borders flicker as the notes scroll
    vec2 to_min = (st - min_site) / max(min_dist, 1e-6f);
    vec2 to_next = (st - next_site) / max(min_dist_next, 1e-6f);
    // how fast ratio - sepRatio grows per pixel, sepRatio itself rising towards the site
    vec2 grow = (to_next - ratio * to_min) / max(min_dist, 1e-6f);
    if (min_dist < 0.01f) {
      grow += addSep * 100.0f * to_min;
    }
    float band_px = (ratio - sepRatio) / max(length(grow / screen_size), 1e-6f);
    // distance to the border between the two cells, a straight line halfway between their sites
    float gap_px = length((next_site - min_site) / screen_size);
    float border_px = next_index < 0 ? 1e6f : (next_d2 - min_d2) / max(2.0f * gap_px, 1e-12f);

    // both sites are equally near on the cell border, so the two colors meet halfway there
    color = mix(color, next_color, 0.5f * (1.0f - smoothstep(0.0f, 1.0f, border_px)));

    // cell separator, never thinner than a pixel, so narrow borders stay solid lines instead of fading out
    float outside = smoothstep(-0.5f, 0.5f, band_px);
    outside = min(outside, smoothstep(0.25f, 0.75f, border_px));
    color = mix(color, bg_color, outside);

    vec3 max_darken = vec3(clamp(min_dist*darkScale, 0.0f, 1.0f));
    color -= max_darken;
    color = clamp(color, vec3(0.0f), vec3(1.0f));
    finalColor = vec4(color, alpha);
}
