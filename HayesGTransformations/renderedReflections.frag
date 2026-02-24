#version 330 core
layout(location = 0) out vec4 color;

in vec4 reflectionPosition;
in vec2 vTex;
in vec4 worldPos;
in vec3 vNormal;
in vec3 FragPos;

uniform vec3 viewPos;
uniform samplerCube env;

uniform sampler2D textureColorBuffer;



void main(){


	vec3 viewDir = normalize(FragPos - viewPos);

	//get reflection colors
	vec3 reflection = reflect(viewDir, normalize(vNormal));

	vec4 reflectedColor = texture(env, reflection);
	vec2 projUV = (worldPos.xy / worldPos.w) * 0.5 + 0.5;
	vec4 renderedColor = texture(textureColorBuffer, projUV);

	//color = mix(reflectedColor, renderedColor, renderedColor.a);
	color = renderedColor;
     //color = vec4(reflection, 1.0f);
	
	

}