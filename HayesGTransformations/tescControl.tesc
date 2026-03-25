#version 410 core
layout (vertices = 3) out; // Output 3 control points per patch

uniform float tessLevel;

in vec4 vs_position[];

void main() {

    if(gl_InvocationID == 0){
            gl_TessLevelOuter[0] = tessLevel; 
            gl_TessLevelOuter[1] = tessLevel;
            gl_TessLevelOuter[2] = tessLevel;
            gl_TessLevelInner[0] = tessLevel;
    }
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}