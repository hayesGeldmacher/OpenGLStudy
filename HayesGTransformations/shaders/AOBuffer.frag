#version 330 core
 layout (location = 0) out vec3 gPosition;
 layout (location = 1) out vec3 gNormal;
 layout (location = 2) out vec4 gColorSpec;
 out vec4 color;

  in vec2 vTex;
  in vec3 fragPos;
  in vec3 vNormal;

  uniform vec3 objectColor;
  uniform int hasTexture;
  uniform sampler2D albedoTexture;

void main()
{             
	if(hasTexture == 0){
		//store diffuse color in texture
		gColorSpec.rgb = vec3(objectColor);	
	}
	else{
		gColorSpec.rgb = vec3(texture(albedoTexture, vTex));
	}

	//store position of the fragment in the first gBuffer texture
	gPosition = fragPos;

	//store normals in texture
	gNormal = normalize(vNormal);


}  