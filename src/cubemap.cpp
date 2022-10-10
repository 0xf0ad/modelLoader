#include "../headers/cubemap.h"

static unsigned int cubeMapVAO, cubeMapVBO, cubemapTexture;

unsigned int loadCubemap(const char** faces){

	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	stbi_set_flip_vertically_on_load(false);

	int width, height, nrChannels;
	for (unsigned char i = 0; i != 6; i++){
		unsigned char *data = stbi_load(faces[i], &width, &height, &nrChannels, 0);

		if (data)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else
			printf("Cubemap texture failed to load at path: %s\n", faces[i]);

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

void initCubeMap(){
	
	float skyboxVertices[] = {
		-1.0f, -1.0f,  1.0f,//        7--------6
		 1.0f, -1.0f,  1.0f,//       /|       /|
		 1.0f, -1.0f, -1.0f,//      4--------5 |
		-1.0f, -1.0f, -1.0f,//      | |      | |
		-1.0f,  1.0f,  1.0f,//      | 3------|-2
		 1.0f,  1.0f,  1.0f,//      |/       |/
		 1.0f,  1.0f, -1.0f,//      0--------1
		-1.0f,  1.0f, -1.0f
	};

	const char* faces[] = { 
		"textures/skybox/right.jpg",
		"textures/skybox/left.jpg",
		"textures/skybox/top.jpg",
		"textures/skybox/bottom.jpg",
		"textures/skybox/front.jpg",
		"textures/skybox/back.jpg"
	};

	// skybox VAO
	glGenVertexArrays(1, &cubeMapVAO);
	glGenBuffers(1, &cubeMapVBO);
	
	glBindVertexArray(cubeMapVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeMapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	cubemapTexture = loadCubemap(faces);
}

void drawCubeMap(){

	unsigned int skyboxIndices[] = {
		// Right
		1, 6, 2,
		6, 1, 5,
		// Left
		0, 7, 4,
		7, 0, 3,
		// Top
		4, 6, 5,
		6, 4, 7,
		// Bottom
		0, 2, 3,
		2, 0, 1,
		// Back
		0, 5, 1,
		5, 0, 4,
		// Front
		3, 6, 7,
		6, 3, 2
	};

	// draw skybox as last
	glDepthFunc(GL_LEQUAL);
	// skybox cube
	glBindVertexArray(cubeMapVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawElements(GL_TRIANGLES, (sizeof(skyboxIndices) / sizeof(skyboxIndices[0])), GL_UNSIGNED_INT, skyboxIndices);
	glBindVertexArray(0);

	glDepthFunc(GL_LESS); // set depth function back to default
}
