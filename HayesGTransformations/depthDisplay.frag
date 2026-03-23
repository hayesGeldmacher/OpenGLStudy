#version 330 core
out vec4 color;
  
in vec2 vTex;

uniform sampler2D depthMap;
uniform sampler2D normalMap;
uniform float near_plane;
uniform float far_plane;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0f;
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));

    

}

void main()
{             
    float depthValue = texture(depthMap, vTex).r;
    color = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0f);
    //color = vec4(vec3(depthValue), 1.0);

    	vec3 normalColor = texture(normalMap, vTex).rgb;
	//color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
	color = vec4(normalColor, 1.0f);

}  