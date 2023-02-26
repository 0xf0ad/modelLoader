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

struct MATRIX {
    union {
		glm::mat4 g;
        float  f[4][4];
        __m128 m[4];
        __m256 n[2];
    };
};

MATRIX myMultiply(MATRIX M1, MATRIX M2) {
    // Perform a 4x4 matrix multiply by a 4x4 matrix 
    // Be sure to run in 64 bit mode and set right flags
    // Properties, C/C++, Enable Enhanced Instruction, /arch:AVX 
    // Having MATRIX on a 32 byte bundry does help performance
    MATRIX mResult;
    __m256 a0, a1, b0, b1;
    __m256 c0, c1, c2, c3, c4, c5, c6, c7;
    __m256 t0, t1, u0, u1;

    t0 = M1.n[0];                                                   // t0 = a00, a01, a02, a03, a10, a11, a12, a13
    t1 = M1.n[1];                                                   // t1 = a20, a21, a22, a23, a30, a31, a32, a33
    u0 = M2.n[0];                                                   // u0 = b00, b01, b02, b03, b10, b11, b12, b13
    u1 = M2.n[1];                                                   // u1 = b20, b21, b22, b23, b30, b31, b32, b33

    a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(0, 0, 0, 0));        // a0 = a00, a00, a00, a00, a10, a10, a10, a10
    a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(0, 0, 0, 0));        // a1 = a20, a20, a20, a20, a30, a30, a30, a30
    b0 = _mm256_permute2f128_ps(u0, u0, 0x00);                      // b0 = b00, b01, b02, b03, b00, b01, b02, b03  
    c0 = _mm256_mul_ps(a0, b0);                                     // c0 = a00*b00  a00*b01  a00*b02  a00*b03  a10*b00  a10*b01  a10*b02  a10*b03
    c1 = _mm256_mul_ps(a1, b0);                                     // c1 = a20*b00  a20*b01  a20*b02  a20*b03  a30*b00  a30*b01  a30*b02  a30*b03

    a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(1, 1, 1, 1));        // a0 = a01, a01, a01, a01, a11, a11, a11, a11
    a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(1, 1, 1, 1));        // a1 = a21, a21, a21, a21, a31, a31, a31, a31
    b0 = _mm256_permute2f128_ps(u0, u0, 0x11);                      // b0 = b10, b11, b12, b13, b10, b11, b12, b13
    c2 = _mm256_mul_ps(a0, b0);                                     // c2 = a01*b10  a01*b11  a01*b12  a01*b13  a11*b10  a11*b11  a11*b12  a11*b13
    c3 = _mm256_mul_ps(a1, b0);                                     // c3 = a21*b10  a21*b11  a21*b12  a21*b13  a31*b10  a31*b11  a31*b12  a31*b13

    a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(2, 2, 2, 2));        // a0 = a02, a02, a02, a02, a12, a12, a12, a12
    a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(2, 2, 2, 2));        // a1 = a22, a22, a22, a22, a32, a32, a32, a32
    b1 = _mm256_permute2f128_ps(u1, u1, 0x00);                      // b0 = b20, b21, b22, b23, b20, b21, b22, b23
    c4 = _mm256_mul_ps(a0, b1);                                     // c4 = a02*b20  a02*b21  a02*b22  a02*b23  a12*b20  a12*b21  a12*b22  a12*b23
    c5 = _mm256_mul_ps(a1, b1);                                     // c5 = a22*b20  a22*b21  a22*b22  a22*b23  a32*b20  a32*b21  a32*b22  a32*b23

    a0 = _mm256_shuffle_ps(t0, t0, _MM_SHUFFLE(3, 3, 3, 3));        // a0 = a03, a03, a03, a03, a13, a13, a13, a13
    a1 = _mm256_shuffle_ps(t1, t1, _MM_SHUFFLE(3, 3, 3, 3));        // a1 = a23, a23, a23, a23, a33, a33, a33, a33
    b1 = _mm256_permute2f128_ps(u1, u1, 0x11);                      // b0 = b30, b31, b32, b33, b30, b31, b32, b33
    c6 = _mm256_mul_ps(a0, b1);                                     // c6 = a03*b30  a03*b31  a03*b32  a03*b33  a13*b30  a13*b31  a13*b32  a13*b33
    c7 = _mm256_mul_ps(a1, b1);                                     // c7 = a23*b30  a23*b31  a23*b32  a23*b33  a33*b30  a33*b31  a33*b32  a33*b33

    c0 = _mm256_add_ps(c0, c2);                                     // c0 = c0 + c2 (two terms, first two rows)
    c4 = _mm256_add_ps(c4, c6);                                     // c4 = c4 + c6 (the other two terms, first two rows)
    c1 = _mm256_add_ps(c1, c3);                                     // c1 = c1 + c3 (two terms, second two rows)
    c5 = _mm256_add_ps(c5, c7);                                     // c5 = c5 + c7 (the other two terms, second two rose)

                                                                    // Finally complete addition of all four terms and return the results
    mResult.n[0] = _mm256_add_ps(c0, c4);       // n0 = a00*b00+a01*b10+a02*b20+a03*b30  a00*b01+a01*b11+a02*b21+a03*b31  a00*b02+a01*b12+a02*b22+a03*b32  a00*b03+a01*b13+a02*b23+a03*b33
                                                //      a10*b00+a11*b10+a12*b20+a13*b30  a10*b01+a11*b11+a12*b21+a13*b31  a10*b02+a11*b12+a12*b22+a13*b32  a10*b03+a11*b13+a12*b23+a13*b33
    mResult.n[1] = _mm256_add_ps(c1, c5);       // n1 = a20*b00+a21*b10+a22*b20+a23*b30  a20*b01+a21*b11+a22*b21+a23*b31  a20*b02+a21*b12+a22*b22+a23*b32  a20*b03+a21*b13+a22*b23+a23*b33
                                                //      a30*b00+a31*b10+a32*b20+a33*b30  a30*b01+a31*b11+a32*b21+a33*b31  a30*b02+a31*b12+a32*b22+a33*b32  a30*b03+a31*b13+a32*b23+a33*b33
	return mResult;
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
		/*MATRIX M_parentTransform, M_nodeTransform;
		M_parentTransform.g = (*parentTransform);
		M_nodeTransform.g = (*nodeTransform);*/

		glm::mat4 ParentTimesNode = (*parentTransform) * (*nodeTransform);
		//glm::mat4 ParentTimesNode = myMultiply(M_parentTransform, M_nodeTransform).g;
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
