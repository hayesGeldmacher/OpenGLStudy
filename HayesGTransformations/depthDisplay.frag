#version 330 core
out vec4 color;
  
in vec2 vTex;

uniform sampler2D depthMap;

void main()
{             
    float depthValue = texture(depthMap, vTex).r;
    color = vec4(vec3(depthValue), 1.0);

}  