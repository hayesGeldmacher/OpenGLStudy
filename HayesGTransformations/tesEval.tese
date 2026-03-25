#version 410 core

layout(triangles, equal_spacing, ccw) in;


uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

void main() {

    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;

    //barycentric coordinates
    vec4 pos = gl_TessCoord.x * p0 + gl_TessCoord.y * p1 + gl_TessCoord.z * p2;

    gl_Position = projection * view * world * pos;
}