#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <stb/stb_image.h>

#include <string>
#include <vector>

#include "shader.h"
#include "../memory/vertexmemory.hpp"

class Scene;

class CubeMap {
public:
	// texture object
	unsigned int id;

	CubeMap();

	void generate();

	void bind();

	void loadTexture(std::string _dir,
		std::string right	= "right.png",
		std::string left	= "left.png",
		std::string top		= "top.png",
		std::string bottom	= "bottom.png",
		std::string front	= "front.png",
		std::string back	= "back.png");

	void allocate(GLenum format, GLuint widht, GLuint height, GLenum type);

	void init();

	void render(Shader shader, Scene *scene);

	void cleanup();

private:
	
	std::string directory;
	std::vector<std::string> faces;
	bool hasTextures;

	// cube object
	// array object VAO
	ArrayObjects VAO;

};

#endif // !CUBEMAP_H
