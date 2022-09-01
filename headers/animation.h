#pragma once

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
	AssimpNodeData m_RootNode;
	std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;

	Animation() = default;

	Animation(const char* animationPath, Model* model){
		Assimp::Importer importer;
		m_Bones.reserve(100);
		m_BoneInfoMap.reserve(100);
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
		assert(scene && scene->mRootNode);
		m_Duration = scene->mAnimations[0]->mDuration;
		m_TicksPerSecond = scene->mAnimations[0]->mTicksPerSecond;
		ReadHeirarchyData(&m_RootNode, scene->mRootNode);
		ReadMissingBones(scene->mAnimations[0], model);
	}

	~Animation() { }

	Bone* FindBone(const std::string& name){
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(), [&](const Bone& Bone){
			return Bone.m_Name == name;
		});

		if (iter == m_Bones.end()) return nullptr;
		else return &(*iter);
	}

private:
	void ReadMissingBones(const aiAnimation* animation, Model* model){
		//getting m_BoneInfoMap and boneCount from Model class
		std::unordered_map<std::string, BoneInfo>& boneInfoMap = model->GetBoneInfoMap();
		unsigned char* boneCount = model->GetBoneCount();

		//reading channels(bones engaged in an animation and their keyframes)
		for (unsigned int i = 0; i < animation->mNumChannels; i++){

			if (boneInfoMap.find(animation->mChannels[i]->mNodeName.data) == boneInfoMap.end())
				boneInfoMap[animation->mChannels[i]->mNodeName.data].id = *(boneCount)++;

			m_Bones.emplace_back(Bone(animation->mChannels[i]->mNodeName.data,
			                       boneInfoMap[animation->mChannels[i]->mNodeName.data].id,
			                  animation->mChannels[i]));
		}
		m_BoneInfoMap = boneInfoMap;
	}

	void ReadHeirarchyData(AssimpNodeData* dest, const aiNode* src){
		assert(src);

		dest->name = src->mName.data;
		dest->transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);

		dest->children.reserve(src->mNumChildren);

		for (unsigned int i = 0; i < src->mNumChildren; i++){
			AssimpNodeData newData;
			ReadHeirarchyData(&newData, src->mChildren[i]);
			dest->children.emplace_back(newData);
		}
	}
};
