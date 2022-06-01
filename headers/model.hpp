#pragma once

#include "libs/stb_image.h"
#include "mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <map>
#include <assert.h>

#define MAX_NUMBER_BONES_PER_VERTEX 4 

struct vertexBoneData{
	unsigned int boneIDs[MAX_NUMBER_BONES_PER_VERTEX] = {0};
	float Weights[MAX_NUMBER_BONES_PER_VERTEX] = {0.0f};

	vertexBoneData(){
	}

	void addBoneData(uint BoneID, float Weight){
		for(unsigned int i = 0; i < (sizeof(boneIDs)/sizeof(uint)); i++){
			if(!Weights[i]){
				boneIDs[i] = BoneID;
				Weights[i] = Weight;
				std::cout<<"bone "<<BoneID<<" weight "<<Weight<<" index "<<i<<'\n';
				return;
			}
		}
		//we should never get here
		assert(false);
	}
};

class Model {
public:

	bool gammaCorrection;

	Model(std::string const &path, bool gamma = false) : gammaCorrection(gamma){
		loadModel(path);
	}

	void Draw(Shader &shader);	

	void loadModel(std::string path);
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
	unsigned int TextureFromFile(const char *path, const std::string &directory);
	void parseMeshes(const aiScene *scene);
};