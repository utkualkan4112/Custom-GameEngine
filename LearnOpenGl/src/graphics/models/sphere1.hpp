#ifndef SPHERE1_HPP
#define SPEHERE1_HPP

#include "../objects/model.h"

class Sphere1 : public Model {
public:
	Sphere1(unsigned int maxNoInstances)
		: Model("sphere", maxNoInstances, NO_TEX | DYNAMIC) {}

	void init() {
		loadModel("assets/models/sphere/scene.gltf");
	}

};

#endif