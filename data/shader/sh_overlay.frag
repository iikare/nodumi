#version 330
in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform vec2 u_resolution;
uniform vec3 line_color;
uniform int use_line_color;
uniform int hover;

out vec4 finalColor;

vec3 blend(vec3 fg, vec3 bg) {
  return mix(1.0 - 2.0 * (1.0 - bg) * (1.0 - fg), 2.0 * bg * fg, step(bg, vec3(0.5)));
}

void main() {
  vec2 st = fragTexCoord;
  st.y = 1.0f - st.y;
  vec2 i_res = vec2(1.0)/u_resolution;
  vec4 tex_color_a = texture(texture0, st);
  vec4 tex_color_b = texture(texture1, st);

  if (use_line_color != 0) {
    finalColor = vec4(blend(tex_color_a.rgb, tex_color_b.rgb), tex_color_b.a);
    if (hover != 0) { // brightened state by default
      finalColor.a = finalColor.a / 2.0f; 
    }
  }
  else {
    // uses overlay blending
    finalColor = vec4(line_color, 1.0f);
    if (hover == 0) {
      finalColor.a = finalColor.a / 2.0f; 
    }
  }

}
