#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include "animation.h"

class Animator{
	std::vector<glm::mat4> m_FinalBoneMatrices;
	Animation* m_CurrentAnimation;
	float m_CurrentTime;
	float m_DeltaTime;

public:
	Animator(Animation* animation){
		m_CurrentTime = 0.0;
		m_CurrentAnimation = animation;

		m_FinalBoneMatrices.reserve(100);

		for (int i = 0; i < 100; i++)
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
	}

	void UpdateAnimation(float dt){
		m_DeltaTime = dt;
		if (m_CurrentAnimation){
			m_CurrentTime += m_CurrentAnimation->m_TicksPerSecond * dt;
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->m_Duration);
			CalculateBoneTransform(&m_CurrentAnimation->m_RootNode, glm::mat4(1.0f));
		}
	}

	void PlayAnimation(Animation* pAnimation){
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
	}

	void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform){
		glm::mat4 nodeTransform = node->transformation;

		if (m_CurrentAnimation->FindBone(node->name)){
			m_CurrentAnimation->FindBone(node->name)->Update(m_CurrentTime);
			nodeTransform = m_CurrentAnimation->FindBone(node->name)->GetLocalTransform();
		}

		auto boneInfoMap = m_CurrentAnimation->m_BoneInfoMap;
		if (boneInfoMap.find(node->name) != boneInfoMap.end()){
			m_FinalBoneMatrices[boneInfoMap[node->name].id] = parentTransform *
			                                                  nodeTransform *
			                                                  boneInfoMap[node->name].offset;
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], parentTransform * nodeTransform);
	}

	std::vector<glm::mat4> GetFinalBoneMatrices(){
		return m_FinalBoneMatrices;
	}
};
