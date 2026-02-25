#version 330 core
layout(location = 0) out vec4 color;

in vec4 worldPos;
in vec3 FragPos;

uniform vec3 viewPos;
uniform samplerCube env;

uniform sampler2D textureColorBuffer;


void main(){

//get the view direction from camera to pixel 
	vec3 viewDir = normalize(FragPos - viewPos);

//get render texture color
	vec2 projUV = (worldPos.xy / worldPos.w) * 0.5 + 0.5;
	vec4 renderedColor = texture(textureColorBuffer, projUV);

//mix the planar reflections to be a bit dark (just my visual preference, not needed)
	vec3 darkColor = vec3(0.0f, 0.0f, 0.0f);
	vec3 finalColor = mix(darkColor, renderedColor.rgb, 0.7f);
	color = vec4(finalColor, 1.0f);
	
	

}