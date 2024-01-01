#pragma once

#include <assimp/Importer.hpp>
#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>
#include <stdint.h>
#include "mymath.h"
#include "animation.h"
#include "bone.h"
#include "model.h"


class Animator{
public:
	glm::mat4 mFinalBoneMatrices[256];
	glm::mat4 mParentBoneMatrices[256];
	Animation* mCurrentAnimation;
	float mCurrentTime = 0.0f;
	uint8_t boneNumber;
	
	Animator(Animation* animation){
		boneNumber = animation->boneNum + 1;
		printf("boneNum %d\n", boneNumber);
		mCurrentAnimation = animation;
	}

	void UpdateAnimation(float dt, float speed){
		if (mCurrentAnimation){
			mCurrentTime += mCurrentAnimation->mTicksPerSecond * dt * speed;
			mCurrentTime += mCurrentAnimation->mDuration * (mCurrentTime < 0);
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
		glm::mat4 ParentTimesNode;
		const BoneInfo* boneInfo = node->bone;

		if(boneInfo){
			uint8_t boneindex = boneInfo->id;
			Bone* bone = mCurrentAnimation->BonesArray[boneindex];
			bone->Update(mCurrentTime);
			mParentBoneMatrices[boneindex] = *parentTransform;
			ParentTimesNode = mul_Mat4Mat4(mParentBoneMatrices[boneindex], bone->localTransform);
			mFinalBoneMatrices[boneindex] = mul_Mat4Mat4(ParentTimesNode, boneInfo->offset);
		} else {
			ParentTimesNode = mul_Mat4Mat4(*parentTransform, node->transformation);
		}			

		for (uint32_t i = 0; i != node->children.size(); i++)
			CalculateBoneTransform(&node->children[i], &ParentTimesNode);
	}
};
