#version 330 core
layout (location = 0) out float fragColor;

//texture coordinates for screen space plane
in vec2 vTex;

//gbuffer variables
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform sampler2D gColorSpec;

//uniform variables
uniform mat4 projection;
uniform int kernelSize;
uniform int power;
uniform float bias;

uniform float nestedRadius[3]; //radius for each hemisphere
uniform float textureLevels[3]; // lod level for each hmeisphere

//tile noise texture over the screen
const vec2 noiseScale = vec2(800.0/4.0, 800/4.0); //screen is 800x800

uniform vec3 samples[64]; //the random sampling positions 

float ComputeKernelSSAO(){

	//get the fragment position from gBuffer texture
	vec3 fragPos = texture(gPosition, vTex).xyz;
	
	//get the normal postion from gBuffer texture
	vec3 normal = texture(gNormal, vTex).rgb;

	//get albedo texture from gBuffer
	vec3 albedo = texture(gColorSpec, vTex).rgb;

	float totalOcclusion = 0.0f;

	//get random noise
	vec3 randomVec = texture(texNoise, vTex * noiseScale).xyz;

	//TBN matrix, transform tangent to view space:
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitTangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitTangent, normal);

	int startSample = 0;
	int endSample = 22; 

	//for loop, go through each nested hemisphere
	for(int i = 0; i < 3; ++i){
	
		float scaleOcclusion = 0.0;
		float currentRadius = nestedRadius[i]; //get the desired radius
	    float currentLod = textureLevels[i]; //get the desired lod
		
		//inner for loop, get random samples
		for(int j = startSample; j < endSample; ++j){
			
			vec3 samplePos = TBN * samples[j]; 
			samplePos = fragPos + samplePos * currentRadius;

			//next transform to screenspace to sample values like from the quad
			vec4 offset = vec4(samplePos, 1.0f);
			offset = projection * offset; //go from view to clip space
			offset.xyz /= offset.w; //do perspective divid for normalize device coordinates
			offset.xyz = offset.xyz * 0.5f + 0.5f; //transform to 0.0-1.0 range

			float sampleDepth = textureLod(gPosition, offset.xy, currentLod).z;
			 
			//do a range check to ensure we aren't sampling geometry far behind the model
		    //smooth step interpolates the range, avoiding ugly harsh borders on the model
		    float rangeCheck = smoothstep(0.0, 1.0f, currentRadius / abs(fragPos.z - sampleDepth));
	   
		    //if the sampled depth is greater than the sample position depth, add to occlusion factor
		     scaleOcclusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
		}
			
		startSample += 20;
		endSample += 20;

		//add to total occlusion factor
		totalOcclusion += ( scaleOcclusion / float(64));

	}

		return totalOcclusion / 3.0; //divided by three cause we are using three resolutions
}

void main(){
	fragColor = ComputeKernelSSAO();
	fragColor = 1 - fragColor;
	fragColor = pow(fragColor, power);
}