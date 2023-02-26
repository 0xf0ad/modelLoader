#pragma once

#include <assimp/Importer.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdint.h>
#include <immintrin.h>
#include <xmmintrin.h>
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

	/*void loadMat4tofloatArr(const glm::mat4& src, float* dest){
		for(uint8_t i = 0; i != 4; i++)
			for(uint8_t j = 0; j != 4; j++)
				dest[((i*4)+j)+16] = src[i][j];
	}

	void loadfloatArrtoMat4(float* src, glm::mat4& dest){
		for(uint8_t i = 0; i != 4; i++)
			for(uint8_t j = 0; j != 4; j++)
				dest[i][j] = src[((i*4)+j)+16];
	}*/

	void M4x4_SSE(const float *A, const float *B, float *C){
		__m128 row1 = _mm_loadu_ps(&B[0]);
		__m128 row2 = _mm_loadu_ps(&B[4]);
		__m128 row3 = _mm_loadu_ps(&B[8]);
		__m128 row4 = _mm_loadu_ps(&B[12]);

		for(int i=0; i<4; i++) {
			__m128 brod1 = _mm_set1_ps(A[4*i + 0]);
			__m128 brod2 = _mm_set1_ps(A[4*i + 1]);
			__m128 brod3 = _mm_set1_ps(A[4*i + 2]);
			__m128 brod4 = _mm_set1_ps(A[4*i + 3]);
			__m128 row = _mm_add_ps(_mm_add_ps(
								_mm_mul_ps(brod1, row1),
								_mm_mul_ps(brod2, row2)),
							_mm_add_ps(
								_mm_mul_ps(brod3, row3),
								_mm_mul_ps(brod4, row4)));
			_mm_store_ps(&C[4*i], row);
		}
	}

	glm::vec4 linalgMulMat4Vec4(const glm::mat4& m, const glm::vec4& v) {

		glm::vec4 returnedVal;

		/*alignas(16)*/ const float* B = &m[0][0];

		__m128 colum0 = _mm_loadu_ps(&B[0]);
		__m128 colum1 = _mm_loadu_ps(&B[4]);
		__m128 colum2 = _mm_loadu_ps(&B[8]);
		__m128 colum3 = _mm_loadu_ps(&B[12]);

		float v0 = v[0];
		float v1 = v[1];
		float v2 = v[2];
		float v3 = v[3];

		__m128 res0 = _mm_set1_ps(v0);
		__m128 res1 = _mm_set1_ps(v1);
		__m128 res2 = _mm_set1_ps(v2);
		__m128 res3 = _mm_set1_ps(v3);

		__m128 row = _mm_fmadd_ps(res0, colum0,
						_mm_fmadd_ps(res1, colum1,
						_mm_fmadd_ps(res2, colum2,
						_mm_mul_ps  (res3, colum3))));

		/*__m128 row = _mm_add_ps(_mm_add_ps(
								_mm_mul_ps(res0, colum0),
								_mm_mul_ps(res1, colum1)),
							_mm_add_ps(
								_mm_mul_ps(res2, colum2),
								_mm_mul_ps(res3, colum3)));*/

		_mm_store_ps(&returnedVal[0], row);

		return returnedVal;
	}

	glm::mat4 linalgMulMat4Mat4(const glm::mat4& m1, const glm::mat4& m2) {

		glm::mat4 result;

		for(uint8_t i = 0; i != 4; i++)
			result[i] = linalgMulMat4Vec4(m2, m1[i]);

		return result;
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

		//loadMat4tofloatArr(*parentTransform, f_parentTransform);
		//loadMat4tofloatArr(*nodeTransform, f_nodeTransform);
		//glm::mat4 ParentTimesNode;

		glm::mat4 ParentTimesNode = (*parentTransform) * (*nodeTransform);
		/*glm::mat4 f_ParentTimesNode = linalgMulMat4Mat4(*parentTransform, *nodeTransform);

		if(ParentTimesNode == f_ParentTimesNode)
			printf("ach had l9lawi\n");
		else
			printf("wa l9laaaaaaaaaaaaaaaawi\n");
		
		//M4x4_SSE(glm::value_ptr(*parentTransform) , glm::value_ptr(*nodeTransform), &ParentTimesNode[0][0]);
		
		printf("|%.2f\t%.2f\t%.2f\t%.2f|   |%.2f\t%.2f\t%.2f\t%.2f|   |%.2f\t%.2f\t%.2f\t%.2f| or |%.2f\t%.2f\t%.2f\t%.2f|\n", (*parentTransform)[0][0], (*parentTransform)[0][1], (*parentTransform)[0][2], (*parentTransform)[0][3], (*nodeTransform)[0][0], (*nodeTransform)[0][1], (*nodeTransform)[0][2], (*nodeTransform)[0][3], ParentTimesNode[0][0], ParentTimesNode[0][1], ParentTimesNode[0][2], ParentTimesNode[0][3], f_ParentTimesNode[0][0], f_ParentTimesNode[0][1], f_ParentTimesNode[0][2], f_ParentTimesNode[0][3]);
		printf("|%.2f\t%.2f\t%.2f\t%.2f| * |%.2f\t%.2f\t%.2f\t%.2f| = |%.2f\t%.2f\t%.2f\t%.2f| or |%.2f\t%.2f\t%.2f\t%.2f|\n", (*parentTransform)[1][0], (*parentTransform)[1][1], (*parentTransform)[1][2], (*parentTransform)[1][3], (*nodeTransform)[1][0], (*nodeTransform)[1][1], (*nodeTransform)[1][2], (*nodeTransform)[1][3], ParentTimesNode[1][0], ParentTimesNode[1][1], ParentTimesNode[1][2], ParentTimesNode[1][3], f_ParentTimesNode[1][0], f_ParentTimesNode[1][1], f_ParentTimesNode[1][2], f_ParentTimesNode[1][3]);
		printf("|%.2f\t%.2f\t%.2f\t%.2f|   |%.2f\t%.2f\t%.2f\t%.2f|   |%.2f\t%.2f\t%.2f\t%.2f| or |%.2f\t%.2f\t%.2f\t%.2f|\n", (*parentTransform)[2][0], (*parentTransform)[2][1], (*parentTransform)[2][2], (*parentTransform)[2][3], (*nodeTransform)[2][0], (*nodeTransform)[2][1], (*nodeTransform)[2][2], (*nodeTransform)[2][3], ParentTimesNode[2][0], ParentTimesNode[2][1], ParentTimesNode[2][2], ParentTimesNode[2][3], f_ParentTimesNode[2][0], f_ParentTimesNode[2][1], f_ParentTimesNode[2][2], f_ParentTimesNode[2][3]);
		printf("|%.2f\t%.2f\t%.2f\t%.2f|   |%.2f\t%.2f\t%.2f\t%.2f|   |%.2f\t%.2f\t%.2f\t%.2f| or |%.2f\t%.2f\t%.2f\t%.2f|\n", (*parentTransform)[3][0], (*parentTransform)[3][1], (*parentTransform)[3][2], (*parentTransform)[3][3], (*nodeTransform)[3][0], (*nodeTransform)[3][1], (*nodeTransform)[3][2], (*nodeTransform)[3][3], ParentTimesNode[3][0], ParentTimesNode[3][1], ParentTimesNode[3][2], ParentTimesNode[3][3], f_ParentTimesNode[3][0], f_ParentTimesNode[3][1], f_ParentTimesNode[3][2], f_ParentTimesNode[3][3]);
		printf("\n");*/
		//loadfloatArrtoMat4(f_ParentTimesNode, ParentTimesNode);


		if(boneInfo)
			mFinalBoneMatrices[boneInfo->id] = ParentTimesNode * boneInfo->offset;

		for (unsigned int i = 0; i != node->children.size(); i++)
			CalculateBoneTransform(&node->children[i], &ParentTimesNode);
	}
};
