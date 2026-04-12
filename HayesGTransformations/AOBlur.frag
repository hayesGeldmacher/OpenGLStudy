#version 330 core
layout (location = 0) out float fragColor;

in vec2 TexCoords;
uniform sampler2D AO;
uniform sampler2D gNormal;
uniform sampler2D gPosition;
uniform sampler2D gColorSpec;

void main(){

	//get size of texel based on stored AO texture size
	vec2 texelSize = 1.0f / vec2(textureSize(AO, 0));

	float fragColor = 0;
	//go through surrounding texels, offset, and average to obtain a simple blur
	float result = 0.0f;
	for(int x = 2; x < 2; x++){
		
		for(int y = -2; y < 2; y++){
			
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(AO, TexCoords + offset).r;
		}
	}
	
	fragColor = result / (4.0 * 4.0);
}