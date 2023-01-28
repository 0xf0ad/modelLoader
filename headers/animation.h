#pragma once

#include <array>
#include <assimp/anim.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <vector>

#include "bone.h"
#include "model.h"

//#define AI_CONFIG_PP_RVC_FLAGS

struct AssimpNodeData{
	glm::mat4 transformation;
	BoneInfo* bone;
	const char* name;
	std::vector<AssimpNodeData> children;
};

class Animation{
public:
	float mDuration;
	int mTicksPerSecond;
	#if FANCYCPPFEUTRES
	//std::vector<Bone> mBones;		// a vector of bones(obviously) 
	#endif
	const char** mAnimationsNames;
	unsigned int mNumAnimations;
	AssimpNodeData mRootNode;
	//std::unordered_map<const char*, BoneInfo, strHash, strequal_to> mBoneInfoMap;	// a hash table of bones and their names (i hate the fact i have to search that table to find bone names on the game loop)
	std::unordered_map<std::string, BoneInfo, stdstrHash, stdstrequal_to> mBoneInfoMap;	// a hash table of bones and their names (i hate the fact i have to search that table to find bone names on the game loop)
	Bone **BonesArray;
	unsigned char boneNum;

	Animation() = default;

	Animation(const char* animationPath, Model* model, unsigned char animIndex = 0){

		// import the model from the given path
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath,
			aiProcess_Triangulate           |\
			/*aiProcess_RemoveComponent       |\*/
			aiProcess_OptimizeGraph         |\
			aiProcess_OptimizeMeshes        |\
			aiProcess_JoinIdenticalVertices |\
			aiProcess_LimitBoneWeights      |\
			aiProcess_ImproveCacheLocality  |\
			/*aiProcess_PreTransformVertices  |\*/
			aiProcess_FindDegenerates       /*|\
			aiProcess_FindInvalidData*/       );


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
		boneNum = *(model->GetBoneCount()) - 1;
		printf("numBones : %d\n", boneNum);
		//mBones.reserve(boneNum);
		BonesArray = (Bone**) malloc(sizeof(Bone[boneNum+1]));
		mBoneInfoMap.reserve(boneNum);
		fillAnimationVector(scene);

		mDuration = scene->mAnimations[animIndex]->mDuration;
		mTicksPerSecond = scene->mAnimations[animIndex]->mTicksPerSecond;
		readMissingBones(scene->mAnimations[animIndex], model);
		readHeirarchyData(&mRootNode, scene->mRootNode, model);
		printf("numBones : %d\n", *(model->GetBoneCount()));
	}

	Animation(const aiScene* scene, Model* model, unsigned char animIndex = 0){
		// reserve the number of bones to the hashed map and the bones vector(dynamic array)
		// get the bone number by subtracting 1 from the boneCount
		boneNum = *(model->GetBoneCount()) - 1;
		#if FANCYCPPFEUTRES
		mBones.reserve(boneNum);
		#endif
		mBoneInfoMap.reserve(boneNum);
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

	Bone* FindBone(unsigned char id){
		if(id <= boneNum)
			return BonesArray[id];
		else
			return nullptr;
	}

private:

	void fillAnimationVector(const aiScene* scene){
		mNumAnimations = scene->mNumAnimations;
		mAnimationsNames = (const char**) malloc(mNumAnimations);
		for (unsigned int i=0; i != scene->mNumAnimations; i++)
			mAnimationsNames[i] = strdup(scene->mAnimations[i]->mName.C_Str());

	}

	
	// THIS FUNCTION FOR SOME REASON DONT FOUND THE EXISTING CONST CHARs* ON THE BONEINFOMAP
	void readMissingBones(const aiAnimation* animation, Model* model){
		// getting m_BoneInfoMap and boneCount from Model class
		//std::unordered_map<const char*, BoneInfo, strHash, strequal_to>& boneInfoMap = model->GetBoneInfoMap();
		std::unordered_map<std::string, BoneInfo, stdstrHash, stdstrequal_to>& boneInfoMap = model->GetBoneInfoMap();
		//m_BoneInfoMap = model->GetBoneInfoMap();
		unsigned char boneCount = *model->GetBoneCount();
		//printf("BoneinfoMap size is %lu\n", boneInfoMap.size());

		// reading channels(bones engaged in an animation and their keyframes)
		for (unsigned int i = 0; i != animation->mNumChannels; i++){

			const aiNodeAnim* channel = animation->mChannels[i];
			const char* nodeName = channel->mNodeName.C_Str();
			//printf("Bone %s\n", nodeName);
			//strcmp(nodeName, boneInfoMap[nodeName]);

			if (boneInfoMap.find(nodeName) == boneInfoMap.end()){
				boneInfoMap[nodeName].id = boneCount++;
				boneNum++;
				//BonesArray = (Bone**) reallocarray(BonesArray, boneNum, sizeof(Bone*));
				/*printf("i found a missing bone %d\n", boneInfoMap[nodeName].id);
				printf("boneCount %d\n", boneCount);
				if(boneInfoMap.find(nodeName) == boneInfoMap.end())
					printf("FUCKING STRANGEEEEEEEEEEEEEE\n");*/
			}/*else{

			}*/

			/*for(unsigned int i = 0; i != m_Bones.size(); i++){
				printf("compare those %s // %s\n", nodeName, m_Bones[i].m_Name.c_str());
				if (strcmp(nodeName, m_Bones[i].m_Name.c_str()) == 0){
					printf("strcmp works hhh\n");
				}
			}*/


			// it3nkch ikhanad achko illa gis std::vector
			Bone* thaBone = (Bone*) malloc(sizeof(Bone));
			*thaBone = Bone(nodeName, boneInfoMap[nodeName].id, channel);
			BonesArray[boneInfoMap[nodeName].id] = thaBone;

			#if FANCYCPPFEUTRES
			mBones.emplace_back(*thaBone);
			#endif

		}
		*model->GetBoneCount() = boneCount;
		//printf("BoneinfoMap size is %lu\n", m_BoneInfoMap.size());
		mBoneInfoMap = boneInfoMap;
	}

	void readHeirarchyData(AssimpNodeData* dest, const aiNode* src, Model* model){
		static std::unordered_map<std::string, BoneInfo, stdstrHash, stdstrequal_to>& boneInfoMap = model->GetBoneInfoMap();
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
			for (unsigned int i = 0; i != src->mNumChildren; i++){
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
			for(unsigned int i = 0; i != src->children.size(); i++)
				freeNodeHeirarchy(&src->children[i]);
		}
	}
};
