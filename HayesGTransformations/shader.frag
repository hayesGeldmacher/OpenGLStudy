#version 330 core

layout(location = 0) out vec4 color;
in vec3 vNormal;
in vec3 FragPos;
in vec3 lightPos;
in vec3 vColor; 
in vec2 vTex;

uniform sampler2D diffuseTex;
uniform sampler2D specTex;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform float specShine;
uniform vec3 viewPos;


void main(){

	vec3 norm = normalize(vNormal);
	vec3 lightDir = normalize(lightPos - FragPos);

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

	//set up final ambient color as a multiplication of the diffuse texture with light color
	vec3 ambientFinal = ambientColor * vec3(texture(diffuseTex, vTex));

	//set up final diffuse color
	vec3 diffuseFinal = diffuse * vec3(texture(diffuseTex, vTex));

	//set up final spec color
	vec3 specFinal = specular * vec3(texture(specTex, vTex));

	//add ambient, diffuse, and spec, multiply by changing object color for final calculation
	color = vec4(ambientFinal + diffuseFinal + specFinal, 1.0f);

}