#include <assert.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cstddef>
#include <cstring>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <vector>
#include "../headers/model.h"

// model data
std::vector<Texture> textures_loaded;
std::vector<Texture> gTextures;
std::vector<GLint>   diffuseTexturesIDs;
std::vector<GLint>   specularTexturesIDs;
std::vector<GLint>   normalTexturesIDs;
std::vector<GLint>   heightTexturesIDs;
Mesh                 BIGMesh;
static unsigned char m_BoneCounter = 1;
static unsigned int  prevMeshNumVertices = 0;
static unsigned int  prevMeshNumIndices = 0;
std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;
unsigned char        size_of_vertex = sizeof(Vertex);


unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false){

	std::string filename = directory + '/' + path;
	const char* c_filename = filename.c_str();

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(c_filename, &width, &height, &nrComponents, 0);
	
	if (data){
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;
		else{
			fprintf(stderr, "texture: \"%s\" has %d component which is invalid\n", c_filename, nrComponents);
			format = GL_FALSE;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}else{
		fprintf(stderr, "Texture failed to load at path: %s\n", c_filename);
	}
	
	stbi_image_free(data);
	return textureID;
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
std::vector<Texture> loadMaterialTextures(const aiMaterial *mat, aiTextureType type, const char* typeName, const char* dir){
	
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
		if(!skip){	// if texture hasn't been loaded already, load it
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), dir);
			texture.path = str.C_Str();
			
			for(unsigned char i=0; i!=(strlen(typeName) + 1); i++)
				texture.type[i] = typeName[i];

			//printf("strlen1 : %zu\n", strlen(typeName));
			//printf("strlen2 : %zu\n", strlen(texture.type));

			textures.push_back(texture);
			textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
		}
	}
	return textures;
}

