#pragma once

#include <assimp/Importer.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>
#include <stdint.h>
#include <immintrin.h>
#include <xmmintrin.h>
#include "animation.h"
#include "bone.h"
#include "model.h"

inline glm::vec4 mul_Mat4Vec4(const float* B, const glm::vec4& v){

	glm::vec4 returnedVal;

	__m128 colum0 = _mm_loadu_ps(&B[0]);
	__m128 colum1 = _mm_loadu_ps(&B[4]);
	__m128 colum2 = _mm_loadu_ps(&B[8]);
	__m128 colum3 = _mm_loadu_ps(&B[12]);

	/*__m128 row = _mm_fmadd_ps(res0, colum0,
					_mm_fmadd_ps(res1, colum1,
					_mm_fmadd_ps(res2, colum2,
					_mm_mul_ps  (res3, colum3))));*/

	__m128 row = _mm_add_ps(_mm_add_ps(
							_mm_mul_ps(_mm_set1_ps(v[0]), colum0),
							_mm_mul_ps(_mm_set1_ps(v[1]), colum1)),
						_mm_add_ps(
							_mm_mul_ps(_mm_set1_ps(v[2]), colum2),
							_mm_mul_ps(_mm_set1_ps(v[3]), colum3)));

	_mm_store_ps(&returnedVal[0], row);

	return returnedVal;
}

inline glm::mat4 mul_Mat4Mat4(const glm::mat4* m1, const glm::mat4* m2) {

	glm::mat4 result;

	for(uint8_t i = 0; i != 4; i++)
		result[i] = mul_Mat4Vec4(&(*m2)[0][0], (*m1)[i]);

	return result;
}

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

		glm::mat4 ParentTimesNode = mul_Mat4Mat4(nodeTransform, parentTransform);

		if(boneInfo)
			mFinalBoneMatrices[boneInfo->id] = mul_Mat4Mat4(&boneInfo->offset, &ParentTimesNode);

		for (unsigned int i = 0; i != node->children.size(); i++)
			CalculateBoneTransform(&node->children[i], &ParentTimesNode);
	}
};
