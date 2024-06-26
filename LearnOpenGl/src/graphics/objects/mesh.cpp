#include "mesh.h"

// generate list of vertices
std::vector<Vertex> Vertex::genList(float* vertices, int noVertices) {
	std::vector<Vertex> ret(noVertices);

	int stride = 8;

	for (int i = 0; i < noVertices; i++) {
		ret[i].pos = glm::vec3(
			vertices[i * stride + 0],
			vertices[i * stride + 1],
			vertices[i * stride + 2]
		);

		ret[i].normal = glm::vec3(
			vertices[i * stride + 3],
			vertices[i * stride + 4],
			vertices[i * stride + 5]
		);

		ret[i].texCoord = glm::vec2(
			vertices[i * stride + 6],
			vertices[i * stride + 7]
		);
	}
	return ret;
}

// calculate tangent vectors for each face
void Vertex::calcTanVectors(std::vector<Vertex>& list, std::vector<unsigned int>& indices)
{
	unsigned char* counts = (unsigned char*)malloc(list.size() * sizeof(unsigned char));
	memset(counts, 0, (int)list.size());

	// itterate trough indices and calculate vectors for each face 
	for (unsigned int i = 0, len = (unsigned int)indices.size(); i < len; i += 3) {
		// 3 vertices corespounds to the face 
		Vertex v1 = list[indices[i + 0]];
		Vertex v2 = list[indices[i + 1]];
		Vertex v3 = list[indices[i + 2]];

		// calcualte edges
		glm::vec3 edge1 = v2.pos - v1.pos;
		glm::vec3 edge2 = v3.pos - v1.pos;

		// calculate uv's
		glm::vec2 deltaUV1 = v2.texCoord - v1.texCoord;
		glm::vec2 deltaUV2 = v3.texCoord - v1.texCoord;

		// use inverse of the UV matrix to determine tangent vector
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

		glm::vec3 tangent = {
			f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
			f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
			f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)
		};

		// avarage in the new tangent vector
		list[indices[i + 0]].tangent += tangent;
		list[indices[i + 1]].tangent += tangent;
		list[indices[i + 2]].tangent += tangent;

		counts[indices[i + 0]]++;
		counts[indices[i + 1]]++;
		counts[indices[i + 2]]++;
	}

	for (unsigned int i = 0, len = (unsigned int)list.size(); i < len; i++) {
		list[indices[i]].tangent /= (float)counts[indices[i]];
	}
	
}

/*
	constructors
*/

// default
Mesh::Mesh()
	: collision(NULL) {}

// intialize with a bounding region
Mesh::Mesh(BoundingRegion br)
	: br(br), collision(NULL)
{
}

// initialize as textured object
Mesh::Mesh(BoundingRegion br, std::vector<Texture> textures)
	: Mesh(br) {
	setupTextures(textures);
}

// initialize as material object
Mesh::Mesh(BoundingRegion br, aiColor4D diff, aiColor4D spec)
	: Mesh(br) {
	setupColors(diff, spec);
}

// initialize with a material
Mesh::Mesh(BoundingRegion br, Material m)
	: Mesh(br)
{
	setupMaterial(m);
}

// load vertex and index data
void Mesh::loadData(std::vector<Vertex> _vertices, std::vector<unsigned int> _indices, bool pad)
{
	this->vertices = _vertices;
	this->indices = _indices;

	// bind VAO
	VAO.generate();
	VAO.bind();

	// generate/set EBO
	VAO["EBO"] = BufferObjects(GL_ELEMENT_ARRAY_BUFFER);
	VAO["EBO"].generate();
	VAO["EBO"].bind();
	VAO["EBO"].setData<GLuint>((GLuint)this->indices.size(), &this->indices[0], GL_STATIC_DRAW);

	// load data into vertex buffers
	VAO["VBO"] = BufferObjects(GL_ARRAY_BUFFER);
	VAO["VBO"].generate();
	VAO["VBO"].bind();

	unsigned int size = (unsigned int)this->vertices.size();
	if (pad && size) {
		size++;
	}

	VAO["VBO"].setData<Vertex>(size, &this->vertices[0], GL_STATIC_DRAW);

	// set the vertex attribute pointers
	VAO["VBO"].bind();
	// vertex Positions
	VAO["VBO"].setAttPointer<GLfloat>(0, 3, GL_FLOAT, 11, 0);
	// normal ray
	VAO["VBO"].setAttPointer<GLfloat>(1, 3, GL_FLOAT, 11, 3);
	// vertex texture coords
	VAO["VBO"].setAttPointer<GLfloat>(2, 2, GL_FLOAT, 11, 6);
	// tangent vector
	VAO["VBO"].setAttPointer<GLfloat>(3, 3, GL_FLOAT, 11, 8);

	VAO["VBO"].clear();

	ArrayObjects::clear();
}

