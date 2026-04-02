#version 330 core
out vec4 color;
 
in vec2 vTex;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColorSpec;



//uniform lighting variables
uniform vec3 lightColor;
uniform float specShine;

uniform vec3 viewPos;
uniform vec3 lightPosition;


void main()
{             
	//retrieve data from G-Buffer
	vec3 fragPos = texture(gPosition, vTex).rgb;
	vec3 normal = texture(gNormal, vTex).rgb;
	vec3 albedo = texture(gColorSpec, vTex).rgb;
	float specular = texture(gColorSpec, vTex).a;

	vec3 lighting =  albedo * 0.1f; //get a hard-coded value for ambient lighting
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 lightDir = normalize(lightPosition - fragPos);

	//so, something here is zeroing out
	vec3 testRedColor = vec3(1.0f, 0.0f, 0.0f);
    float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * testRedColor;
	//vec3 diffuse = max(dot(normal, lightDir), 0.0f) * albedo * testRedColor;



	lighting += diffuse;
	color = vec4(lighting, 1.0f);
   
  // color = vec4(1.0f, 0.0f, 0.0f, 0.0f);
  // color = vec4(normal, 1.0f);

}  