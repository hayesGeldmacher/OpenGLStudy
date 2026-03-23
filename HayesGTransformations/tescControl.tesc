#version 410 core

//this will run 3 times, as many as vertices are being input
layout (vertices = 3) out; //runs 3 times per triangle primitive

//returns an array of the outputs for every vertex
//in this case, an array with 4 elements, each a vec3
in vec3 myInData[];

//output array for passing along above data into frag shader
out vec3 myData[];

//per-vertex variables, for sending to frag shader
in vec3 vNormalTesc[];
in vec3 FragPosTesc[];
in vec2 vTexTesc[];
in vec4 fragPosLightSpaceTesc[];

//out matching pairs for above
out vec3 vNormalFrag[];
out vec3 FragPosFrag[];
out vec2 vTexFrag[];
out vec4 fragPosLightSpaceFrag[];

void main(void){

//tricky because shader runs 4 times, but each time has access to other 3
//so really, only one invocation needs to set all the data, the rest can just output

	//setting the tessellation level for each vertex
	gl_TessLevelOuter[0] = 4.0f;
	gl_TessLevelOuter[1] = 4.0f;
	gl_TessLevelOuter[2] = 4.0f;

	gl_TessLevelInner[0] = 4.0f;

	//only outputting one per invocation each time
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	//how we send variables to next fields
	//vNormalFrag[gl_InvocationID].gl_Position = vNormalTesc[gl_InvocationID];

}