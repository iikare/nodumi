#version 330

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 u_resolution;
uniform vec2 box_dim;
uniform vec2 box_pos;
uniform vec3 blend_color;

out vec4 finalColor;

vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{

    vec2 ss_box_dim = box_dim/u_resolution;
    vec2 ss_box_pos = box_pos/u_resolution;

    if (fragTexCoord.x <= ss_box_pos.x ||
        fragTexCoord.y <= ss_box_pos.y ||
        fragTexCoord.x > ss_box_pos.x + ss_box_dim.x || 
        fragTexCoord.y > ss_box_pos.y +ss_box_dim.y) {
      discard;
    }

    float scaleS = (fragTexCoord.x - ss_box_pos.x) / (ss_box_dim.x);
    float scaleV = 1.0f - (fragTexCoord.y - ss_box_pos.y) / (ss_box_dim.y);

    finalColor = vec4(hsv2rgb(rgb2hsv(blend_color)*vec3(1.0f,scaleS,scaleV)),1.0f);
}
