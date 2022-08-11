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

class Model {
public:

	// constructor, expects a filepath to a 3D model.
	Model(const char* path);

	// draws the model, and thus all its meshes
	void Draw(Shader &shader);

	std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap();
	unsigned char* GetBoneCount();

};
