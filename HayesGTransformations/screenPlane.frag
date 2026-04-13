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

uniform mat4 view;

void main()
{          

	vec3 transformedLightPosition = vec3(view * vec4(lightPosition, 1.0f));

	//just testing a few values
	float Linear = 0.007;
	float Quadratic = 0.002f;
	float radius = 50.0f;

	//retrieve data from G-Buffer
	vec3 fragPos = texture(gPosition, vTex).rgb;
	vec3 normal = texture(gNormal, vTex).rgb;
	vec3 albedo = texture(gColorSpec, vTex).rgb;
	float specularIntensity = texture(gColorSpec, vTex).a;
	float occlusion = texture(AO, vTex).r;

	//blinn-phong (in view space)
	vec3 ambient = vec3(0.3f * albedo * occlusion);
	vec3 lighting = ambient;
	vec3 viewDir = normalize(-fragPos); //in screenspace, viewpos 0,0,0


	//this is causing the bug - light pos is in worldspace, while frag pos is in world-view space
	//get diffuse
	vec3 lightDir = normalize(transformedLightPosition - fragPos);
	vec3 diffuse = max(dot(normal, lightDir), 0.0) * albedo * lightColor;

	//get specular
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 8.0);
	vec3 specular = lightColor * spec;

	//get attenuation
	float dist = length(transformedLightPosition - fragPos);
	float attenuation = 1.0 / (1.0 + Linear * dist + Quadratic * (dist * dist));
	diffuse *= attenuation;
	specular *= attenuation;
	lighting += diffuse + specular;

	color = vec4(lighting, 1.0f);

	//for now, we are using just standard albedo
   	color = vec4(vec3(normal) ,1.0f);
	color *= occlusion;
}  