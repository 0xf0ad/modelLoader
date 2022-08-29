#include <assert.h>
#include <glm/fwd.hpp>
#include <stdio.h>
#include "../headers/model.h"

// model data
std::vector<Texture> textures_loaded;
Mesh                 BIGMesh;
unsigned char        m_BoneCounter = 1;
unsigned int prevMeshNumVertices = 0;
std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false){

	std::string filename = directory + '/' + path;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data){
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;
		else
			printf("failed to load channels on texture: %s\n", filename.c_str());

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}else{
		printf("Texture failed to load at path: %s\n", filename.c_str());
	}
	
	stbi_image_free(data);
	return textureID;
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, const char* typeName, const char* dir){
	std::vector<Texture> textures;
	for(unsigned int i = 0; i != mat->GetTextureCount(type); i++){
		aiString str;
		mat->GetTexture(type, i, &str);
		
		// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
		stbi_set_flip_vertically_on_load(true);

		// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		bool skip = false;
		for(unsigned int j = 0; j != textures_loaded.size(); j++){
			if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0){
				textures.push_back(textures_loaded[j]);
				skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
				break;
			}
		}
		if(!skip){   // if texture hasn't been loaded already, load it
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), dir);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
		}
	}
	return textures;
}

Mesh processMesh(aiMesh* mesh, const aiScene* scene , const char* dir){
	std::vector<Vertex>  vertices;
	std::vector<uint>    indices;
	std::vector<Texture> textures;

	unsigned int materialIndex = mesh->mMaterialIndex;
	// process meshes
	// --------------
	for(unsigned int i = 0; i != mesh->mNumVertices; i++){
		Vertex vertex;
		for(int i = 0; i != MAX_BONE_INFLUENCE; i++){
			vertex.boneIDs[i] = 0;
			vertex.weights[i] = 0.0f;
		}

		//process mesh positions if it has any
		if(mesh->HasPositions()){
			vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
		}
		// process normals if it has any
		if(mesh->HasNormals()){
			vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);
		}
		// load textures if it has any
		if(mesh->mTextureCoords[0]){
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
			vertex.textureIndex = materialIndex;
		}else{
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		}
		vertices.push_back(vertex);
	}

	// process faces
	// -------------
	for(unsigned int i = 0; i != mesh->mNumFaces; i++){
		aiFace face = mesh->mFaces[i];
		for(unsigned int j = 0; j != face.mNumIndices; j++)
			indices.push_back(face.mIndices[j] + prevMeshNumVertices);
	}

	prevMeshNumVertices += mesh->mNumVertices;

	// process bones
	// -------------
	if(mesh->HasBones()){
		for(uint boneIndex = 0; boneIndex != mesh->mNumBones; boneIndex++){
			unsigned char boneID = 0;
			std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
			if(m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()){
				BoneInfo newBoneInfo;
				newBoneInfo.id = m_BoneCounter;
				newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
				m_BoneInfoMap[boneName] = newBoneInfo;
				boneID = m_BoneCounter;
				m_BoneCounter++;
			}else{
				boneID = m_BoneInfoMap[boneName].id;
			}
			assert(boneID);

			for(uint weightIndex = 0; weightIndex != mesh->mBones[boneIndex]->mNumWeights; weightIndex++){
				int vertexId = mesh->mBones[boneIndex]->mWeights[weightIndex].mVertexId;
				assert(vertexId <= (int)vertices.size());
			
				for(unsigned char i = 0; i != MAX_BONE_INFLUENCE; i++){
					if(!vertices[vertexId].boneIDs[i]){
						vertices[vertexId].weights[i] = mesh->mBones[boneIndex]->mWeights[weightIndex].mWeight;
						vertices[vertexId].boneIDs[i] = boneID;
						break;
					}
				}
			}
		}
	}

	// assigne textures
	// ----------------
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", dir);
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", dir);
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", dir);
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height", dir);
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());


	return Mesh(vertices, indices, textures);
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void processNode(aiNode *node, const aiScene *scene,std::vector<Mesh>& meshes, const char* dir){
	// process all the node's meshes (if any)
	for(unsigned int i = 0; i != node->mNumMeshes; i++)
		meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene, dir));
	for(unsigned int i = 0; i != node->mNumChildren; i++)	// then do the same for each of its children
		processNode(node->mChildren[i], scene, meshes, dir);
}

// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void loadModel(const std::string& path){
	std::vector<Mesh>    meshes;
	// read file via ASSIMP
	Assimp::Importer import;
	const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
	//check for importing errors
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
		printf("ERROR::ASSIMP::%s\n", import.GetErrorString());
		return;
	}
	//const char* directory = path.substr(0, path.find_last_of('/')).c_str();
	std::string directory = path.substr(0, path.find_last_of('/'));
	processNode(scene->mRootNode, scene, meshes, directory.c_str());

	for (unsigned int i = 0; i!= meshes.size(); i++){
		for (unsigned int j = 0; j != meshes[i].vertices.size(); j++)
			BIGMesh.vertices.push_back(meshes[i].vertices[j]);
		for (unsigned int j = 0; j != meshes[i].indices.size(); j++)
			BIGMesh.indices.push_back(meshes[i].indices[j]);
		for (unsigned int j = 0; j != meshes[i].textures.size(); j++)
			BIGMesh.textures.push_back(meshes[i].textures[j]);
	}

	BIGMesh.setupMesh();

	/*unsigned int VAO, VBO, EBO;
	
	glGenVertexArrays(true, &VAO);
	glGenBuffers(true, &VBO);
	glGenBuffers(true, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, BIGMesh.vertices.size() * sizeof(BIGMesh.vertices[0]), &BIGMesh.vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, BIGMesh.indices.size() * sizeof(BIGMesh.indices[0]), &BIGMesh.indices[0], GL_STATIC_DRAW);

	unsigned int offset = 0;
	
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(BIGMesh.vertices), &BIGMesh.vertices);
	offset += sizeof(BIGMesh.vertices);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(BIGMesh.indices),  &BIGMesh.indices);
	offset += sizeof(BIGMesh.indices);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(BIGMesh.textures), &BIGMesh.textures);
	offset = 0;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)offset);
	offset += sizeof(BIGMesh.vertices);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)offset);
	offset += sizeof(BIGMesh.indices);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)offset);

	glBindVertexArray(0);*/
}

Model::Model(const char* path){
	loadModel(path);
}

// draws the model, and thus all its meshes
void Model::Draw(Shader &shader){
	BIGMesh.Draw(shader);
}

std::unordered_map<std::string, BoneInfo>& Model::GetBoneInfoMap() const { return m_BoneInfoMap; }
unsigned char* Model::GetBoneCount() const { return &m_BoneCounter; }
