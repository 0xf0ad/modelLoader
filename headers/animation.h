#pragma once

#include <assimp/anim.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <vector>

#include "bone.h"
#include "model.h"

struct AssimpNodeData{
	glm::mat4 transformation;
	BoneInfo* bone;
	const char* name;
	std::vector<AssimpNodeData> children;
};


#define ANIM_IMPORT_FLAGS               \
	aiProcess_Triangulate              |\
	aiProcess_RemoveComponent          |\
	aiProcess_OptimizeGraph            |\
	aiProcess_OptimizeMeshes           |\
	aiProcess_JoinIdenticalVertices    |\
	aiProcess_LimitBoneWeights         |\
	aiProcess_ImproveCacheLocality     |\
	aiProcess_FindDegenerates

#define ANIM_REMOVED_COMPONENTS         \
	aiComponent_NORMALS                |\
	aiComponent_TANGENTS_AND_BITANGENTS|\
	aiComponent_COLORS                 |\
	aiComponent_TEXCOORDS              |\
	aiComponent_TEXTURES               |\
	aiComponent_LIGHTS                 |\
	aiComponent_CAMERAS                |\
	aiComponent_MATERIALS


class Animation{
public:
	float mDuration;
	int mTicksPerSecond;

	#if FANCYCPPFEUTRES
	std::vector<Bone> mBones;		// a vector of bones(obviously) 
	#endif
	const char** mAnimationsNames;
	uint32_t mNumAnimations;
	AssimpNodeData mRootNode;
	//std::unordered_map<const char*, BoneInfo, strHash, strequal_to> mBoneInfoMap;	// a hash table of bones and their names
	std::unordered_map<std::string, BoneInfo, stdstrHash, stdstrequal_to>* mBoneInfoMap;	// a hash table of bones and their names
	Bone **BonesArray;
	uint8_t boneNum;

	Animation() = default;

	Animation(const char* animationPath, Model* model, uint8_t animIndex = 0){

		// import the model from the given path
		Assimp::Importer importer;

		importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, ANIM_REMOVED_COMPONENTS);
		importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);

		const aiScene* scene = importer.ReadFile(animationPath, ANIM_IMPORT_FLAGS);


		// error handlling
		if(!(scene && scene->mRootNode)){
			fprintf(stderr, "ERROR::ASSIMP::%s\n", importer.GetErrorString());
			return;
		}
		if(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE){
			fprintf(stderr, "pre-processor did not complet\n");
		}
		if(!scene->HasAnimations()){
			fprintf(stderr, "that path doesn't have animations\n");
			return;
		}

		#if 0
		assert(scene && scene->mRootNode);
		#endif

		// reserve the number of bones to the hashed map and the bones vector(dynamic array)
		boneNum = model->m_BoneCounter - 1;
		printf("numBones : %d\n", boneNum);
		//mBones.reserve(boneNum);
		BonesArray = (Bone**) malloc(sizeof(Bone[boneNum+1]));
		mBoneInfoMap = &model->boneInfoMap;
		mBoneInfoMap->reserve(boneNum);
		fillAnimationVector(scene);

		mDuration = scene->mAnimations[animIndex]->mDuration;
		mTicksPerSecond = scene->mAnimations[animIndex]->mTicksPerSecond;
		readMissingBones(scene->mAnimations[animIndex], model);
		readHeirarchyData(&mRootNode, scene->mRootNode, model);
		printf("numBones : %d\n", model->m_BoneCounter);
	}

	Animation(const aiScene* scene, Model* model, uint8_t animIndex = 0){
		// reserve the number of bones to the hashed map and the bones vector(dynamic array)
		// get the bone number by subtracting 1 from the boneCount
		boneNum = model->m_BoneCounter - 1;
		#if FANCYCPPFEUTRES
		mBones.reserve(boneNum);
		#endif
		mBoneInfoMap = &model->boneInfoMap;
		mBoneInfoMap->reserve(boneNum);
		BonesArray = (Bone**) malloc(sizeof(Bone[boneNum+1]));
		fillAnimationVector(scene);

		mDuration = scene->mAnimations[animIndex]->mDuration;
		mTicksPerSecond = scene->mAnimations[animIndex]->mTicksPerSecond;
		readMissingBones(scene->mAnimations[animIndex], model);
		readHeirarchyData(&mRootNode, scene->mRootNode, model);
	}

	~Animation() {
		// freed heap allocated strings
		free(mAnimationsNames);
		free(BonesArray);
		freeNodeHeirarchy(&mRootNode);	
	}

	#if FANCYCPPFEUTRES
	Bone* FindBone(const char* name){
		// idk wtf is going on
		auto iter = std::find_if(mBones.begin(), mBones.end(), [&](const Bone& bone){
			return (!strcmp(bone.mName, name));
		});

		if (iter == mBones.end()) return nullptr;
		else return &(*iter);
	}
	#endif

	Bone* FindBone(uint8_t id){
		return (id <= boneNum) ? BonesArray[id] : nullptr;
	}

