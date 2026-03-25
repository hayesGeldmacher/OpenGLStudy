#version 330 core

layout(location = 0) out vec4 color;

//per-vertex variables
in vec3 fragNormal;
in vec3 fragPos;
in vec2 fragTex;
in vec4 fragPosLightSpace;

//shadows
uniform sampler2D shadowTexture;

//normal mapping
uniform sampler2D normalTexture;


//uniform lighting variables
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform float specShine;


uniform vec3 viewPos;
uniform vec3 lightPosition;


//calculates how much shadow should be applied to current fragment
float ShadowCalculation(vec4 fragPos){

//ensures this works with both ortho and perspective matrices
	vec3 projCoords = fragPos.xyz / fragPos.w;

	projCoords = projCoords * 0.5 + 0.5;
	float bias  = 0.00005f;

	vec2 TexelSize = 1.0 / textureSize(shadowTexture, 0);
	float shadowSum = 0.0f;

	for(int y = -1; y <= 1; y++){
	
		for(int x = -1; x <= 1; x++){
			vec2 Offset = vec2(x, y) * TexelSize;
			float depth = texture(shadowTexture, projCoords.xy + Offset).x;
			if(depth + bias < projCoords.z){
				shadowSum += 0.0f;
			}
			else{
				shadowSum += 1.0f;
			}
		}
	}

	float finalShadowFactor = shadowSum / 9.0;

	if(projCoords.z > 1.0f){
		finalShadowFactor = 0.0f;
	}
	return finalShadowFactor;



}


void main(){
	
//vec3 rgb_normal = normal * 0.5 + 0.5; //transforms to normal map of -1 to 1
	
	vec3 normal = texture(normalTexture, fragTex).rgb;
	vec3 norm = normalize(normal * 2.0 - 1.0);

	//vec3 norm = normalize(fragNormal);
	vec3 lightDir = normalize(lightPosition - fragPos);

	//get diffuse 
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	
	//get ambient
	vec3 ambientColor = ambientStrength * lightColor;
	

	//get reflections for phong shading
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	vec3 reflectDir = reflect(-lightDir, norm);


	//get specular
	float spec = pow(max(dot(fragNormal, reflectDir), 0.0f), specShine);
	float specularStrength = 0.5f;
	vec3 specular = specularStrength * spec * lightColor;

	vec3 ambientFinal = ambientColor * objectColor;

	//apply shadow to the final coloring wiht lighting


	float shadow = ShadowCalculation(fragPosLightSpace);

	vec3 lighting = (ambientFinal + (shadow)) * (diffuse + specular);
	//vec3 lighting = ambientFinal * (diffuse + specular);
	color = vec4(lighting, 1.0f);

//	color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	

}