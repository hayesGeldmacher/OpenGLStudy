#version 330 core
layout(location = 0) in vec3 pos;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

gl_Position = projection * view * world * vec4(pos.x,pos.y,pos.z,1.0f);
