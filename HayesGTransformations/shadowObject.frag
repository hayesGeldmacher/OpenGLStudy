#version 330 core

layout(location = 0) out vec4 color;

//per-vertex variables
in vec3 vNormal;
in vec3 FragPos;
in vec3 lightPos;
in vec2 vTex;

//shadows
in vec4 lightView_Position;
uniform sampler2D shadowTexture;



//uniform lighting variables
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

	vec3 ambientFinal = ambientColor * objectColor;
	color = vec4(ambientFinal + diffuse + specular, 1.0f);

	//below will enable shadows once this is working!
	//color *= textureProj(shadow, lightView_Position);


	//color = vec4(1.0f, 0.0f, 1.0f, 1.0f);



}