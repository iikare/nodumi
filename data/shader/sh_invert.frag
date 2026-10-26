#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

out vec4 finalColor;

void main() {
    // the blend inverts what is under by this much, so faded lines and the soft edges of text invert partly,
    // while the thin strokes of small text, which are never fully opaque, still invert all the way
    float amount = smoothstep(0.05f, 0.6f, texture(texture0, fragTexCoord).a) * fragColor.a;
    if (amount <= 0.0f) {
      discard;
    }
    finalColor = vec4(vec3(amount), amount);
}
