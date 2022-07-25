#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "bone.h"
#include <functional>
#include "model.h"

struct AssimpNodeData{
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};

class Animation{
public:
	float m_Duration;
	int m_TicksPerSecond;
	std::vector<Bone> m_Bones;
	AssimpNodeData m_RootNode;
	std::map<std::string, BoneInfo> m_BoneInfoMap;

	Animation() = default;

	Animation(const std::string& animationPath, Model* model){
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
		assert(scene && scene->mRootNode);
		m_Duration = scene->mAnimations[0]->mDuration;
		m_TicksPerSecond = scene->mAnimations[0]->mTicksPerSecond;
		aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
		globalTransformation = globalTransformation.Inverse();
		ReadHeirarchyData(m_RootNode, scene->mRootNode);
		ReadMissingBones(scene->mAnimations[0], *model);
	}

	~Animation() { }

	Bone* FindBone(const std::string& name){
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(), [&](const Bone& Bone){
			return Bone.GetBoneName() == name;
		});

		if (iter == m_Bones.end()) return nullptr;
		else return &(*iter);
	}

private:
	void ReadMissingBones(const aiAnimation* animation, Model& model){
		auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
		int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

		//reading channels(bones engaged in an animation and their keyframes)
		for (unsigned int i = 0; i < animation->mNumChannels; i++){
			if (boneInfoMap.find(animation->mChannels[i]->mNodeName.data) == boneInfoMap.end()){
				boneInfoMap[animation->mChannels[i]->mNodeName.data].id = boneCount;
				boneCount++;
			}
			m_Bones.push_back(Bone(animation->mChannels[i]->mNodeName.data,
			                       boneInfoMap[animation->mChannels[i]->mNodeName.data].id,
			                       animation->mChannels[i]));
		}
		m_BoneInfoMap = boneInfoMap;
	}

	void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src){
		assert(src);

		dest.name = src->mName.data;
		dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
		dest.childrenCount = src->mNumChildren;

		for (unsigned int i = 0; i < src->mNumChildren; i++){
			AssimpNodeData newData;
			ReadHeirarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}
};
