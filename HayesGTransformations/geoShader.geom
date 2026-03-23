#version 330 core

layout(triangles, invocations = 2)in;
//max_vertices is an optimization for compiler
layout(line_strip, max_vertices = 5) out;

in vec2 someData[] //array received from vert

out vec2 vData; //data to send to frag shader

uniform float value // takes uniform values like anything else

void main(){

	//different invocation id for instances
	if(gl_InvocationID == 0){
		//do stuff for first instance
	}
	else{
		//do stuff for second instance
	}
	
	
	vData = someData[0];


	//what we take as input doesn't have to be related to what we output!
	//if we get triangles but output line-strips, what matters isn't the triangles - 
	//what matters is the line strip

	//vertex counts 

	//takes a triangle and outputs those tree vertices as a line strip

	gl_Position = gl_in[0].gl_Position;
	//outputs a vertex
	EmitVertex();
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();

	//ends the current line strip
	EndPrimitive();
	
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();

	//emit vertex exactly between 0 and 1 points
	gl_Position =  (gl_in[0].gl_Position + gl_in[2].gl_Position) / 2;
	EmitVertex();

	//dont have tot call endPrimitive at the end of function, 
	//as it automatically ends at end of execution

}

