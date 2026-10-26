#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

void main() {
    // texture coordinates are 0 at the dot's center and 1 on its edge, so one pixel is their step across it
    float pixel = abs(dFdx(fragTexCoord.x));
    float cover = clamp((1.0f - length(fragTexCoord)) / pixel + 0.5f, 0.0f, 1.0f);
    if (cover <= 0.0f) {
      discard;
    }
    finalColor = vec4(fragColor.rgb, fragColor.a * cover);
}
