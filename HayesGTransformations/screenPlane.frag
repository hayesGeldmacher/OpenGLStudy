#version 330 core
out vec4 color;
 
in vec2 vTex;

uniform sampler2D AO;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColorSpec;

//uniform lighting variables
uniform vec3 lightColor;

uniform vec3 viewPos;
uniform vec3 lightPosition;
uniform mat4 projection;


void main()
{             
	//retrieve data from G-Buffer
	vec3 fragPos = texture(gPosition, vTex).rgb;
	vec3 normal = texture(gNormal, vTex).rgb;
	vec3 albedo = texture(gColorSpec, vTex).rgb;
	float specular = texture(gColorSpec, vTex).a;
	float occlusion = texture(AO, vTex).r;

	//for now, we are using just standard albedo
   	color = vec4(vec3(albedo) ,1.0f);
	color *= occlusion;
}  