#pragma once

#include <assimp/Importer.hpp>
#include <glm/fwd.hpp>
#include <string>
#include "animation.h"
#include "model.h"

class Animator{
public:
	glm::mat4 m_FinalBoneMatrices[255] = { glm::mat4(1.0f) };
	Animation* m_CurrentAnimation;
	float m_CurrentTime = 0.0f;

	Animator(Animation* animation){
		m_CurrentAnimation = animation;
	}

	void UpdateAnimation(float dt){
		if (m_CurrentAnimation){
			m_CurrentTime += m_CurrentAnimation->m_TicksPerSecond * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->m_Duration);
			glm::mat4 rootNode(1.0f);
			CalculateBoneTransform(&m_CurrentAnimation->m_RootNode, &rootNode);
		}
	}

	void PlayAnimation(Animation* pAnimation){
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
	}

	void CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4* parentTransform){
		const glm::mat4* nodeTransform;
		const char* index = node->name;
		Bone* bone = m_CurrentAnimation->FindBone(index);

		if (bone){
			bone->Update(m_CurrentTime);
			nodeTransform = bone->GetLocalTransform();
		}else{
			nodeTransform = &node->transformation;
		}

		//std::unordered_map<const char*, BoneInfo, strHash, strequal_to>& boneInfoMap = m_CurrentAnimation->m_BoneInfoMap;
		std::unordered_map<std::string, BoneInfo, stdstrHash, stdstrequal_to>& boneInfoMap = m_CurrentAnimation->m_BoneInfoMap;
		glm::mat4 ParentTimesNode = (*parentTransform) * (*nodeTransform);
		
		if (boneInfoMap.find(index) != boneInfoMap.end()){
			BoneInfo* thaBone = &boneInfoMap[index];
			m_FinalBoneMatrices[thaBone->id] = ParentTimesNode * thaBone->offset;
		}

		for (unsigned int i = 0; i != node->children.size(); i++)
			CalculateBoneTransform(&node->children[i], &ParentTimesNode);
	}
};
