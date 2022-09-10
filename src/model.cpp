#include <assert.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <stdio.h>
#include <vector>
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
			glBufferSubData(GL_ARRAY_BUFFER, (i + prevMeshNumVertices) * sizeof(Vertex), sizeof(vertex.Position), &vertex.Position);	
		}
		// process normals if it has any
		if(mesh->HasNormals()){
			vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);
			glBufferSubData(GL_ARRAY_BUFFER, ((i + prevMeshNumVertices) * sizeof(Vertex)) + offsetof(Vertex, Normal), sizeof(vertex.Normal), &vertex.Normal);
		}
		// load textures if it has any
		if(mesh->mTextureCoords[0]){
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
			vertex.textureIndex = materialIndex;
			glBufferSubData(GL_ARRAY_BUFFER, ((i + prevMeshNumVertices) * sizeof(Vertex)) + offsetof(Vertex, TexCoords)   , sizeof(vertex.TexCoords), &vertex.TexCoords);
			glBufferSubData(GL_ARRAY_BUFFER, ((i + prevMeshNumVertices) * sizeof(Vertex)) + offsetof(Vertex, textureIndex), sizeof(vertex.textureIndex), &vertex.textureIndex);
		}else{
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			glBufferSubData(GL_ARRAY_BUFFER, ((i + prevMeshNumVertices) * sizeof(Vertex)) + offsetof(Vertex, TexCoords)   , sizeof(vertex.TexCoords), &vertex.TexCoords);
		}
		vertices.push_back(vertex);
		//glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(Vertex), sizeof(Vertex), &vertex);
	}

	// process faces
	// -------------
	for(unsigned int i = 0; i != mesh->mNumFaces; i++){
		aiFace face = mesh->mFaces[i];
		for(unsigned int j = 0; j != face.mNumIndices; j++)
			indices.push_back(face.mIndices[j] + prevMeshNumVertices);
	}

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

	prevMeshNumVertices += mesh->mNumVertices;
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

unsigned int getNumVertices(aiNode *node, const aiScene *scene){
	unsigned int NumVertices = 0;
	for(unsigned int i = 0; i != node->mNumMeshes; i++)
		NumVertices += scene->mMeshes[node->mMeshes[i]]->mNumVertices;
	for(unsigned int i = 0; i != node->mNumChildren; i++)
		NumVertices += getNumVertices(node->mChildren[i], scene);
	return NumVertices;	
}

// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void loadModel(const std::string& path){
	std::vector<Mesh>    meshes;
	// read file via ASSIMP
	Assimp::Importer import;
	const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_OptimizeGraph);
	//check for importing errors
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
		printf("ERROR::ASSIMP::%s\n", import.GetErrorString());
		return;
	}

	unsigned int sizeofthebuffer = getNumVertices(scene->mRootNode, scene);

	glGenVertexArrays(true, &BIGMesh.VAO);
	glGenBuffers(true, &BIGMesh.VBO);
	glGenBuffers(true, &BIGMesh.EBO);

	glBindVertexArray(BIGMesh.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, BIGMesh.VBO);

	glBufferData(GL_ARRAY_BUFFER, (sizeofthebuffer * sizeof(BIGMesh.vertices[0])), 0, GL_STATIC_DRAW);

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

	//BIGMesh.setupMesh();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BIGMesh.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, BIGMesh.indices.size() * sizeof(BIGMesh.indices[0]), &BIGMesh.indices[0], GL_STATIC_DRAW);


	for (unsigned int i = 0; i != BIGMesh.vertices.size(); i++){
		//glBufferSubData(GL_ARRAY_BUFFER, (i * sizeof(Vertex)) + offsetof(Vertex, Position)     , sizeof(BIGMesh.vertices[i].Position), &BIGMesh.vertices[i].Position);
		//glBufferSubData(GL_ARRAY_BUFFER, (i * sizeof(Vertex)) + offsetof(Vertex, Normal)       , sizeof(BIGMesh.vertices[i].Normal)       , &BIGMesh.vertices[i].Normal);
		//glBufferSubData(GL_ARRAY_BUFFER, (i * sizeof(Vertex)) + offsetof(Vertex, TexCoords)    , sizeof(BIGMesh.vertices[i].TexCoords)    , &BIGMesh.vertices[i].TexCoords);
		//glBufferSubData(GL_ARRAY_BUFFER, (i * sizeof(Vertex)) + offsetof(Vertex, textureIndex) , sizeof(BIGMesh.vertices[i].textureIndex) , &BIGMesh.vertices[i].textureIndex);
		glBufferSubData(GL_ARRAY_BUFFER, (i * sizeof(Vertex)) + offsetof(Vertex, boneIDs)      , sizeof(BIGMesh.vertices[i].boneIDs)      , &BIGMesh.vertices[i].boneIDs);
		glBufferSubData(GL_ARRAY_BUFFER, (i * sizeof(Vertex)) + offsetof(Vertex, weights)      , sizeof(BIGMesh.vertices[i].weights)      , &BIGMesh.vertices[i].weights);
	}

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, Normal)));
	
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, TexCoords)));

	// vertex tangent
	//glEnableVertexAttribArray(3);
	//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeofVertex, (void*)offsetof(Vertex, Tangent));

	// vertex bitangent
	//glEnableVertexAttribArray(4);
	//glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeofVertex, (void*)offsetof(Vertex, Bitangent));

	// material id used by the vertx
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE , sizeof(Vertex), (void*)(offsetof(Vertex, textureIndex))); // * BIGMesh.vertices.size()));

	// bone ids
	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT  , sizeof(Vertex), (void*)(offsetof(Vertex, boneIDs)));// * BIGMesh.vertices.size()));

	// weights
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, weights))); // * BIGMesh.vertices.size()));

	glBindVertexArray(0);

	for(unsigned int i = 0; i != BIGMesh.textures.size(); i++){

		glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
		
		std::string name = BIGMesh.textures[i].type;
		unsigned char textureID = BIGMesh.textures[i].id;

		if     (name == "texture_diffuse"){
			BIGMesh.defuseTexturesIDs.push_back(textureID);
			glBindTextureUnit(textureID , textureID);
		}
		else if(name == "texture_specular")
			BIGMesh.specularTexturesIDs.push_back(textureID);
		else if(name == "texture_normal")
			BIGMesh.normalTexturesIDs.push_back(textureID);
		else if(name == "texture_height")
			BIGMesh.heightTexturesIDs.push_back(textureID);
	}

}

Model::Model(const char* path){
	loadModel(path);
}

// draws the model, and thus all its meshes
void Model::Draw(Shader &shader){
	//BIGMesh.Draw(shader);

	//batch the textures and upload them the GPU
	GLint location = glGetUniformLocation(shader.ID, "texture_diffuse");
	glUniform1iv(location, BIGMesh.defuseTexturesIDs.size() , BIGMesh.defuseTexturesIDs.data());
	
	//upload the inddeces to the GPU
	glBindVertexArray(BIGMesh.VAO);
	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(BIGMesh.indices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);

}

std::unordered_map<std::string, BoneInfo>& Model::GetBoneInfoMap() const { return m_BoneInfoMap; }
unsigned char* Model::GetBoneCount() const { return &m_BoneCounter; }
