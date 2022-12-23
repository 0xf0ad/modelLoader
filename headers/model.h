#pragma once

#include <glm/glm.hpp>
#include "libs/stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>
#include <string.h>

#include "mesh.h"

struct BoneInfo{
	// id is index in finalBoneMatrices
	unsigned char id;
	// offset matrix transforms vertex from model space to bone space
	glm::mat4 offset;
};

struct vertexBoneData{
	unsigned char boneIDs[4];
	float weights[4];
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
	// constructor, expects a filepath to a 3D model.
	Model(const char* path);
	
	/*Model(const Model& other){
		//printf("bro you coppied the model class\n");
	}*/

	~Model(){
		//printf("model destructed\n");
	}

	// load animation if already loaded from the model class
	// @note this is a hacky way to do that
	void* loadAnim();

	// draws the model, and thus all its meshes
	void Draw(Shader &shader);

	//std::unordered_map<const char*, BoneInfo, strHash, strequal_to>& GetBoneInfoMap() const;
	std::unordered_map<std::string, BoneInfo, stdstrHash, stdstrequal_to>& GetBoneInfoMap() const;
	unsigned char* GetBoneCount() const;
};
