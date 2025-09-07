#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform float thr_bw;
uniform float thr_r;
uniform float thr_g;
uniform float thr_b;
uniform int use_bw;
uniform int use_rgb;

out vec4 finalColor;

void main() {
  vec2 st = fragTexCoord;
  vec4 texColor = texture(texture0, st);

  if (use_bw != 0) {
    texColor = texColor * vec4(vec3(thr_bw), 1.0f);
  }
  if (use_rgb != 0) {
    texColor = texColor * vec4(thr_r, thr_g, thr_b, 1.0f);
  }

  finalColor = texColor;
}
