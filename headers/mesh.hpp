#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "libs/glad.h"
#include "shader.hpp"
#include <vector>

#define MAX_BONE_INFLUENCE 4

struct Vertex {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texCoords
	glm::vec2 TexCoords;
	// tangent
	glm::vec3 Tangent;
	// bitangent
	glm::vec3 Bitangent;
	//bone id
	int m_BoneIDs[MAX_BONE_INFLUENCE] = {0};
	//bone weight
	float m_Weights[MAX_BONE_INFLUENCE] = {0.0f};
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

class Mesh{
public:
	std::vector<Vertex>       vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture>      textures;
	unsigned int VAO, VBO, EBO;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	void Draw(Shader &shader);

	void setupMesh();
};
