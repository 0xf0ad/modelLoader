#pragma once

#include "shader.h"

class FrameBuffer{
public:

	float quadVertices[16] = {
	// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		 1.0f,  1.0f,  1.0f, 1.0f
	};

	unsigned int quadIndices[6] = {
		0, 1, 2,
		0, 2, 3
	};


	unsigned int FBO;
	unsigned int RBO;
	unsigned int framebufferTexture;
	unsigned int quadVAO, quadVBO;

	Shader* FBOShader;

	FrameBuffer(unsigned int weight, unsigned int height){

		Shader shader("shaders/frmebuffervs","shaders/frmebufferfs");
		this->FBOShader = &shader;

		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);

		glGenTextures(1, &framebufferTexture);
		glBindTexture(GL_TEXTURE_2D, framebufferTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, weight, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, weight, height); // use a single renderbuffer object for both a depth AND stencil buffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);


		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		// error checking
		GLenum framebufferstat = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (framebufferstat != GL_FRAMEBUFFER_COMPLETE){
			printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!(%i)\n", framebufferstat);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		FBOShader->use();
		FBOShader->setInt("screenTexture", 0);
	}

	void firstPass(){
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	}

	void secondPass(){
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		FBOShader->use();
		glBindVertexArray(quadVAO);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, framebufferTexture);
		glDrawElements(GL_TRIANGLES, sizeof(quadIndices), GL_UNSIGNED_INT, quadIndices);
		glBindVertexArray(0);
	}

	void clear(){
		glDeleteVertexArrays(1, &quadVAO);
		glDeleteBuffers(1, &quadVBO);
		glDeleteFramebuffers(1, &FBO);
	}
};