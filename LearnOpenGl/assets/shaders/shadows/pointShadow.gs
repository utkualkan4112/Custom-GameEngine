#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out; // 6 faces = 6 triangles = 18 vertices

uniform mat4 lightSpaceMatrices[6];

out vec4 FragPos;

void main() {
	for( int face = 0; face < 6; ++face){
		// built in variable to determine face on cubemap
		gl_Layer = face;

		// render triangle
		for(int i = 0; i < 3; ++i){
			FragPos = gl_in[i].gl_Position;
			gl_Position = lightSpaceMatrices[face] * FragPos;
			EmitVertex();
		}

		EndPrimitive();
	}
}