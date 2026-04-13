#version 330 core
layout (location = 0) out float fragColor;


in vec2 vTex;
out vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform sampler2D gColorSpec;

uniform mat4 projection;
uniform int power;

uniform float radius;
uniform float bias;

//tile noise texture of the screen
const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0); //screen is 800x800

uniform vec3 samples[64]; //the random sampling positions 

void main(){

	//pass texture coordinates to the blur shader
	TexCoords = vTex;

	//get the fragment position from gBuffer texture
	vec3 fragPos = texture(gPosition, vTex).xyz;

	//get the normal postion from gBuffer texture
	vec3 normal = texture(gNormal, vTex).rgb;

	//get albedo texture from gBuffer
	vec3 albedo = texture(gColorSpec, vTex).rgb;

	//get random noise
	vec3 randomVec = texture(texNoise, vTex * noiseScale).xyz;

	//TBN matrix, transform tangent-to-view space:
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitTangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitTangent, normal);

	float occlusion = 0.0f;
	int kernelSize = 64;

	for(int i = 0; i < kernelSize; i++){
		
		//transform from tangent space to view-sapce
	   vec3 samplePos = TBN * samples[i]; 
	   samplePos = fragPos + samplePos * radius;

	   //next transform to screenspace to sample values like from the quad
	   vec4 offset = vec4(samplePos, 1.0f);
	   offset = projection * offset; //go from view to clip space
	   offset.xyz /= offset.w; //do perspective divid for normalize device coordinates
	   offset.xyz = offset.xyz * 0.5f + 0.5f; //transform to 0.0-1.0 range

	   //get the first non-occluded visible fragmnet
	   float sampleDepth = texture(gPosition, offset.xy).z;

	   //do a range check to ensure we aren't sampling geometry far behind the model
	   //smooth step interpolates the range, avoiding ugly harsh borders on the model
	   float rangeCheck = smoothstep(0.0, 1.0f, radius / abs(fragPos.z - sampleDepth));
	   
	   //if the sampled depth is greater than the sample position depth, add to occlusion factor
	   occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	//normalize occusion by kernel size
	occlusion = 1.0 - (occlusion/kernelSize);
	fragColor = pow(occlusion, power);
}