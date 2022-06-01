#pragma once

#include "libs/stb_image.h"
#include "mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <map>

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
};