#pragma once

#include <glm/glm.hpp>
#include "shader.h"

class Light{
	glm::vec3 position;
	glm::vec3 color;
	uint8_t power;


	void init(uint32_t VBO);

	void update();
};

inline void Light::init(uint32_t VBO){
	uint32_t lightVAO;
	Shader lightingShader("shaders/vertexLightS","shaders/fragmentLightS");

	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	// we only need to bind to the VBO, the container's VBO's data already contains the data.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// set the vertex attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	// don't forget to use the corresponding shader program first (to set the uniform)
	lightingShader.use();
	//lightingShader.setVec3("objectColor", color);
	lightingShader.setVec3("lightColor",  color);
}