Mesh processMesh(aiMesh* mesh, const aiScene* scene , const char* dir){
	std::vector<vertexBoneData> tmpVerticesBoneData;
	std::vector<Texture>        textures;

	// process meshes
	// --------------
	for(unsigned int i = 0; i != mesh->mNumVertices; i++){
		vertexBoneData tmpVertexBoneData;
		
		if (tmpVerticesBoneData.size() == prevMeshNumVertices)
			tmpVerticesBoneData.reserve(mesh->mNumVertices);
		
		//vertices.reserve(mesh->mNumVertices);
		for(int i = 0; i != MAX_BONE_INFLUENCE; i++){
			tmpVertexBoneData.boneIDs[i] = 0;
			tmpVertexBoneData.weights[i] = 0.0f;
		}

		//process mesh positions if it has any
		if(mesh->HasPositions())
			glBufferSubData(GL_ARRAY_BUFFER, (i + prevMeshNumVertices) * size_of_vertex, sizeof(mesh->mVertices[i]), &mesh->mVertices[i]);	
	
		// process normals if it has any
		if(mesh->HasNormals())
			glBufferSubData(GL_ARRAY_BUFFER, ((i + prevMeshNumVertices) * size_of_vertex) + offsetof(Vertex, Normal), sizeof(mesh->mNormals[i]), &mesh->mNormals[i]);
	
		// load textures if it has any
		if(mesh->HasTextureCoords(0)){
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			glBufferSubData(GL_ARRAY_BUFFER, ((i + prevMeshNumVertices) * size_of_vertex) + offsetof(Vertex, TexCoords)   , sizeof(vec), &vec);
			glBufferSubData(GL_ARRAY_BUFFER, ((i + prevMeshNumVertices) * size_of_vertex) + offsetof(Vertex, textureIndex), sizeof(mesh->mMaterialIndex), &mesh->mMaterialIndex);
		}else{
			glm::vec2 nullvec = glm::vec2(0.0f);
			glBufferSubData(GL_ARRAY_BUFFER, ((i + prevMeshNumVertices) * size_of_vertex) + offsetof(Vertex, TexCoords)   , sizeof(nullvec), &nullvec);
		}
		tmpVerticesBoneData.emplace_back(tmpVertexBoneData);
	}

	// process indices
	// -------------
	for(unsigned int i = 0; i != mesh->mNumFaces; i++){
		aiFace face = mesh->mFaces[i];
		for(unsigned int j = 0; j != face.mNumIndices; j++){
			unsigned int tmpCuzPtrs = face.mIndices[j] + prevMeshNumVertices;
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (((i * face.mNumIndices) + j + prevMeshNumIndices) * sizeof(uint)), sizeof(tmpCuzPtrs), &tmpCuzPtrs);
		}
	}

	// process bones
	// -------------
	if(mesh->HasBones()){
		for(uint boneIndex = 0; boneIndex != mesh->mNumBones; boneIndex++){
			unsigned char boneID = 0;
			aiBone* bone = mesh->mBones[boneIndex];
			const char* boneName = bone->mName.C_Str();
			if(m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()){
				BoneInfo newBoneInfo;
				newBoneInfo.id = m_BoneCounter;
				newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(bone->mOffsetMatrix);
				m_BoneInfoMap[boneName] = newBoneInfo;
				boneID = m_BoneCounter;
				m_BoneCounter++;
			}else{
				boneID = m_BoneInfoMap[boneName].id;
			}
			assert(boneID);

			for(uint weightIndex = 0; weightIndex != mesh->mBones[boneIndex]->mNumWeights; weightIndex++){
				unsigned int vertexId = bone->mWeights[weightIndex].mVertexId;
				assert(vertexId <= (mesh->mNumVertices));
				
				for(unsigned char boneIndex = 0; boneIndex != MAX_BONE_INFLUENCE; boneIndex++){
					if(!tmpVerticesBoneData[vertexId].boneIDs[boneIndex]){
						tmpVerticesBoneData[vertexId].weights[boneIndex] = bone->mWeights->mWeight;
						tmpVerticesBoneData[vertexId].boneIDs[boneIndex] = boneID;
						break;
					}
				}
				glBufferSubData(GL_ARRAY_BUFFER, ((vertexId + prevMeshNumVertices) * size_of_vertex) + offsetof(Vertex, boneIDs),
					sizeof(tmpVerticesBoneData.begin()->boneIDs), &tmpVerticesBoneData[vertexId].boneIDs);
				glBufferSubData(GL_ARRAY_BUFFER, ((vertexId + prevMeshNumVertices) * size_of_vertex) + offsetof(Vertex, weights),
					sizeof(tmpVerticesBoneData.begin()->weights), &tmpVerticesBoneData[vertexId].weights);
			}
		}
	}

	// assigne textures
	// ----------------
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "textureDiffus", dir);
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "textureSpecul", dir);
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "textureNormal", dir);
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "textureHeight", dir);
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
	
	gTextures.insert(gTextures.begin(), textures.begin(), textures.end());

	/*for(unsigned int i = 0; i != diffuseMaps.size(); i++){

		glActiveTexture(GL_TEXTURE0 + mesh->mMaterialIndex + i); // activate proper texture unit before binding

		printf("**************************\n");
		printf("mesh->mMaterialIndex = %d\n", mesh->mMaterialIndex);


		std::string name = diffuseMaps[i].type;
		unsigned int textureID = diffuseMaps[i].id;

		printf("textureID = %d\n", textureID);
		//printf("name : %s\n", name.c_str());


		if(name == "textureDiffus"){
			diffuseTexturesIDs.push_back(textureID);	
			glBindTextureUnit(textureID, textureID);
			//printf("1 specular loaded : %d\n", textureID);
		}else if(name == "textureSpecul"){
			BIGMesh.specularTexturesIDs.push_back(textureID);
		}else if(name == "textureNormal"){
			BIGMesh.normalTexturesIDs.push_back(textureID);
		}else if(name == "textureHeight"){
			BIGMesh.heightTexturesIDs.push_back(textureID);
		}
	}*/

	prevMeshNumVertices += mesh->mNumVertices;
	prevMeshNumIndices  += mesh->mNumFaces * mesh->mFaces->mNumIndices;
	//glBindTextureUnit(3 , 3);
	return Mesh(textures);
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void processNode(aiNode *node, const aiScene *scene,std::vector<Mesh>& meshes, const char* dir){
	// process all the node's meshes (if any)
	for(unsigned int i = 0; i != node->mNumMeshes; i++)
		meshes.emplace_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene, dir));
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

unsigned int getNumIndices(aiNode *node, const aiScene *scene){
	unsigned int NumIndices = 0;
	for(unsigned int i = 0; i != node->mNumMeshes; i++){
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		for(unsigned int j = 0; j != mesh->mNumFaces; j++)
			NumIndices += mesh->mFaces[j].mNumIndices;
	}
	for(unsigned int i = 0; i != node->mNumChildren; i++)
		NumIndices += getNumIndices(node->mChildren[i], scene);
	return NumIndices;
}

