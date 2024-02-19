#ifndef VERTEXMEMORY_HPP
#define VERTEXMEMORY_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <map>

/*
	class for buffer objects
	- VBO, EBO, etc.
*/

class BufferObjects {
public:
	// value/location
	GLuint val;
	// type of buffer (GL_ARRAY_BUFFER | GL_ELEMENT_ARRAY_BUFFER
	GLenum type;
	
	/*
		constructor
	*/

	// default and initialize with type
	BufferObjects(GLenum type = GL_ARRAY_BUFFER)
		: type(type) {}

	// generate object
	void generate() {
		glGenBuffers(1, &val);
	}

	// bind object
	void bind() {
		glBindBuffer(type, val);
	}

	// set data (glBufferData)
	template< typename T>
	void setData(GLuint noElements, T* data, GLenum usage) {
		glBufferData(type, noElements * sizeof(T), data, usage);
	}

	// update data (glBufferSubData)
	template< typename T>
	void updateData(GLintptr offset, GLuint noElements, T* data) {
		glBufferSubData(type, offset, noElements * sizeof(T), data);
	}

	// set attribute points
	template< typename T>
	void setAttPointer(GLuint idx, GLint size, GLenum type, GLuint stride, GLuint offset, GLuint divisor = 0) {
		glVertexAttribPointer(idx, size, type, GL_FALSE, stride * sizeof(T), (void*)(offset * sizeof(T)));
		glEnableVertexAttribArray(idx);
		if (divisor > 0) {
			// reset idx attribute every _divisor_ itteration (instancing)
			glVertexAttribDivisor(idx, divisor);
		}
	}

	// clear buffer objects
	void clear() {
		glBindBuffer(type, 0);
	}

	// cleanup method
	void cleanup() {
		glDeleteBuffers(1, &val);
	}

};

/*
	class for array objects 
	- VAO
*/

class ArrayObjects {
public:
	// value/location
	GLuint val;

	// map of names to buffers
	std::map<const char*, BufferObjects> buffers;

	// get buffer (override operator [])
	BufferObjects& operator[](const char* key) {
		return buffers[key];
	}

	// generate object
	void generate() {
		glGenVertexArrays(1, &val);
	}

	// bind object
	void bind() {
		glBindVertexArray(val);
	}

	// draw arrays
	void draw(GLenum mode, GLuint first, GLuint count, GLuint instancecount = 1) {
		glDrawArraysInstanced(mode, first, count, instancecount);
	}

	// draw
	void draw(GLenum mode, GLuint count, GLenum type, GLint indices, GLuint instanceCount = 1) {
		glDrawElementsInstanced(mode, count, type, (void*)indices, instanceCount);
	}

	// cleanup
	void cleanup() {
		glDeleteVertexArrays(1, &val);
		for (auto& pair : buffers) {
			pair.second.cleanup();
		}
	}

	// clear array object (bind 0)
	static void clear() {
		glBindVertexArray(0);
	}

};

#endif // !VERTEXMEMORY_HPP
