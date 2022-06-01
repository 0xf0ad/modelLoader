#pragma once

#include "libs/glad.h"
#include "libs/stb_image.h"
#include "shader.h"
#include "mesh.h"

#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

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