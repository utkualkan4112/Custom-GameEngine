#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include "mesh.h"

#include "../models/box.hpp"

#include "../../physics/rigidbody.h"
#include "../../physics/collisionmodel.h"

#include "../../algorithms/bounds.h"

// model switches
#define DYNAMIC					(unsigned int)1 // 0b00000001
#define CONST_INSTANCES			(unsigned int)2 // 0b00000010
#define NO_TEX					(unsigned int)4	// 0b00000100

// forward decleration
class Scene; 

/*
	class to represent model
*/

class Model {
public:
	// id of model in scene
	std::string id;

	// list of meshes
	std::vector<Mesh> meshes;
	// pointer to the collision model
	CollisionModel* collision;
	// list of bounding regions (1 for each mesh)
	std::vector<BoundingRegion> boundingRegions;

	// list of instances
	std::vector<RigidBody*> instances;

	// maximum number of instances
	unsigned int maxNoInstances;
	// current number of instances
	unsigned int currentNoInstances;

	// combination of switches above
	unsigned int switches;

	/*
		constructor
	*/

	// initialize with parameters
	Model(std::string id, unsigned int maxNoInstances, unsigned int flags = 0);

	/*
		process functions
	*/

	// initialize method (to be overriden)
	virtual void init();

	// load model from path
	void loadModel(std::string path);

	// enable collision model
	void enableCollisionModel();

	// add a mesh to list
	void addMesh(Mesh* mesh);

	// render instance(s)
	virtual void render(Shader shader, float dt, Scene* scene, bool shadow = false, bool gun = false);

	// free up memory
	void cleanup();

	/*
		instance methods
	*/

	// generate instance with parameters
	RigidBody* generateInstances(glm::vec3 size, float mass, glm::vec3 pos, glm::vec3 rot);

	// initialize memory for instances
	void initInstances();

	// remove instance at idx
	void removeInstance(unsigned int idx);

	// remove instance with id
	void removeInstance(std::string instanceId);

	// get index of instance with id
	unsigned int getIdx(std::string id);

protected:
	// true if doesn't have textures
	bool noTex;

	// directory containing object file
	std::string directory;

	// list of loaded textures
	std::vector<Texture> textures_loaded;

	/*
		model loading functions (ASSIMP)
	*/

	// process node in object file
	void processNode(aiNode* node, const aiScene* scene);

	// process mesh in object file
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	// process custom mesh
	Mesh processMesh(BoundingRegion br,
		unsigned int noVertices, float* vertices,
		unsigned int noIndices, unsigned int* indices,
		bool calcTanVectors = true,
		unsigned int noCollisionPoints = 0, float* collisionPoints = NULL,
		unsigned int collisionFaces = 0, unsigned int* collisionIndices = NULL,
		bool pad = false);

	// load list of textures
	std::vector<Texture> loadTexture(aiMaterial* mat, aiTextureType type);

	// VBOs for model matrices
	BufferObjects modelVBO;
	BufferObjects normalModelVBO;
};

#endif // !MODEL_H
