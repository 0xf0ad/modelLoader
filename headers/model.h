#pragma once

#include <glm/glm.hpp>
#include "libs/stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <unordered_map>

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

class Model {
public:
	// constructor, expects a filepath to a 3D model.
	Model(const char* path);
	
	Model(const Model& other){
		//printf("bro you coppied the model class\n");
	}

	~Model(){
		//printf("model destructed\n");
	}

	// draws the model, and thus all its meshes
	void Draw(Shader &shader);

	std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() const;
	unsigned char* GetBoneCount() const;
};
