#pragma once

#include <assimp/material.h>

#include <stdlib.h>
#include <stdint.h>
#include "shader.h"
#include <vector>

#define MAX_BONE_INFLUENCE 4

#define TANGENT                            false
#define BITANGENT                          false
#define IWANTMYIMPLEMENTATIONOFTEXTUREENUM false

#ifndef _GNU_SOURCE
inline char* strndup(const char* src, size_t size){
	size_t len = strnlen(src, size);
	len = len < size ? len : size;
	char* dst = (char*)malloc(len + 1);

	if (!dst)
		return NULL;

	memcpy(dst, src, len);
	dst[len] = '\0';
	return dst;
}
#endif

struct Vertex {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texture cordinates
	glm::vec2 TexCoords;
	// texture ID to batch textures
	uint8_t textureIndex;

#if TANGENT 
	// tangent
	glm::vec3 Tangent;
#endif /* TANGENT */
#if BITANGENT
	// bitangent
	glm::vec3 Bitangent;
#endif /* BITANGENT */

	//bone id
	uint8_t boneIDs[MAX_BONE_INFLUENCE] = {0};
	//bone weight
	float weights[MAX_BONE_INFLUENCE] = {0.0f};
};

#if IWANTMYIMPLEMENTATIONOFTEXTUREENUM
enum textureType{
	diffuse_texture,
	specular_texture,
	normal_texture,
	height_texture
};
#endif /* IWANTMYIMPLEMENTATIONOFTEXTUREENUM */ 

struct Texture {
	GLuint id;
	aiTextureType type;
	std::string path;
};

class Mesh{
public:

	std::vector<GLint>  defuseTexturesIDs;
	std::vector<GLint>  normalTexturesIDs;
	std::vector<GLint>  heightTexturesIDs;
	std::vector<GLint>  specularTexturesIDs;

	std::vector<Vertex>  vertices;
	std::vector<GLuint>  indices;
	std::vector<Texture> textures;
	uint32_t VAO, VBO, EBO;

	Mesh(const std::vector<Vertex>&   vertices,
	     const std::vector<uint32_t>& indices,
	     const std::vector<Texture>&  textures);
	
	Mesh(const std::vector<uint32_t>& indices,
	     const std::vector<Texture>&  textures);
	
	Mesh(const std::vector<Texture>&  textures);

	Mesh(){
			//printf("OMG i just allocated %zu bytes\n", sizeof(*this));
	};

	/*Mesh(const Mesh& other){
		//printf("noooo i just coppied the mesh %zu bytes\n", sizeof(Mesh));
	}*/

	~Mesh(){
		//printf("mesh go brrrr %zu \n", textures.size() * sizeof(Texture));
	}

	void Draw(Shader &shader);

	void setupMesh();
};
