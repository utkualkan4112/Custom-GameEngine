#ifndef FRAMEMEMORY_HPP
#define FRAMEMEMORY_HPP

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <vector>

#include "../rendering/texture.h"
#include "../rendering/cubemap.h"

class RenderBufferObject {
public:
	GLuint val;

	void generate() {
		glGenRenderbuffers(1, &val);
	}

	void bind() {
		glBindRenderbuffer(GL_RENDERBUFFER, val);
	}

	void allocate(GLenum format, GLuint width, GLuint height) {
		glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
	}

	void cleanup() {
		glDeleteRenderbuffers(1, &val);
	}
};

class FrameBufferObject {

public:
	GLuint val;
	GLuint width;
	GLuint height;
	GLbitfield bitCombo;

	std::vector<GLuint> rbos;
	std::vector<Texture> textures;
	CubeMap cubemap;

	FrameBufferObject() : val(0), width(0), height(0), bitCombo(0) {}

	FrameBufferObject(GLuint width, GLuint height, GLbitfield bitCombo ) : val(0), width(width), height(height), bitCombo(bitCombo) {}

	void generate() {
		glGenFramebuffers(1, &val);
	}

	void disableColorBuffer() {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	void bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, val);
	}

	void setViewport() {
		glViewport(0, 0, width, height);
	}

	void clear() {
		glClear(bitCombo);
	}
	
	void activate() {
		setViewport();
		bind();
		clear();
	}

	void allocateAndAttachRBO(GLenum attachType, GLenum format) {
		// render buffer to store collor buffer unformated
		GLuint RBO;

		// generate
		glGenRenderbuffers(1, &RBO);

		// bind
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);

		// allocate set storage of the RBO to the color attachment / allocate memory for the RBO
		glRenderbufferStorage(GL_RENDERBUFFER, format, width, height);
		// attach attach renderbuffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachType, GL_RENDERBUFFER, RBO);

		rbos.push_back(RBO);
	}

	void allocateAndAttachTexture(GLenum attachType, GLenum format, GLenum type) {
		std::string name = "tex" + textures.size();
		Texture tex(name);

		// allocate
		tex.bind();
		tex.allocate(format, width, height, type);
		Texture::setParams(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
		

		float borderColor[] = {
			1.0f, 1.0f, 1.0f, 1.0f
		};
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		//glTextureParameterfv(tex.id, GL_TEXTURE_BORDER_COLOR, borderColor);
		
		// attach
		bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachType, GL_TEXTURE_2D, tex.id, 0);

		textures.push_back(tex);
		glBindTexture(GL_TEXTURE_2D, 0);

	}

	void attachTexture(GLenum attachType, Texture tex) {
		// attach existing texture
		bind();
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachType, GL_TEXTURE_2D, tex.id, 0);
	}

	void allocateAndAttachCubemap(GLenum attachType, GLenum format, GLenum type) {
		cubemap = CubeMap();
		cubemap.generate();

		cubemap.bind();
		cubemap.allocate(format, width, height, type);

		bind();
		glFramebufferTexture(GL_FRAMEBUFFER, attachType, cubemap.id, 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void cleanup() {
		// delete RBO's
		glDeleteRenderbuffers((GLsizei)rbos.size(), &rbos[0]);

		// delete generated texture
		for (Texture t : textures) {
			t.cleanup();
		}

		cubemap.cleanup();

		// delete FBO
		glDeleteFramebuffers(1, &val);
	}
};
#endif // !FRAMEMEMORY_HPP
