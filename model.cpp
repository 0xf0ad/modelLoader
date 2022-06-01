#include "headers/model.hpp"

// model data
std::vector<Texture> textures_loaded;
std::vector<Mesh>    meshes;
std::string          directory;

//bone data
std::vector<vertexBoneData> vertexToBone;
std::vector<int>            meshBaseVertex;
std::map<std::string, uint> boneNameToIndexMap;

int getBoneID(const aiBone* bone){
    
	int boneID = 0;
    std::string boneName(bone->mName.C_Str());

    if (boneNameToIndexMap.find(boneName) == boneNameToIndexMap.end()) {
        // Allocate an index for a new bone
        boneID = (int)boneNameToIndexMap.size();
        boneNameToIndexMap[boneName] = boneID;
    }else{
        boneID = boneNameToIndexMap[boneName];
    }

    return boneID;
}

void Model::Draw(Shader &shader){
    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shader);
}

void Model::loadModel(std::string path){

    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);	
	
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << '\n';
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
	parseMeshes(scene);
}

void Model::parseMeshes(const aiScene *scene){
	std::cout<<"parsing "<<scene->mNumMeshes<<" meshes\n\n";

	int Tvertices = 0, Tindices = 0, Tbones = 0;

	meshBaseVertex.resize(scene->mNumMeshes);

	for (unsigned int k = 0; k < scene->mNumMeshes; k++){
		const aiMesh* mesh = scene->mMeshes[k];
		int Nvertices = mesh->mNumVertices;
		int Nindices = mesh->mNumFaces * 3;
		int Nbones = mesh->mNumBones;
		meshBaseVertex[k] = Tvertices;
		
		std::cout<<"Mesh "<<k<<' '<<mesh->mName.C_Str()<<" : vertices "<<Nvertices<<" indices "<<Nindices<<" bones "<<Nbones<<'\n';
		
		Tvertices += Nvertices;
		Tindices += Nindices;
		Tbones += Nbones;
		vertexToBone.resize(Tvertices);

		if(mesh->HasBones()){
			for(unsigned int i = 0; i < mesh->mNumBones; i++){
				std::cout<<"\tBoner : "<<i<<' '<<mesh->mBones[i]->mName.C_Str()<<" ,vertices effected by that bone : "<<mesh->mBones[i]->mNumWeights<<'\n';
				int boneID = getBoneID(mesh->mBones[i]);
				std::cout<<"bone id : "<<boneID<<'\n';
				
				for(unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++){
					if(!j){
						std::cout<<'\n';
					}
					unsigned int globalVertexID = meshBaseVertex[k] + mesh->mBones[i]->mWeights[j].mVertexId;
					std::cout<<'\t'<<j<<" : vertex id "<<mesh->mBones[i]->mWeights[j].mVertexId<<" weight : "<<mesh->mBones[i]->mWeights[j].mWeight<<'\n';

					assert(globalVertexID < vertexToBone.size());
					vertexToBone[globalVertexID].addBoneData(boneID, mesh->mBones[i]->mWeights[j].mWeight);
				}
				std::cout<<'\n';
			}
		}
	}
	std::cout<<"\nTotal vertices : "<<Tvertices<<" Total indices : "<<Tindices<<" Total bones : "<<Tbones<<'\n';
}

void Model::processNode(aiNode *node, const aiScene *scene){
	// process all the node's meshes (if any)
	for(unsigned int i = 0; i < node->mNumMeshes; i++){
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
		meshes.push_back(processMesh(mesh, scene));			
	}
	// then do the same for each of its children
	for(unsigned int i = 0; i < node->mNumChildren; i++)
		processNode(node->mChildren[i], scene);
}


Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene){
    // data to fill
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;

	// walk through each of the mesh's vertices
	for(unsigned int i = 0; i < mesh->mNumVertices; i++){
		Vertex vertex;

		//SetVertexBoneDataToDefault(vertex);

		vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
		vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);
		glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        // normals
        if (mesh->HasNormals()){
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // texture coordinates
            if(mesh->mTextureCoords[0]){ // does the mesh contain texture coordinates?
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
		//ExtractBoneWeightForVertices(vertices,mesh,scene);

        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for(unsigned int i = 0; i < mesh->mNumFaces; i++){
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);        
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    
    // return a mesh object created from the extracted mesh data
    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName){
    std::vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++){
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded.size(); j++){
            if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0){
                textures.push_back(textures_loaded[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
                }
            }if(!skip){   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
        }
    }
    return textures;
}

unsigned int Model::TextureFromFile(const char *path, const std::string &directory){
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data){
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else{
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}
