#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "../headers/model.h"
#include "../headers/animation.h"

// model data
std::vector<GLint> diffuseTexturesIDs;
std::vector<GLint> specularTexturesIDs;
std::vector<GLint> normalTexturesIDs;
std::vector<GLint> heightTexturesIDs;
static uint8_t     size_of_vertex = sizeof(Vertex);

#define IMPORT_FLAGS                     \
	aiProcess_Triangulate               |\
	aiProcess_GenSmoothNormals          |\
	aiProcess_JoinIdenticalVertices     |\
	aiProcess_GenUVCoords               |\
	aiProcess_LimitBoneWeights          |\
	aiProcess_RemoveComponent           |\
	aiProcess_RemoveRedundantMaterials  |\
	aiProcess_OptimizeMeshes            |\
	aiProcess_OptimizeGraph             |\
	aiProcess_ImproveCacheLocality      |\
	aiProcess_FindDegenerates           |\
	aiProcess_FindInvalidData

#define REMOVED_COMPONENTS               \
	aiComponent_TANGENTS_AND_BITANGENTS |\
	aiComponent_COLORS                  |\
	aiComponent_LIGHTS                  |\
	aiComponent_CAMERAS



uint32_t TextureFromFile(const char* path, const char* directory, const aiTexture* emTexture/*, bool gamma = false*/){

	size_t strlenth = strlen(directory) + strlen(path) + 2;
	char filename[strlenth];
	snprintf(filename, strlenth, "%s/%s", directory, path);

	for(size_t i = 0; i != strlenth; i++)
		if(filename[i] == '\\')
			filename[i] = '/';

	printf("load texture : %s\n", filename);

	uint32_t textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	uint8_t* data;

	// check if the texture is embedded on the model file
	if(!emTexture)
		data = stbi_load(filename, &width, &height, &nrComponents, 0);
	else
		data = stbi_load_from_memory((stbi_uc*)emTexture->pcData, emTexture->mWidth,
		                           &width, &height, &nrComponents, 0);

	if(data){
		GLenum format = GL_FALSE;
		GLenum formats[4] = {GL_RED, GL_FALSE, GL_RGB, GL_RGBA};
		if(nrComponents < 5)
			format = formats[nrComponents - 1];
		else
			fprintf(stderr, "texture: \"%s\" has %d component which is invalid\n", filename, nrComponents);

		glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	} else {
		fprintf(stderr, "Texture failed to load at path: %s\n\t%s |/| %s\n%p\n", filename, directory, path, emTexture);
	}

	stbi_image_free(data);
	return textureID;
}


// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
void loadMaterialTextures(std::vector<Texture>& textures, const aiMaterial *mat, aiTextureType type, const char* dir, const aiScene* scene){

	size_t numTextures = mat->GetTextureCount(type);
	static std::vector<Texture> textures_loaded;
	textures_loaded.reserve(numTextures);
	textures.reserve(numTextures);

	for(uint32_t i = 0; i != numTextures; i++){
		aiString str;
		// get the texture
		if(!mat->GetTexture(type, i, &str)){

			// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
			stbi_set_flip_vertically_on_load(true);

			// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			bool alreeadyLoaded = false;
			for(uint32_t j = 0; j != textures_loaded.size(); j++){
				if(!strcmp(textures_loaded[j].path.c_str(), str.C_Str())){
					textures.emplace_back(textures_loaded[j]);
					alreeadyLoaded = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}

			if(!alreeadyLoaded){	// if texture hasn't been loaded already, load it
				Texture texture;
				const aiTexture* emTexture = scene->GetEmbeddedTexture(str.C_Str());
				texture.id = TextureFromFile(str.C_Str(), dir, emTexture);
				texture.path = str.C_Str();
				texture.type = type;
				textures.emplace_back(texture);
				// store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
				textures_loaded.emplace_back(texture);
			}
		}
	}
}

void processMesh(Model* model, aiMesh* mesh, const aiScene* scene, const char* dir){
	std::vector<vertexBoneData> tmpVerticesBoneData;
	std::vector<Texture>        textures;

	// process meshes
	// --------------
	tmpVerticesBoneData.reserve(mesh->mNumVertices);

	for(uint32_t i = 0; i != mesh->mNumVertices; i++){
		vertexBoneData tmpVertexBoneData;

		//process mesh positions if it has any
		if(mesh->HasPositions())
			glBufferSubData(GL_ARRAY_BUFFER, ((i + model->prevMeshNumVertices) * size_of_vertex) + offsetof(Vertex, Position)    , sizeof(mesh->mVertices[i])  , &mesh->mVertices[i]);	

		// process normals if it has any
		if(mesh->HasNormals())
			glBufferSubData(GL_ARRAY_BUFFER, ((i + model->prevMeshNumVertices) * size_of_vertex) + offsetof(Vertex, Normal)      , sizeof(mesh->mNormals[i])   , &mesh->mNormals[i]);

		// load textures if it has any if not load null coords insted
		if(mesh->HasTextureCoords(0)){
			// insert the texture coordinates to the VAO directly from ASSIMP
			glBufferSubData(GL_ARRAY_BUFFER, ((i + model->prevMeshNumVertices) * size_of_vertex) + offsetof(Vertex, TexCoords)   , sizeof(float[2])            , &mesh->mTextureCoords[0][i]);
			glBufferSubData(GL_ARRAY_BUFFER, ((i + model->prevMeshNumVertices) * size_of_vertex) + offsetof(Vertex, textureIndex), sizeof(mesh->mMaterialIndex), &mesh->mMaterialIndex);
		}else{
			glm::vec2 nullvec(0.0f);	/* I should define a variable to use a pointer with in the next function */
			glBufferSubData(GL_ARRAY_BUFFER, ((i + model->prevMeshNumVertices) * size_of_vertex) + offsetof(Vertex, TexCoords)   , sizeof(nullvec)             , &nullvec);
		}
		tmpVerticesBoneData.emplace_back(tmpVertexBoneData);
	}

	// process indices
	// -------------
	for(uint32_t i = 0; i != mesh->mNumFaces; i++){
		aiFace face = mesh->mFaces[i];
		for(uint32_t j = 0; j != face.mNumIndices; j++){
			uint32_t tmpCuzPtrs = face.mIndices[j] + model->prevMeshNumVertices;
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (((i * face.mNumIndices) + j + model->prevMeshNumIndices) * sizeof(uint32_t)), sizeof(tmpCuzPtrs), &tmpCuzPtrs);
		}
	}

	// process bones
	// -------------
	if(mesh->HasBones()){
		for(uint32_t boneIndex = 0; boneIndex != mesh->mNumBones; boneIndex++){
			uint8_t boneID = 0;
			aiBone* bone = mesh->mBones[boneIndex];
			const char* boneName = bone->mName.C_Str();
			if(model->boneInfoMap.find(boneName) == model->boneInfoMap.end()){
				BoneInfo newBoneInfo;
				newBoneInfo.id = model->m_BoneCounter;
				newBoneInfo.offset = assimpMatrix2glm(bone->mOffsetMatrix);
				model->offsets.push_back(newBoneInfo.offset);
				model->boneInfoMap[boneName] = newBoneInfo;
				boneID = model->m_BoneCounter;
				model->m_BoneCounter++;
			}else{
				boneID = model->boneInfoMap[boneName].id;
			}
			assert(boneID);

			for(uint32_t weightIndex = 0; weightIndex != mesh->mBones[boneIndex]->mNumWeights; weightIndex++){
				uint32_t vertexId = bone->mWeights[weightIndex].mVertexId;
				assert(vertexId <= (mesh->mNumVertices));

				for(uint8_t boneIndex = 0; boneIndex != MAX_BONE_INFLUENCE; boneIndex++){
					if(!tmpVerticesBoneData[vertexId].boneIDs[boneIndex]){
						tmpVerticesBoneData[vertexId].weights[boneIndex] = bone->mWeights->mWeight;
						tmpVerticesBoneData[vertexId].boneIDs[boneIndex] = boneID;
						break;
					}
				}
				glBufferSubData(GL_ARRAY_BUFFER, ((vertexId + model->prevMeshNumVertices) * size_of_vertex) + offsetof(Vertex, boneIDs),
					sizeof(tmpVerticesBoneData.begin()->packed_IDs), &tmpVerticesBoneData[vertexId].packed_IDs);
				glBufferSubData(GL_ARRAY_BUFFER, ((vertexId + model->prevMeshNumVertices) * size_of_vertex) + offsetof(Vertex, weights),
					sizeof(tmpVerticesBoneData.begin()->weights), &tmpVerticesBoneData[vertexId].weights);
			}
		}
	}

	// assigne textures
	// ----------------
	const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	loadMaterialTextures(textures, material, aiTextureType_DIFFUSE,  dir, scene);
	loadMaterialTextures(textures, material, aiTextureType_SPECULAR, dir, scene);
	loadMaterialTextures(textures, material, aiTextureType_HEIGHT,   dir, scene);
	loadMaterialTextures(textures, material, aiTextureType_AMBIENT,  dir, scene);

	for(uint32_t i = 0; i != textures.size(); i++){

		aiTextureType typeName = textures[i].type;
		GLuint textureID = textures[i].id;

		glActiveTexture(GL_TEXTURE0 + mesh->mMaterialIndex + i); // activate proper texture unit before binding

		if(typeName == aiTextureType_DIFFUSE){
			diffuseTexturesIDs.push_back(textureID);
			glBindTextureUnit(textureID, textureID);
		}else if(typeName == aiTextureType_SPECULAR){
			specularTexturesIDs.push_back(textureID);
		}else if(typeName == aiTextureType_HEIGHT){
			normalTexturesIDs.push_back(textureID);
		}else if(typeName == aiTextureType_AMBIENT){
			heightTexturesIDs.push_back(textureID);
		}
	}

	model->prevMeshNumVertices += mesh->mNumVertices;
	model->prevMeshNumIndices  += mesh->mNumFaces * mesh->mFaces->mNumIndices;
}

