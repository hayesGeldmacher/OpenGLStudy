#version 330 core
layout(location=0) out vec4 color;

in vec4 lightView_Position;

uniform sampler2D shadow // depth map

void main(){
//set whatever else color object should have
color = vec4(1.0f, 0.0f 0.0f, 1.0f);

//mult color by zero if in shadow, or by 1 if not in shadow
//set up depth comparison in the cpp code, so just send z like normal
color *= textureProj(shadow, lightView_Position);
}