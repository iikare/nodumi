#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

out vec4 finalColor;

void main() {
    vec4 texColor = texture(texture0, fragTexCoord);


    if (texColor.a < 0.51) {
      discard;
    }

    //finalColor = mix(texColor, bgColor, vec4(0.5f));
    //texColor.rgb = vec3((texColor.r+texColor.g+texColor.b)/3.0f);

    //texColor.rgb = mix(texColor.rgb, vec3(0,0,0), 2*distance(fragTexCoord, vec2(0.5, 0.5)));
    texColor.a = 1;//smoothstep(0.0f, 1.0f, texColor.a);

    finalColor = texColor;
}

