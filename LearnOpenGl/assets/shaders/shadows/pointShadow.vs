#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 4) in mat4 model;


void main(){
	gl_Position = vec4(model * vec4(aPos, 1.0));
}