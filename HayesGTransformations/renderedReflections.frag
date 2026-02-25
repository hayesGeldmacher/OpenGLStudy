#version 330 core
layout(location = 0) out vec4 color;

in vec4 reflectionPosition;
in vec2 vTex;
in vec4 worldPos;
in vec3 vNormal;
in vec3 FragPos;

//uniform reflection variables
uniform vec3 viewPos;
uniform samplerCube env;
uniform sampler2D textureColorBuffer;

void main(){
	
	//get reflections for phong shading
	vec3 viewDir = normalize(FragPos - viewPos);

	//get reflection colors
	vec3 reflection = reflect(viewDir, normalize(vNormal));


	float texX = gl_FragCoord.x / 800;
	float texY = gl_FragCoord.y / 800;
	vec2 projUV = vec2(texX, texY);

	//vec2 projUV = (worldPos.xy / worldPos.w) * 0.5 + 0.5;
	vec4 renderedColor = texture(textureColorBuffer, projUV);

	
	vec4 reflectedColor = texture(env, reflection);
	vec4 darkColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	//color = vec4(reflectedColor);
	//color = vec4(renderedColor);

	color = mix(renderedColor, darkColor, 0.5f);

	//vec3 finalColor = mix(renderedColor.rgb, reflectedColor.rgb, 0.7);
	//color = vec4(finalColor, 1.0f);

	// color = vec4(vec3(renderedColor + reflectedColor), 1.0f);

}