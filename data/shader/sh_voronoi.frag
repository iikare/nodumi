#version 330

in vec2 fragTexCoord;

uniform int vertex_count;
uniform vec2 vertex_data[496];
uniform vec3 vertex_color[496];
uniform vec3 bg_color;
uniform float render_bound;

out vec4 finalColor;

void main() {
    vec2 st = fragTexCoord;

    if (st.y <= render_bound) {
      discard;
    }


    float min_dist = 2.0f;
    int min_index = 0;
    float min_dist_next = 2.0f;
    int min_index_next = 0;

    // flip normals
    st.y = 1.0f-st.y;

    // naive O(n^2) implementation
    for (int i = 0; i < vertex_count; i++) {
        float dist = distance(st, vertex_data[i]);
        if (dist < min_dist) {
            min_dist_next = min_dist;
            min_index_next = min_index;
            min_dist = dist;
            min_index = i;
        }
    }

    // color by track of nearest note
    vec3 color = vertex_color[min_index];

    const float sepRatio = 1.1;

    // bounded by [1.0, sepRatio]
    const float sepFilterRatio = sepRatio-0.001;
    const float sepScale = 1/(sepRatio-sepFilterRatio);

    float alpha = 1.0f;
    float darkScale = 1.0f;

    // cell separator
    if (min_dist_next / min_dist > sepRatio) {
      //color = vec3(1.0f);
      color = bg_color;
    }
    else if (min_dist_next / min_dist > sepFilterRatio) {
      // min_dist_next/min_dist in range 1<[sepFilterRatio, sepRatio]
      //color = mix(color, vec3(1.0f), sepScale*((min_dist_next/min_dist)-sepFilterRatio));
      color = mix(color, bg_color, sepScale*((min_dist_next/min_dist)-sepFilterRatio));
      //darkScale += sepScale*((min_dist_next/min_dist)-sepFilterRatio)/2;
    }
    else if (min_dist_next / min_dist < 1.0001){// && min_dist_next / min_dist > 1.00005){
      //alpha = mix(0.5f, 1.0f, sepScale*((min_dist_next/min_dist)-sepFilterRatio));
      //color = mix(color, vec3(1.0f), sepScale*((min_dist_next/min_dist)-sepFilterRatio));
      //color = mix(color, vec3(1.0f), sepScale*((min_dist_next/min_dist)-sepFilterRatio));
      //darkScale += sepScale*((min_dist_next/min_dist)-sepFilterRatio)/2;
    }

    vec3 max_darken = vec3(clamp(min_dist*darkScale, 0.0f, 1.0f));
    color -= max_darken;
    //color -= vec3(min_dist*darkScale);
    color = clamp(color, vec3(0.0f), vec3(1.0f));
    finalColor = vec4(color, alpha);
}