private:

	void fillAnimationVector(const aiScene* scene){
		mNumAnimations = scene->mNumAnimations;
		mAnimationsNames = (const char**) malloc(mNumAnimations);
		for (uint32_t i=0; i != scene->mNumAnimations; i++)
			mAnimationsNames[i] = strdup(scene->mAnimations[i]->mName.C_Str());

	}
	
	// THIS FUNCTION FOR SOME REASON DONT FOUND THE EXISTING CONST CHARs* ON THE BONEINFOMAP
	void readMissingBones(const aiAnimation* animation, Model* model){
		// getting m_BoneInfoMap and boneCount from Model class
		//std::unordered_map<const char*, BoneInfo, strHash, strequal_to>& boneInfoMap = model->GetBoneInfoMap();
		std::unordered_map<std::string, BoneInfo, stdstrHash, stdstrequal_to>& boneInfoMap = model->boneInfoMap;

		// reading channels(bones engaged in an animation and their keyframes)
		for (uint32_t i = 0; i != animation->mNumChannels; i++){

			const aiNodeAnim* channel = animation->mChannels[i];
			const char* nodeName = channel->mNodeName.C_Str();

			if (boneInfoMap.find(nodeName) == boneInfoMap.end())
				boneInfoMap[nodeName].id = ++boneNum;

			Bone* thaBone = (Bone*) malloc(sizeof(Bone));
			new (thaBone) Bone(nodeName, boneInfoMap[nodeName].id, channel);
			BonesArray[boneInfoMap[nodeName].id] = thaBone;

			#if FANCYCPPFEUTRES
			mBones.emplace_back(*thaBone);
			#endif

		}
		model->m_BoneCounter = boneNum + 1;
	}

	void readHeirarchyData(AssimpNodeData* dest, const aiNode* src, Model* model){
		static std::unordered_map<std::string, BoneInfo, stdstrHash, stdstrequal_to>& boneInfoMap = model->boneInfoMap;
		if(src){
			// write the node data to the AssimpNodeData
			dest->name = strdup(src->mName.C_Str());
			dest->transformation = assimpMatrix2glm(src->mTransformation);

			if (boneInfoMap.find(dest->name) != boneInfoMap.end())
				dest->bone = &boneInfoMap[dest->name];
			else
				dest->bone = nullptr;

			dest->children.reserve(src->mNumChildren);

			// load the children to the children vector
			for (uint32_t i = 0; i != src->mNumChildren; i++){
				AssimpNodeData newData;
				readHeirarchyData(&newData, src->mChildren[i], model);
				dest->children.emplace_back(newData);
			}
		}else{
			fprintf(stderr, "could not read the animation heirarchy\n");
			return;
		}
	}

	void freeNodeHeirarchy(AssimpNodeData* src){
		if(src){
			// free the node name which is allocated on the heap
			free((void*)src->name);

			// recall this function recursivlly on every children of that node
			for(uint32_t i = 0; i != src->children.size(); i++)
				freeNodeHeirarchy(&src->children[i]);
		}
	}
};
