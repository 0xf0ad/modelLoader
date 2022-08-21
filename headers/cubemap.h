#pragma once

#include "libs/glad.h"
#include "libs/stb_image.h"
#include "shader.h"
#include "camera.h"
#include <glm/fwd.hpp>
#include <stdio.h>
#include <vector>
#include <string>

unsigned int loadCubemap(std::vector<std::string> faces){

	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	stbi_set_flip_vertically_on_load(false);

	int width, height, nrChannels;
	for (unsigned char i = 0; i < 6; i++){
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

		if (data)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else
			printf("Cubemap texture failed to load at path: %s\n", faces[i].c_str());

		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	return textureID;
}

class CubeMap{
public:

	Shader* skyboxShader;
	unsigned int skyboxVAO, skyboxVBO;
	unsigned int cubemapTexture;

	CubeMap(){

		float skyboxVertices[] = {
			// positions
			/*
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f */
		-1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
		};

		unsigned char skyboxIndices[] = {
			// indices
			0, 2, 1,
			0, 2, 3,
			0, 4, 1,
			0, 4, 5,
			6, 4, 5,
			6, 4, 7,
			6, 2, 7,
			6, 2, 3,
			1, 2, 4,
			2, 4, 7,
			0, 6, 3,
			0, 6, 5 
		};

		std::vector<std::string> faces = { 
			"./textures/skybox/right.jpg",
			"./textures/skybox/left.jpg",
			"./textures/skybox/top.jpg",
			"./textures/skybox/bottom.jpg",
			"./textures/skybox/front.jpg",
			"./textures/skybox/back.jpg"
		};

		Shader shader("shaders/skyboxvs","shaders/skyboxfs");
		skyboxShader = &shader;

		// skybox VAO
		glGenVertexArrays(1, &skyboxVAO);
		glGenBuffers(1, &skyboxVBO);
		glBindVertexArray(skyboxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		cubemapTexture = loadCubemap(faces);
	}

	void Draw(glm::mat4* view, glm::mat4* projection){
		// draw skybox as last
		glDepthFunc(GL_LEQUAL);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		skyboxShader->use();
		printf("3\n");
		skyboxShader->setMat4("view", *view);
		printf("4\n");
		skyboxShader->setMat4("projection", *projection);
		printf("5\n");

		glDepthFunc(GL_TRUE); // set depth function back to default
	}

	~CubeMap(){
		glDeleteVertexArrays(1, &skyboxVAO);
		glDeleteBuffers(1, &skyboxVBO);
	}
};