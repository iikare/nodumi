#version 330

in vec2 fragTexCoord;

uniform int vertex_count;
uniform vec2 vertex_data[496];
uniform vec3 vertex_color[496];

out vec4 finalColor;

void main() {
    vec2 st = fragTexCoord;
    float min_dist = 2.0f;
    int min_index = 0;
    float min_dist_next = 2.0f;
    int min_index_next = 0;

    // flip normals
    st.y = 1.0f-st.y;

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

    // cell separator
    if (min_dist_next / min_dist > 1.1) {
      color = vec3(1.0f);
    }

    color -= vec3(min_dist*1.0f);
    finalColor = vec4(color,1.0f);
}
