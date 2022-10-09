#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include <vector>

#define MAX_BONE_INFLUENCE 4

struct Vertex {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texture cordinates
	glm::vec2 TexCoords;
	// texture ID to batch textures
	unsigned char textureIndex;
	// tangent
	//glm::vec3 Tangent;
	// bitangent
	//glm::vec3 Bitangent;
	//bone id
	unsigned char boneIDs[MAX_BONE_INFLUENCE] = {0};
	//bone weight
	float weights[MAX_BONE_INFLUENCE] = {0.0f};
};

struct Texture {
	unsigned int id;
	char type[14];
	std::string path;
};

class Mesh{
public:

	//GLint defuseTexturesIDs[16] = { 0 };
	std::vector<GLint>  defuseTexturesIDs;
	std::vector<GLint>  normalTexturesIDs;
	std::vector<GLint>  heightTexturesIDs;
	std::vector<GLint>  specularTexturesIDs;

	std::vector<Vertex>  vertices;
	std::vector<GLuint>  indices;
	std::vector<Texture> textures;
	unsigned int VAO, VBO, EBO;

	Mesh(const std::vector<Vertex>&  vertices,
	     const std::vector<uint>&    indices,
	     const std::vector<Texture>& textures);
	
	Mesh(//const std::vector<Vertex>&  vertices,
	     const std::vector<uint>&    indices,
	     const std::vector<Texture>& textures);
	
	Mesh(//const std::vector<Vertex>&  vertices,
	     //const std::vector<uint>&    indices,
	     const std::vector<Texture>& textures);

	Mesh(){
			//printf("OMG i just allocated %zu bytes\n", sizeof(*this));

	};

	Mesh(const Mesh& other){
		//printf("noooo i just coppied the mesh %zu bytes\n", sizeof(Mesh));
	}

	~Mesh(){
		//printf("mesh go brrrr %zu \n", textures.size() * sizeof(Texture));
	}

	void Draw(Shader &shader);

	void setupMesh();
};
