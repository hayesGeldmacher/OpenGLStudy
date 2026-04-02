#version 330 core
out vec4 color;
 
in vec2 vTex;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColorSpec;

//struct Light {
	//vec3 Position;
	//vec3 Color;
//}

//const int NR_LIGHTS = 32;
//uniforrm Light lights[NR_LIGHTS];
//uniform vec3 viewPos;


void main()
{             
	//retrieve data from G-Buffer
	vec3 fragPos = texture(gPosition, vTex).rgb;
	vec3 normal = texture(gNormal, vTex).rgb;
	vec3 albedo = texture(gColorSpec, vTex).rgb;
	float specular = texture(gColorSpec, vTex).a;

	//vec3 lighting =  albedo * 0.1f; //get a hard-coded value for ambient lighting
	//vec3 viewDir = normalize(viewPos - fragPos);
	//for(int i = 0; i < NR_LIGHTS; i++){
	
		/////get diffuse
		//vec3 lightDir = noramlize(lights[i].Position - fragPos);
		//vec3 diffuse = max(dot(normal, lightDir), 0.0f) * albedo * lights[i].Color;
		//lighting += diffuse;
	//}

	//color = vec4(lighting, 1.0f);
   
   color = vec4(1.0f, 0.0f, 0.0f, 0.0f);
   color = vec4(albedo, 1.0f);

}  