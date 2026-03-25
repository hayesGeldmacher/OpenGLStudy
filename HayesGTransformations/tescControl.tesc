#version 410 core
layout (vertices = 3) out; // Output 3 control points per patch

uniform float tessLevel;

in vec3 tcsNormal[];
out vec3 evalNormal[];

in vec2 tcsTex[];
out vec2 evalTex[];

void main() {

    if(gl_InvocationID == 0){
            gl_TessLevelOuter[0] = tessLevel; 
            gl_TessLevelOuter[1] = tessLevel;
            gl_TessLevelOuter[2] = tessLevel;
            gl_TessLevelInner[0] = tessLevel;
    }
    //pass along position data
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    //pass along normal data
    evalNormal[gl_InvocationID] = tcsNormal[gl_InvocationID];

    //pass along texture coordinate data
    evalTex[gl_InvocationID] = tcsTex[gl_InvocationID];
}