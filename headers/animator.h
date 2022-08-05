#pragma once

#include <assimp/Importer.hpp>
#include "animation.h"

class Animator{
public:
	glm::mat4 m_FinalBoneMatrices[100] = { glm::mat4(1.0f) };
	Animation* m_CurrentAnimation;
	float m_CurrentTime = 0.0f;

	Animator(Animation* animation){
		m_CurrentAnimation = animation;
	}

	void UpdateAnimation(float dt){
		if (m_CurrentAnimation){
			m_CurrentTime += m_CurrentAnimation->m_TicksPerSecond * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->m_Duration);
			CalculateBoneTransform(&(m_CurrentAnimation->m_RootNode), glm::mat4(1.0f));
		}
	}

	void PlayAnimation(Animation* pAnimation){
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
	}

	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform){
		glm::mat4 nodeTransform = node->transformation;
		glm::mat4 boneScale(1.0f);
		glm::mat4 boneRotation(1.0f);
		glm::mat4 bonePosition(1.0f);
		Bone* bone = m_CurrentAnimation->FindBone(node->name);

		if (bone){
			//bone->Update(m_CurrentTime);
			nodeTransform = glm::mat4(1.0f);
			boneScale     = bone->InterpolateScaling (m_CurrentTime);
			boneRotation  = bone->InterpolateRotation(m_CurrentTime);
			bonePosition  = bone->InterpolatePosition(m_CurrentTime);
			//nodeTransform = bone->GetLocalTransform();
		}

		std::unordered_map<std::string, BoneInfo> boneInfoMap = m_CurrentAnimation->m_BoneInfoMap;
		glm::mat4 mutip = parentTransform * bonePosition * boneRotation * boneScale * nodeTransform;
		
		if (boneInfoMap.find(node->name) != boneInfoMap.end()){
			m_FinalBoneMatrices[boneInfoMap[node->name].id] = mutip * boneInfoMap[node->name].offset;
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], mutip);
	}
};