// setup collision mesh
void Mesh::loadCollisionMesh(unsigned int noPoints, float* coordinates, unsigned int noFaces, unsigned int* indices)
{
	this->collision = new CollisionMesh(noPoints, coordinates, noFaces, indices);
	this->br = this->collision->br;
}

// setup textures
void Mesh::setupTextures(std::vector<Texture> textures)
{
	this->noTex = false;
	this->textures.insert(this->textures.end(), textures.begin(), textures.end());
}

// setup material colors
void Mesh::setupColors(aiColor4D diff, aiColor4D spec)
{
	this->noTex = true;
	this->diffuse = diff;
	this->specular = spec;
}

// set material structure
void Mesh::setupMaterial(Material mat)
{
	this->noTex = true;
	this->diffuse = { mat.diffuse.r, mat.diffuse.g, mat.diffuse.b, 1.0f };
	this->specular = { mat.specular.r, mat.specular.g, mat.specular.b, 1.0f };
}

// render number of instances using shader
void Mesh::render(Shader shader, unsigned int noInstances)
{
	shader.setBool("noNormalMap", true);
	shader.setBool("noDiffuse", true);
	shader.setBool("noSpec", true);

	// materials
	shader.set4Float("material.diffuse", diffuse);
	shader.set4Float("material.specular", specular);
	
	

	// textures
	unsigned int diffuseIdx = 0;
	unsigned int normalIdx = 0;
	unsigned int specularIdx = 0;


	for (unsigned int i = 0; i < textures.size(); i++) {

		// retrive tex info
		std::string name;
		switch (textures[i].type) {
		case aiTextureType_DIFFUSE:
			name = "diffuse" + std::to_string(diffuseIdx++);
			shader.setBool("noDiffuse", false);
			break;
		case aiTextureType_NORMALS:
			name = "normal" + std::to_string(normalIdx++);
			shader.setBool("noNormalMap", false);
			break;
		case aiTextureType_SPECULAR:
			name = "specular" + std::to_string(specularIdx++);
			shader.setBool("noSpec", false);
			break;
		default:
			name = textures[i].name;
			break;

		}

		// bind texture
		glBindTextureUnit(i + 1, textures[i].id);
		// shader
		shader.setInt(name, i + 1);
		
		
	}


	VAO.bind();
	VAO.draw(GL_TRIANGLES, (GLuint)indices.size(), GL_UNSIGNED_INT, 0, noInstances);
	ArrayObjects::clear();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

// free up memory
void Mesh::cleanup()
{
	VAO.cleanup();

	for (Texture t : textures) {
		t.cleanup();
	}
}

// setup data with buffers
void Mesh::setup() {
	// create buffers/arrays

	// bind VAO
	VAO.generate();
	VAO.bind();

	// generate/set EBO
	VAO["EBO"] = BufferObjects(GL_ELEMENT_ARRAY_BUFFER);
	VAO["EBO"].generate();
	VAO["EBO"].bind();
	VAO["EBO"].setData<GLuint>((GLuint)indices.size(), &indices[0], GL_STATIC_DRAW);

	// generate/set VBO
	VAO["VBO"] = BufferObjects(GL_ARRAY_BUFFER);
	VAO["VBO"].generate();
	VAO["VBO"].bind();
	VAO["VBO"].setData<Vertex>((GLuint)vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// set vertex attrib pointers
	// vertex positions
	VAO["VBO"].setAttPointer<GLfloat>(0, 3, GL_FLOAT, 8, 0);
	// normal ray
	VAO["VBO"].setAttPointer<GLfloat>(1, 3, GL_FLOAT, 8, 3);
	// texture coordinates
	VAO["VBO"].setAttPointer<GLfloat>(2, 2, GL_FLOAT, 8, 6);

	VAO["VBO"].clear();

	ArrayObjects::clear();
}