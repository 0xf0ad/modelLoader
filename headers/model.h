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
		unsigned int long value = 0;

		for(unsigned int i = 0; i != str.size(); i++)
			value = value * 37 + str[i];

		return value;
	}
};

class Model {
public:

	std::unordered_map<std::string, BoneInfo, stdstrHash, stdstrequal_to> boneInfoMap;

	// constructor, expects a filepath to a 3D model.
	Model(const char* path);

	/*Model(const Model& other){
		//printf("bro you coppied the model class\n");
	}*/

	~Model();

	// draws the model, and thus all its meshes
	void Draw(Shader &shader);

	//std::unordered_map<const char*, BoneInfo, strHash, strequal_to>& GetBoneInfoMap() const;
	//std::unordered_map<std::string, BoneInfo, stdstrHash, stdstrequal_to>& GetBoneInfoMap() const;
	uint8_t* GetBoneCount() const;
};
