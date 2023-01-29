#pragma once

#include <assimp/Importer.hpp>
#include <glm/fwd.hpp>
#include <vector>
#include "animation.h"
#include "bone.h"
#include "model.h"

class Animator{
public:
	glm::mat4 mFinalBoneMatrices[256];
	Animation* mCurrentAnimation;
	float mCurrentTime = 0.0f;

	Animator(Animation* animation){
		mCurrentAnimation = animation;
	}

	void UpdateAnimation(float dt){
		if (mCurrentAnimation){
			mCurrentTime += mCurrentAnimation->mTicksPerSecond * dt;
			mCurrentTime = fmodf(mCurrentTime, mCurrentAnimation->mDuration);
			glm::mat4 rootNode(1.0f);
			CalculateBoneTransform(&mCurrentAnimation->mRootNode, &rootNode);
		}
	}

	void PlayAnimation(Animation* pAnimation){
		mCurrentAnimation = pAnimation;
		mCurrentTime = 0.0f;
	}

	void CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4* parentTransform){
		const glm::mat4* nodeTransform;
		const BoneInfo* boneInfo = node->bone;

		if(boneInfo){
			Bone* bone = mCurrentAnimation->BonesArray[boneInfo->id];
			bone->Update(mCurrentTime);
			nodeTransform = bone->GetLocalTransform();
		}else{
			nodeTransform = &node->transformation;
		}

		glm::mat4 ParentTimesNode = (*parentTransform) * (*nodeTransform);

		if(boneInfo)
			mFinalBoneMatrices[boneInfo->id] = ParentTimesNode * boneInfo->offset;

		for (unsigned int i = 0; i != node->children.size(); i++)
			CalculateBoneTransform(&node->children[i], &ParentTimesNode);
	}
};
