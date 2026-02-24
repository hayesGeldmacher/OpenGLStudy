#version 330 core
layout(location = 0) out vec4 color;

in vec3 vNormal;
in vec3 FragPos;
in vec3 lightPos;

//uniform reflection variables
uniform vec3 viewPos;
uniform samplerCube env;

//uniform lighting variables
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform float specShine;
uniform float reflectiveStrength;


void main(){
	
	vec3 norm = normalize(vNormal);
	vec3 lightDir = normalize(lightPos - FragPos);

	//get diffuse 
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	
	//get ambient
	vec3 ambientColor = ambientStrength * lightColor;

	//get reflections for phong shading
	vec3 viewDir = normalize(FragPos - viewPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	vec3 reflectDir = reflect(-lightDir, norm);

	//get specular
	float spec = pow(max(dot(vNormal, reflectDir), 0.0f), specShine);
	float specularStrength = 0.5f;
	vec3 specular = specularStrength * spec * lightColor;

	//get reflection colors
	vec3 reflection = reflect(viewDir, normalize(vNormal));

	//add ambient, diffuse, and spec, multiply by changing object color for final calculation
	vec3 resultColor = ( ambientColor + diffuse + specular);
	
	vec3 reflectedColor = texture(env, reflection).rgb;
	vec3 finalColor = mix(resultColor, reflectedColor, reflectiveStrength);
	color = vec4(finalColor, 1.0f);

}