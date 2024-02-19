#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

struct DirLight {
	sampler2D depthBuffer;
};
uniform DirLight dirLight;

uniform sampler2D bufferTex;

float near = 0.1;
float far = 100.0;

void main() {
	// DEPTH MAP (Perspective)
	//float depth = texture(bufferTex, TexCoords).r;
	//float z = depth * 2.0 - 1;
	//float linearDepth = (2.0 * near * far) / (depth * (far - near) - (far + near)); // taking the inverse of projection matrix (perspective)
	//float factor = (near + linearDepth) / (near - far); // convert from back to [0, 1]

	//FragColor = vec4(vec3(1 - factor), 1.0);

	// DEPTH MAP (ortho)
	FragColor = vec4(vec3(texture(dirLight.depthBuffer, TexCoords).r), 1.0);

	// COLOR MAP
	//FragColor = texture(bufferTex, TexCoords);
}
