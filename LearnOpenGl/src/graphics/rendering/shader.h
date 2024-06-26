#ifndef SHADER_H
#define SHADER_H


#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <assimp/scene.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


/*
    class to represent shader program
*/

class Shader {

public:
    // program ID
	unsigned int id;

    /*
        constructors
    */

    // default
    Shader();

    // initialize with paths to vertex, fragment, and optional geometry shaders
	Shader(bool includeDefaultHeader,
        const char* vertexShaderPath,
        const char* fragmentShaderPath,
        const char* geoShaderPath = nullptr);

    /*
        process functions
    */

    // generate using vertex, frag, and optional geometry shaders
    void generate(bool includeDefaultHeader, 
        const char* vertexShaderPath, 
        const char* fragmentShaderPath, 
        const char* geoShaderPath = nullptr);

    // activate shader
	void activate();

    // cleanup
    void cleanup();

    /*
        set uniform variables
    */

    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void set3Float(const std::string& name, float v1, float v2, float v3);
    void set3Float(const std::string& name, glm::vec3 v);
    void set4Float(const std::string& name, float v1, float v2, float v3, float v4);
    void set4Float(const std::string& name, aiColor4D color);
    void set4Float(const std::string& name, glm::vec4 v);
    void setMat3(const std::string& name, glm::mat3 val);
    void setMat4(const std::string& name, glm::mat4 val);

    /*
        static
    */

    // compile shader program
    static GLuint compileShader(bool includeDefaultHeader, const char* filepath, GLuint type);

    // default directory
    static std::string defaultDirectory;

    // stream containing default header source
    static std::stringstream defaultHeaders;

    // load into default header
    static void loadIntoDefault(const char* filepath);

    // clear the default (after compiled shaders)
    static void clearDefault();

    // load string from file
    static char* loadShaderSrc(bool includeDefaultHeader, const char* filepath);

};

#endif // !SHADER_H