// processes a node in a recursive fashion. Processes each individual mesh located at 
// the node and repeats this process on its children nodes (if any).
void processNode(Model* model, aiNode *node, const aiScene *scene, uint32_t offset, const char* dir){
	// process all the node's meshes (if any)
	uint32_t i = offset;
	for(; i != (node->mNumMeshes + offset); i++)
		processMesh(model, scene->mMeshes[node->mMeshes[i - offset]], scene, dir);

	for(uint32_t j = 0; j != node->mNumChildren; j++)	// then do the same for each of its children
		processNode(model, node->mChildren[j], scene, i, dir);
}

// get the number of meshes and indices and verices from a model and stick it into 
// 3 variables entered by ther addresses as parametres
void getThemAll(uint32_t* numMeshes, uint32_t* numIndices, uint32_t* numVertices, aiNode* node, const aiScene* scene){

	*numMeshes += node->mNumMeshes;

	for(uint32_t i = 0; i != node->mNumMeshes; i++){
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		*numVertices += scene->mMeshes[node->mMeshes[i]]->mNumVertices;
		for(uint32_t j = 0; j != mesh->mNumFaces; j++)
			*numIndices += mesh->mFaces[j].mNumIndices;
	}

	for(uint32_t i = 0; i != node->mNumChildren; i++){
		getThemAll(numMeshes, numIndices, numVertices, node->mChildren[i], scene);
	}
}

// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void loadModel(Model* model, const char* path){
	// read file via ASSIMP
	Assimp::Importer importer;
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, REMOVED_COMPONENTS);
	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
	const aiScene *scene = importer.ReadFile(path, IMPORT_FLAGS);

	//check for importing errors
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
		fprintf(stderr, "ERROR::ASSIMP::%s\n", importer.GetErrorString());
		return;
	}

	getThemAll(&model->numMeshs, &model->numIndices, &model->numVertices, scene->mRootNode, scene);

	uint32_t size_of_the_array_buffer_in_bytes   = model->numVertices * size_of_vertex;
	uint32_t size_of_the_element_buffer_in_bytes = model->numIndices  * size_of_vertex;
	printf("first vertexNumber = %d\nfirst indexNumber = %d\n", model->numVertices, model->numIndices);

	model->hasAnimation = scene->HasAnimations();


	glGenVertexArrays(true, &model->VAO);
	glGenBuffers(true, &model->VBO);
	glGenBuffers(true, &model->EBO);

	glBindVertexArray(model->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, model->VBO);
	glBufferData(GL_ARRAY_BUFFER, size_of_the_array_buffer_in_bytes, nullptr, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->EBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_of_the_element_buffer_in_bytes, nullptr, GL_STATIC_DRAW);
 
	uint32_t difference = strrchr(path, '/') - path;
	const char* directory ;
	directory = strndup(path, difference);

	processNode(model, scene->mRootNode, scene, 0, directory);

	free((void*)directory);


	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, size_of_vertex, (void*)0);

	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, size_of_vertex, (void*)(offsetof(Vertex, Normal)));

	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, size_of_vertex, (void*)(offsetof(Vertex, TexCoords)));

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

	aiMemoryInfo mem;
	importer.GetMemoryRequirements(mem);
	printf("allocated : %i bytes in %i node\n", mem.total, mem.nodes);
	//printf("%i of them are animations\n", in.animations);
}

Model::Model(const char* path){
	loadModel(this, path);
}

// draws the model, and thus all its meshes
void Model::Draw(Shader &shader){
	// batch the textures and upload them the GPU
	GLint location = glGetUniformLocation(shader.ID, "texture_diffuse");
	glUniform1iv(location, diffuseTexturesIDs.size(), diffuseTexturesIDs.data());

	// upload the inddeces to the GPU
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, static_cast<uint32_t>(prevMeshNumIndices), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}

// draws the model, and thus all its meshes
void Model::Draw(Shader &shader, uint32_t count){
	// batch the textures and upload them the GPU
	GLint location = glGetUniformLocation(shader.ID, "texture_diffuse");
	glUniform1iv(location, diffuseTexturesIDs.size(), diffuseTexturesIDs.data());

	// upload the inddeces to the GPU
	glBindVertexArray(VAO);
	glDrawElementsInstanced(GL_TRIANGLES, static_cast<uint32_t>(prevMeshNumIndices), GL_UNSIGNED_INT, 0, count);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}
Model::~Model(){
}
//std::unordered_map<const char*, BoneInfo, strHash, strequal_to>& Model::GetBoneInfoMap() const { return boneInfoMap; }
//std::unordered_map<std::string, BoneInfo, stdstrHash, stdstrequal_to>& Model::GetBoneInfoMap() const { return boneInfoMap; }
