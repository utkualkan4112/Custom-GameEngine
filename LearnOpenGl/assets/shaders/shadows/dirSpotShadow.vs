#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 4) in mat4 model;

out vec4 FragPos;

uniform mat4 lightSpaceMatrix;

void main() {
	// ~ projection * view * model * pos

	FragPos = vec4(model * vec4(aPos, 1.0));
	gl_Position = lightSpaceMatrix * FragPos;

}