#pragma once

#include <assimp/anim.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <climits>
#include <functional>
#include <string.h>
#include <iterator>
#include <memory>
#include <string.h>
#include <unordered_map>
#include <utility>

#include "bone.h"
#include "model.h"

//#define AI_CONFIG_PP_RVC_FLAGS

struct AssimpNodeData{
	glm::mat4 transformation;
	//std::string name;
	const char* name;
	std::vector<AssimpNodeData> children;
};

class Animation{
public:
	float m_Duration;
	int m_TicksPerSecond;
	std::vector<Bone> m_Bones;		// a vector of bones(obviously) 
	std::vector<const char*> m_AnimationsNames;		// a vector of animation names allocated on the heap
	AssimpNodeData m_RootNode;
	//std::unordered_map<const char*, BoneInfo, strHash, strequal_to> m_BoneInfoMap;	// a hash table of bones and their names (i hate the fact i have to search that table to find bone names on the game loop) 
	std::unordered_map<std::string, BoneInfo, stdstrHash, stdstrequal_to> m_BoneInfoMap;	// a hash table of bones and their names (i hate the fact i have to search that table to find bone names on the game loop) 

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
		printf("numBones : %d\n", *(model->GetBoneCount()));
		m_Bones.reserve((*model->GetBoneCount())-1);
		m_BoneInfoMap.reserve((*model->GetBoneCount())-1);
		fillAnimationVector(scene);

		m_Duration = scene->mAnimations[animIndex]->mDuration;
		m_TicksPerSecond = scene->mAnimations[animIndex]->mTicksPerSecond;
		readHeirarchyData(&m_RootNode, scene->mRootNode);
		readMissingBones(scene->mAnimations[animIndex], model);
		printf("numBones : %d\n", *(model->GetBoneCount()));
		importer.FreeScene();
	}

	Animation(const aiScene* scene, Model* model, unsigned char animIndex = 0){
		// reserve the number of bones to the hashed map and the bones vector(dynamic array)
		m_Bones.reserve((*model->GetBoneCount())-1);		// get the bone number by subtracting 1 from the boneCount
		m_BoneInfoMap.reserve((*model->GetBoneCount())-1);	// same here
		fillAnimationVector(scene);

		m_Duration = scene->mAnimations[animIndex]->mDuration;
		m_TicksPerSecond = scene->mAnimations[animIndex]->mTicksPerSecond;
		readHeirarchyData(&m_RootNode, scene->mRootNode);
		readMissingBones(scene->mAnimations[animIndex], model);
	}

	~Animation() {
		// freed heap allocated strings
		for (unsigned int i = 0; i != m_AnimationsNames.size(); i++)
			free((void*)m_AnimationsNames[i]);
		freeNodeHeirarchy(&m_RootNode);	
	}

	Bone* FindBone(const char* name){
		// idk wtf is going on
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(), [&](const Bone& Bone){
			return (!strcmp(Bone.m_Name, name));
			//return Bone.m_Name == name;
		});

		if (iter == m_Bones.end()) return nullptr;
		else return &(*iter);
	}

private:

	void fillAnimationVector(const aiScene* scene){
		m_AnimationsNames.reserve(scene->mNumAnimations);
		for (unsigned int i=0; i != scene->mNumAnimations; i++){
			const char* Name = strdup(scene->mAnimations[i]->mName.C_Str());
			m_AnimationsNames.emplace_back(Name);
		}
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
				//printf("i found a missing bone %d\n", boneInfoMap[nodeName].id);
				//printf("boneCount %d\n", boneCount);
				//if(boneInfoMap.find(nodeName) == boneInfoMap.end())
				//	printf("FUCKING STRANGEEEEEEEEEEEEEE\n");
			}//else{
				//printf("IT FUCKING FAILLED\n");
			//}

			//for(unsigned int i = 0; i != m_Bones.size(); i++){
				//printf("compare those %s // %s\n", nodeName, m_Bones[i].m_Name.c_str());
				//if (strcmp(nodeName, m_Bones[i].m_Name.c_str()) == 0){
					//printf("strcmp works hhh\n");
				//}
			//}
			
			Bone thaBone = Bone(nodeName, boneInfoMap[nodeName].id, channel);

			m_Bones.emplace_back(thaBone);
		}
		*model->GetBoneCount() = boneCount;
		//printf("BoneinfoMap size is %lu\n", m_BoneInfoMap.size());
		m_BoneInfoMap = boneInfoMap;
	}

	void readHeirarchyData(AssimpNodeData* dest, const aiNode* src){
		if(src){
			// write the node data to the AssimpNodeData
			dest->name = strdup(src->mName.C_Str());
			dest->transformation = assimpMatrix2glm(src->mTransformation);
			dest->children.reserve(src->mNumChildren);

			// load the children to the children vector
			for (unsigned int i = 0; i != src->mNumChildren; i++){
				AssimpNodeData newData;
				readHeirarchyData(&newData, src->mChildren[i]);
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
