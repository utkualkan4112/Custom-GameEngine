#ifndef BRICKWALL_HPP
#define BRICKWALL_HPP

#include "plane.hpp"

class BrickWall : public Plane {
public:
	void init() {
		std::vector<Texture> textures = {
			Texture::loadFromFile("assets/textures", "brickwall_diff.jpeg", aiTextureType_DIFFUSE),
			Texture::loadFromFile("assets/textures", "birckwall_normal.jpg", aiTextureType_NORMALS),
			Texture::loadFromFile("assets/textures", "brickwall_spec.jpeg", aiTextureType_SPECULAR)
		};

		Plane::init(textures);
	}
	/*
	void render(Shader shader, float dt, Scene* scene, glm::mat4 model = glm::mat4(1.0f)) {
		model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), { 1.0f, 0.0f, 0.0f });
		Model::render(shader, dt, scene, model);
	}*/
	
};

#endif // !BRICKWALL_HPP
