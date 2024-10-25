#version 330

#define M_PI 3.14159

in vec2 fragTexCoord;

uniform float ring_len;
uniform float ring_width;

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

void main() {
    if (fragTexCoord.x == 0.5 && fragTexCoord.y == 0.5) {
      discard;
    }
    
    float alpha = 1.0f;
    float alpha_dist = 0.01f;
    
    float dist = distance(fragTexCoord, vec2(0.5f, 0.5f));

    if (dist > ring_len+alpha_dist ||
        dist < ring_len - ring_width-alpha_dist) {
      discard;
    }
    else if (dist < ring_len+alpha_dist && dist > ring_len) {
      alpha = (1.0f/alpha_dist)*(ring_len+alpha_dist-dist);
    }
    else if (dist > ring_len-ring_width-alpha_dist && dist < ring_len - ring_width) {
      alpha = (1.0f/alpha_dist)*(dist-(ring_len-ring_width-alpha_dist));
    }
    
    float h = -atan(fragTexCoord.y-0.5, fragTexCoord.x-0.5)/(2*M_PI);
        
    finalColor = vec4(hsv2rgb(vec3(h, 1.0f, 1.0f)), alpha);
}
