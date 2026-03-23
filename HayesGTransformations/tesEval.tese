#version 410 core

//accepts quads, equal spacing, counter clockwise order
layout(triangles, equal_spacing, ccw) in;

//access to all data for quad
in vec3 myData[];

void main(){

	//setting just for the current vertex, per-vertex
	gl_Position = vec4(gl_TessCoord.x,
						gl_TessCoord.y,
						0,
						1);

	//BUT we have access to all four verts in the quad
	//gl_Position = interpolate( gl_in[0].gl_Position,
							//	gl_in[1].gl_Position,
							//	gl_in[2].gl_Position,
						//		gl_in[3].gl_Position);

}