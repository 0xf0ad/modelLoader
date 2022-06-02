#pragma once

#include "libs/stb_image.h"
#include "bone.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <map>
#include <assert.h>

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