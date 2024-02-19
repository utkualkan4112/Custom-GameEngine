#ifndef GUN_HPP
#define GUN_HPP



#include "../objects/model.h"
#include "../../io/camera.h"
#include "../../io/mouse.h"
#include "../../io/keyboard.h"

#include "../../scene.h"
#include "../../physics/environment.h"

class Gun : public Model {

public:
	glm::vec3 targetPos;
	glm::vec3 currentPos;

	float interpolationFactor = 0.0f;


	Gun(unsigned int maxNoInstances)
		: Model("m4a1", maxNoInstances, DYNAMIC | NO_TEX) {}
	
	void render(Shader shader, float dt, Scene *scene, bool shadow, bool gun) {
		glm::mat4 model = glm::mat4(1.0f);

		currentPos = instances[0]->pos;

		if (Mouse::button(GLFW_MOUSE_BUTTON_2)) {
			targetPos = scene->getActiveCamera()->cameraPos + glm::vec3(scene->getActiveCamera()->cameraFront * 0.18f) - glm::vec3(scene->getActiveCamera()->cameraUp * 0.060f);
			interpolationFactor = 0.0f;
		}
		else {
			targetPos = scene->getActiveCamera()->cameraPos + glm::vec3(scene->getActiveCamera()->cameraFront * 0.25f) - glm::vec3(scene->getActiveCamera()->cameraUp * 0.08f) + glm::vec3(scene->getActiveCamera()->cameraRight * 0.03f);
			interpolationFactor = 0.0f;
		}

		if (Keyboard::key(GLFW_KEY_W) || Keyboard::key(GLFW_KEY_S) || Keyboard::key(GLFW_KEY_A) || Keyboard::key(GLFW_KEY_D) || Keyboard::key(GLFW_KEY_SPACE) || Keyboard::key(GLFW_KEY_LEFT_SHIFT)) {
			instances[0]->pos = targetPos;
		}
		else {
			// Interpolate between the current position and the target position
			interpolationFactor += 0.05f; // Adjust this value for speed of interpolation
			interpolationFactor = std::min(interpolationFactor, 1.0f); // Clamp to 1.0 to avoid overshooting
			instances[0]->pos = instances[0]->pos + (targetPos - instances[0]->pos) * interpolationFactor;
		}
		

		float theta;

		// rotate around camera right using dot product
		theta = acos(glm::dot(Environment::worldUp, scene->getActiveCamera()->cameraFront) /
			glm::length(scene->getActiveCamera()->cameraUp) / glm::length(scene->getActiveCamera()->cameraFront));
		model = glm::rotate(model, atanf(1) * 2 - theta, scene->getActiveCamera()->cameraRight); // offset by pi/2 radians bc angle btwn camFront and gunFront

		// rotate around cameraUp using dot product
		glm::vec2 front2d = glm::vec2(scene->getActiveCamera()->cameraFront.x, scene->getActiveCamera()->cameraFront.z);
		theta = acos(glm::dot(glm::vec2(1.0f, 0.0f), front2d) / glm::length(front2d));
		model = glm::rotate(model, scene->getActiveCamera()->cameraFront.z < 0 ? theta : -theta, Environment::worldUp);
		
		instances[0]->rotationMatrix = model;

		Model::render(shader, dt, scene, shadow, true);
	}

	void init() {
		loadModel("assets/models/m4a1/scene.gltf");
	}
};


#endif // !
