#version 330

in vec2 fragTexCoord;

uniform vec3 blend_color;

out vec4 finalColor;


void main() {
  //float scaleS = (fragTexCoord.x);
  //float scaleV = 1.0f - (fragTexCoord.y);
  
  finalColor = vec4(blend_color, fragTexCoord.x);
}
