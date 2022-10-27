#pragma once

#include <assimp/anim.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <unordered_map>

#include "bone.h"
#include "model.h"

struct AssimpNodeData{
	glm::mat4 transformation;
	std::string name;
	std::vector<AssimpNodeData> children;
};

class Animation{
public:
	float m_Duration;
	int m_TicksPerSecond;
	std::vector<Bone> m_Bones;
	//std::vector<aiAnimation> m_Animations;
	//std::vector<char> m_AnimationsNames;
	AssimpNodeData m_RootNode;
	std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;
	//const char* animeNames;

	Animation() = default;

	Animation(const char* animationPath, Model* model, unsigned char animIndex = 0){
		// import the model from the given path
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath,
			aiProcess_Triangulate | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices);
		assert(scene && scene->mRootNode);
		
		// reserve the number of bones to the hashed map and the bones vector(dynamic array)
		m_Bones.reserve((*model->GetBoneCount())-1);
		m_BoneInfoMap.reserve((*model->GetBoneCount())-1);
		fillAnimationVector(scene);
		printf("that mesh contain %i animations\n", scene->mNumAnimations);
		
		m_Duration = scene->mAnimations[animIndex]->mDuration;
		m_TicksPerSecond = scene->mAnimations[animIndex]->mTicksPerSecond;
		ReadHeirarchyData(&m_RootNode, scene->mRootNode);
		ReadMissingBones(scene->mAnimations[animIndex], model);
		importer.FreeScene();
	}

	//~Animation() { }

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
		//m_Animations.reserve(scene->mNumAnimations);
		//m_AnimationsNames.reserve(scene->mNumAnimations);
		//animeNames = (char*) calloc(scene->mNumAnimations, sizeof(scene->mAnimations[0]->mName.data));
		//for (unsigned int i=0; i != scene->mNumAnimations; i++)
			//m_Animations.emplace_back(scene->mAnimations[i]);
			//printf("this animation is %s\n", m_Animations[i].mName.C_Str());

		//for(unsigned int i = 0; i != m_Animations.size(); i++)
			//m_AnimationsNames.emplace_back(m_Animations[i].mName.data);
			//memcpy((void*)(animeNames + i * 1024), &m_Animations[i].mName.data, sizeof(char)*1024);
	}

	void ReadMissingBones(const aiAnimation* animation, Model* model){
		//getting m_BoneInfoMap and boneCount from Model class
		std::unordered_map<std::string, BoneInfo>& boneInfoMap = model->GetBoneInfoMap();
		unsigned char* boneCount = model->GetBoneCount();

		const aiNodeAnim* channel;
		const char* nodeName;

		//reading channels(bones engaged in an animation and their keyframes)
		for (unsigned int i = 0; i != animation->mNumChannels; i++){

			channel = animation->mChannels[i];
			nodeName = channel->mNodeName.C_Str();

			if (boneInfoMap.find(nodeName) == boneInfoMap.end())
				boneInfoMap[nodeName].id = *(boneCount)++;

			m_Bones.emplace_back(Bone(nodeName,
			                       boneInfoMap[nodeName].id,
			                          channel));
		}
		m_BoneInfoMap = boneInfoMap;
	}

	void ReadHeirarchyData(AssimpNodeData* dest, const aiNode* src){
		assert(src);

		dest->name = src->mName.data;
		dest->transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);

		dest->children.reserve(src->mNumChildren);

		for (unsigned int i = 0; i != src->mNumChildren; i++){
			AssimpNodeData newData;
			ReadHeirarchyData(&newData, src->mChildren[i]);
			dest->children.emplace_back(newData);
		}
	}
};
