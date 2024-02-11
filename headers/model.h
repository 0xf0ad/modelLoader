#pragma once

#include <string.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>

#include "libs/stbi/stb_image.h"
#include "mesh.h"

struct BoneInfo{
	// id is index in finalBoneMatrices
	uint8_t id;
	// offset matrix transforms vertex from model space to bone space
	glm::mat4 offset;
};

struct vertexBoneData{

	union{
		uint8_t  boneIDs[4];
		uint32_t packed_IDs = 0;
	};

	float weights[4] = { 0.0f };
};

struct stdstrequal_to{
	bool operator()(const std::string& s1, const std::string& s2) const{
		return (!strcmp(s1.c_str(), s2.c_str()));
	}
};

struct stdstrHash{
	int operator()(const std::string& str) const{
		uint64_t value = 0;

		for(uint32_t i = 0; i != str.size(); i++)
			value = value * 37 + str[i];

		return value;
	}
};

class Model {
public:

	uint32_t VAO, VBO, EBO;
	uint32_t numMeshs = 0, numIndices = 0, numVertices = 0;
	uint8_t  m_BoneCounter       = 1;
	uint32_t prevMeshNumVertices = 0;
	uint32_t prevMeshNumIndices  = 0;
	bool hasAnimation = false;

	std::unordered_map<std::string, BoneInfo, stdstrHash, stdstrequal_to> boneInfoMap;
	std::vector<glm::mat4> offsets;

	// constructor, expects a filepath to a 3D model.
	Model(const char* path);

	/*Model(const Model& other){
		//printf("bro you coppied the model class\n");
	}*/

	~Model();

	// draws the model, and thus all its meshes
	void Draw(Shader& shader);
	void Draw(Shader& shader, uint32_t count);

	//std::unordered_map<const char*, BoneInfo, strHash, strequal_to>& GetBoneInfoMap() const;
	//std::unordered_map<std::string, BoneInfo, stdstrHash, stdstrequal_to>& GetBoneInfoMap() const;
};
