#version 410 core

layout(triangles, equal_spacing, ccw) in;

//uniform transformation fields
uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

//uniform displacement fields
uniform sampler2D displaceTexture;

//matrix for shadow sampling
uniform mat4 lightMat;

//per-vertex fields passed from buffers
in vec3 evalNormal[];
out vec3 fragNormal;

in vec2 evalTex[];
out vec2 fragTex;

out vec4 fragPosLightSpace;
out vec3 fragPos;


void main() {

    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;

    //barycentric coordinates for position
    vec4 pos = gl_TessCoord.x * p0 + gl_TessCoord.y * p1 + gl_TessCoord.z * p2;

    
    //barycentric coordinates for texture coordinates
    vec2 coord = 
    gl_TessCoord.x * evalTex[0] +
    gl_TessCoord.y * evalTex[1] + 
    gl_TessCoord.z * evalTex[2]; 

    fragTex = coord;

    vec3 normal = 
    gl_TessCoord.x * evalNormal[0] + 
    gl_TessCoord.y * evalNormal[1] + 
    gl_TessCoord.z * evalNormal[2];

    fragNormal = normal;


    //add displacement based on map

    //get the added distance from sampled height map
    float dispMult = texture(displaceTexture, fragTex).r;

    float verticalPos = pos.y + dispMult;

    vec4 finalPos = vec4(pos.x, verticalPos, pos.z, 1.0f);

    gl_Position = projection * view * world * finalPos;


    //get fragPos lightspace
    fragPos = vec3(world * vec4(pos.xyz, 1.0f));
    fragPosLightSpace = lightMat * vec4(fragPos, 1.0f);

}