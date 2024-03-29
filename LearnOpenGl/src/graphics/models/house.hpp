#ifndef HOUSE_HPP
#define HOUSE_HPP

#include "../objects/model.h"

class House : public Model {
public:
    House() : Model("house", 2, CONST_INSTANCES) {}

    void init() {
        loadModel("assets/models/CV/ev/utku_street.gltf");

        int noVertices = 4;

        float quadVertices[] = {
            // position			normal				texcoord
             0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,	1.0f, 1.0f, // top right
            -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,	0.0f, 1.0f, // top left
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,	0.0f, 0.0f, // bottom left
             0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,	1.0f, 0.0f  // bottom right
        };
        float collisionVertices[] = {
            // position			
             0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f
        };

        unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3
        };

        //BoundingRegion br(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.5f, 0.5f, 0.0f));
        BoundingRegion br(glm::vec3(0.0f), 1 / sqrt(2.0f));

    }

};

#endif // !HOUSE_HPP
