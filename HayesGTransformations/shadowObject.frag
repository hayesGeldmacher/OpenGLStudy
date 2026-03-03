#version 330 core

layout(location = 0) out vec4 color;

//per-vertex variables
in vec3 vNormal;
in vec3 FragPos;
in vec2 vTex;
in vec4 fragPosLightSpace;

//shadows
uniform sampler2D shadowTexture;



//uniform lighting variables
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform float specShine;


uniform vec3 viewPos;
uniform vec3 lightPosition;



float ShadowCalculation(vec4 fragPos){

//ensures this works with both ortho and perspective matrices
	vec3 projCoords = fragPos.xyz / fragPos.w;

	projCoords = projCoords * 0.5 + 0.5;

	//get the closest depth on the depth map
	float closestDepth = texture(shadowTexture, projCoords.xy).r;

	//get our current depth
	float currentDepth = projCoords.z;

	//if closest, set to 1 - otherwise, set to 0
	float shadow = currentDepth > closestDepth ? 1.0f : 0.0f;

	return shadow;


}

void main(){

	vec3 norm = normalize(vNormal);
	vec3 lightDir = normalize(lightPosition - FragPos);

	//get diffuse 
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	
	//get ambient
	vec3 ambientColor = ambientStrength * lightColor;

	//get reflections for phong shading
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	vec3 reflectDir = reflect(-lightDir, norm);


	//get specular
	float spec = pow(max(dot(vNormal, reflectDir), 0.0f), specShine);
	float specularStrength = 0.5f;
	vec3 specular = specularStrength * spec * lightColor;

	vec3 ambientFinal = ambientColor * objectColor;

	float shadow = ShadowCalculation(fragPosLightSpace);
	vec3 lighting = (ambientFinal + (1.0f - shadow)) * (diffuse + specular);
	color = vec4(lighting, 1.0f);

	//color = vec4(ambientFinal + diffuse + specular, 1.0f);


}