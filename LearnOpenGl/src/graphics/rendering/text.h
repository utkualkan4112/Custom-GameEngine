#ifndef TEXT_H
#define TEXT_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>

#include <string>
#include <map>

#include "../memory/vertexmemory.hpp"
#include "shader.h"

typedef struct {
	unsigned int	textureId;	// texture id storing the character
	glm::ivec2		size;		// size of the character
	glm::ivec2		bearing;	// distance from origin to top left of the character
	unsigned int	advance;	// distance from origin to next origin (1/64th pixels)
} Character;

class TextRenderer {
public:
	TextRenderer();
	TextRenderer(int height);

	bool loadFonts(FT_Library& ft, std::string path);

	void render(Shader shader, std::string text, float x, float y, glm::vec2 scale, glm::vec3 color);

	void cleanup();

private:
	int height;
	

	std::map<char, Character> chars;

	ArrayObjects VAO;
};

#endif // !TEXT_H
