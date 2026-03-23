#version 410 core

//this will run 4 times, as many as vertices are being input
layout (vertices = 4) out;

//returns an array of the outputs for every vertex
//in this case, an array with 4 elements, each a vec3
in vec3 myInData[];

//output array for passing along above data into frag shader
out vec3 myData[];

void main(void){

//tricky because shader runs 4 times, but each time has access to other 3
//so really, only one invocation needs to set all the data, the rest can just output

	//setting the tessellation level for each vertex
	gl_TessLevelOuter[0] = 2.0f;
	gl_TessLevelOuter[1] = 4.0f;
	gl_TessLevelOuter[2] = 6.0f;
	gl_TessLevelOuter[3] = 8.0f;

	gl_TessLevelInner[0] = 8.0f;
	gl_TessLevelInner[1] = 8.0f;

	//only outputting one per invocation each time
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

}