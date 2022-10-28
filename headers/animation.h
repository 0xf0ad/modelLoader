#pragma once

#include <assimp/anim.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <string.h>
#include <unordered_map>

#include "bone.h"
#include "model.h"

//#define AI_CONFIG_PP_RVC_FLAGS

struct AssimpNodeData{
	glm::mat4 transformation;
	std::string name;
	std::vector<AssimpNodeData> children;
};

class Animation{
public:
	float m_Duration;
	int m_TicksPerSecond;
	std::vector<Bone> m_Bones;		// a vector of bones(obviously) 
	std::vector<const char*> m_AnimationsNames;		// a vector of animation names allocated on the heap
	AssimpNodeData m_RootNode;
	std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;	// a hash table of bones and their names (i hate the fact i have to search that table to find bone names on the game loop) 

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
		m_Bones.reserve((*model->GetBoneCount())-1);
		m_BoneInfoMap.reserve((*model->GetBoneCount())-1);
		fillAnimationVector(scene);
		//printf("that mesh contain %i animations\n", scene->mNumAnimations);

		m_Duration = scene->mAnimations[animIndex]->mDuration;
		m_TicksPerSecond = scene->mAnimations[animIndex]->mTicksPerSecond;
		ReadHeirarchyData(&m_RootNode, scene->mRootNode);
		ReadMissingBones(scene->mAnimations[animIndex], model);
		importer.FreeScene();
	}

	Animation(const aiScene* scene, Model* model, unsigned char animIndex = 0){
		// reserve the number of bones to the hashed map and the bones vector(dynamic array)
		m_Bones.reserve((*model->GetBoneCount())-1);		// get the bone nuber by subtracting 1 from the boneCount
		m_BoneInfoMap.reserve((*model->GetBoneCount())-1);	// same here
		fillAnimationVector(scene);
		//printf("that mesh contain %i animations\n", scene->mNumAnimations);

		m_Duration = scene->mAnimations[animIndex]->mDuration;
		m_TicksPerSecond = scene->mAnimations[animIndex]->mTicksPerSecond;
		ReadHeirarchyData(&m_RootNode, scene->mRootNode);
		ReadMissingBones(scene->mAnimations[animIndex], model);
	}

	~Animation() {
		// freed heap allocated strings
		for (unsigned int i = 0; i != m_AnimationsNames.size(); i++)
			free((void*)m_AnimationsNames[i]);
	}

	Bone* FindBone(const std::string& name){
		// idk wtf is going on
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(), [&](const Bone& Bone){
			return Bone.m_Name == name;
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

	void ReadMissingBones(const aiAnimation* animation, Model* model){
		// getting m_BoneInfoMap and boneCount from Model class
		std::unordered_map<std::string, BoneInfo>& boneInfoMap = model->GetBoneInfoMap();
		unsigned char* boneCount = model->GetBoneCount();

		// reading channels(bones engaged in an animation and their keyframes)
		for (unsigned int i = 0; i != animation->mNumChannels; i++){

			const aiNodeAnim* channel = animation->mChannels[i];
			const char* nodeName = channel->mNodeName.C_Str();

			if (boneInfoMap.find(nodeName) == boneInfoMap.end())
				boneInfoMap[nodeName].id = *(boneCount)++;

			Bone thaBone = Bone(nodeName, boneInfoMap[nodeName].id, channel);

			m_Bones.emplace_back(thaBone);
		}
		m_BoneInfoMap = boneInfoMap;
	}

	void ReadHeirarchyData(AssimpNodeData* dest, const aiNode* src){
		if(src){
			// write the node data to the AssimpNodeData
			dest->name = src->mName.data;
			dest->transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
			dest->children.reserve(src->mNumChildren);

			// load the children to the children vector
			for (unsigned int i = 0; i != src->mNumChildren; i++){
				AssimpNodeData newData;
				ReadHeirarchyData(&newData, src->mChildren[i]);
				dest->children.emplace_back(newData);
			}
		}else{
			fprintf(stderr, "couldnot read the animation heirarchy\n");
			return;
		}
	}
};