unsigned int getNumMeshs(aiNode *node, const aiScene *scene){
	unsigned int numMeshs = 0;
	numMeshs += node->mNumMeshes;
	for(unsigned int i = 0; i != node->mNumChildren; i++)
		numMeshs += getNumMeshs(node->mChildren[i], scene);
	return numMeshs;
}

// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void loadModel(const std::string& path){
	std::vector<Mesh>    meshes;
	
	// read file via ASSIMP
	Assimp::Importer import;
	const aiScene *scene = import.ReadFile(path, 
		aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);
	
	//check for importing errors
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
		fprintf(stderr, "ERROR::ASSIMP::%s\n", import.GetErrorString());
		return;
	}

	unsigned int size_of_the_array_buffer_in_bytes   = getNumVertices(scene->mRootNode, scene) * size_of_vertex;
	unsigned int size_of_the_element_buffer_in_bytes = getNumIndices(scene->mRootNode, scene) * size_of_vertex;

	meshes.reserve(getNumMeshs(scene->mRootNode, scene));

	glGenVertexArrays(true, &BIGMesh.VAO);
	glGenBuffers(true, &BIGMesh.VBO);
	glGenBuffers(true, &BIGMesh.EBO);

	glBindVertexArray(BIGMesh.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, BIGMesh.VBO);
	glBufferData(GL_ARRAY_BUFFER, size_of_the_array_buffer_in_bytes, nullptr, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BIGMesh.EBO);
	
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_of_the_element_buffer_in_bytes, nullptr, GL_STATIC_DRAW);

	std::string directory = path.substr(0, path.find_last_of('/'));
	processNode(scene->mRootNode, scene, meshes, directory.c_str());

	for (unsigned int i = 0; i!= meshes.size(); i++){
		for (unsigned int j = 0; j != meshes[i].textures.size(); j++)
			BIGMesh.textures.push_back(meshes[i].textures[j]);
	}

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, size_of_vertex, (void*)0);

	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, size_of_vertex, (void*)(offsetof(Vertex, Normal)));
	
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, size_of_vertex, (void*)(offsetof(Vertex, TexCoords)));

	// vertex tangent
	//glEnableVertexAttribArray(3);
	//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeofVertex, (void*)offsetof(Vertex, Tangent));

	// vertex bitangent
	//glEnableVertexAttribArray(4);
	//glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeofVertex, (void*)offsetof(Vertex, Bitangent));

	// material id used by the vertx
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE , size_of_vertex, (void*)(offsetof(Vertex, textureIndex)));

	// bone ids
	// it a char[4] but for sort of comprissing we pack it to one int
	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT  , size_of_vertex, (void*)(offsetof(Vertex, boneIDs)));

	// weights
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, size_of_vertex, (void*)(offsetof(Vertex, weights)));

	glBindVertexArray(0);

	for(int i=(gTextures.size()-1); i!=-1; i--){

		glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
		//printf("***************************\n");
		//printf("non-batched i = %d\n", i);
		
		std::string name = gTextures[i].type;
		unsigned char textureID = gTextures[i].id;
		//printf("non-batched textureID = %d\n", textureID);
		//printf("non-batched name : %s\n", name.c_str());
		
		if     (name == "textureDiffus"){
			diffuseTexturesIDs.push_back(textureID);
			glBindTextureUnit(textureID , textureID);
		}
		else if(name == "texture_specular")
			specularTexturesIDs.push_back(textureID);
		else if(name == "texture_normal")
			normalTexturesIDs.push_back(textureID);
		else if(name == "texture_height")
			heightTexturesIDs.push_back(textureID);
	}
}

Model::Model(const char* path){
	loadModel(path);
}

// draws the model, and thus all its meshes
void Model::Draw(Shader &shader){
	//batch the textures and upload them the GPU
	GLint location = glGetUniformLocation(shader.ID, "texture_diffuse");
	glUniform1iv(location, diffuseTexturesIDs.size(), diffuseTexturesIDs.data());
	
	//upload the inddeces to the GPU
	glBindVertexArray(BIGMesh.VAO);
	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(prevMeshNumIndices), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}

std::unordered_map<std::string, BoneInfo>& Model::GetBoneInfoMap() const { return m_BoneInfoMap; }
unsigned char* Model::GetBoneCount() const { return &m_BoneCounter; }
