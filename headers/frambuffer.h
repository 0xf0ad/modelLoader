#pragma once

#include "shader.h"

class FrameBuffer{
public:

	float mQuadVertices[16] = {
	// positions	// texCoords
	-1.0f,  1.0f,	0.0f, 1.0f,
	-1.0f, -1.0f,	0.0f, 0.0f,
	 1.0f, -1.0f,	1.0f, 0.0f,
	 1.0f,  1.0f,	1.0f, 1.0f
	};

	unsigned int mQuadIndices[6] = {
		0, 1, 2,
		0, 2, 3
	};


	unsigned int mFBO;
	unsigned int mRBO;
	unsigned int mFramebufferTexture;
	unsigned int mQuadVAO, mQuadVBO;

	Shader* mFBOShader;

	FrameBuffer(unsigned int weight, unsigned int height){

		Shader shader("shaders/frmebuffervs","shaders/frmebufferfs");
		mFBOShader = &shader;

		glGenFramebuffers(1, &mFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

		glGenTextures(1, &mFramebufferTexture);
		glBindTexture(GL_TEXTURE_2D, mFramebufferTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, weight, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mFramebufferTexture, 0);

		glGenRenderbuffers(1, &mRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, weight, height); // use a single renderbuffer object for both a depth AND stencil buffer.
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);


		glGenVertexArrays(1, &mQuadVAO);
		glGenBuffers(1, &mQuadVBO);
		glBindVertexArray(mQuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, mQuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(mQuadVertices), &mQuadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		// error checking
		GLenum framebufferstat = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (framebufferstat != GL_FRAMEBUFFER_COMPLETE){
			fprintf(stderr, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!(%i)\n", framebufferstat);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		mFBOShader->use();
		mFBOShader->setInt("screenTexture", 0);
	}

	void firstPass(){
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	}

	void secondPass(){
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		mFBOShader->use();
		glBindVertexArray(mQuadVAO);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, mFramebufferTexture);
		glDrawElements(GL_TRIANGLES, sizeof(mQuadIndices), GL_UNSIGNED_INT, mQuadIndices);
		glBindVertexArray(0);
	}

	void clear(){
		glDeleteVertexArrays(1, &mQuadVAO);
		glDeleteBuffers(1, &mQuadVBO);
		glDeleteFramebuffers(1, &mFBO);
	}
};