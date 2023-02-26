#pragma once

#include <assimp/quaternion.h>
#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <immintrin.h>

static inline glm::mat4 assimpMatrix2glm(const aiMatrix4x4& from){
	glm::mat4 to;
	//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
	to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
	to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
	to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
	to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
	return to;
}

static inline glm::vec3 assimpVec2glm(const aiVector3D& vec) { 
	return glm::vec3(vec.x, vec.y, vec.z); 
}

static inline glm::quat assimpQuat2glm(const aiQuaternion& pOrientation){
	return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
}

inline glm::mat4 mul_Mat4Mat4(const glm::mat4* m1, const glm::mat4* m2) {

	glm::mat4 result;
	__m128 row;
	//const float* mat1 = &(*m2)[0][0];
	//const float* mat2 = &(*m2)[0][0];
	__m128 colum0 = _mm_loadu_ps(&(*m2)[0][0]);
	__m128 colum1 = _mm_loadu_ps(&(*m2)[1][0]);
	__m128 colum2 = _mm_loadu_ps(&(*m2)[2][0]);
	__m128 colum3 = _mm_loadu_ps(&(*m2)[3][0]);

	for(uint8_t i = 0; i != 4; i++){
		row = _mm_add_ps(_mm_add_ps(
							_mm_mul_ps(_mm_set1_ps((*m1)[i][0]), colum0),
							_mm_mul_ps(_mm_set1_ps((*m1)[i][1]), colum1)),
						_mm_add_ps(
							_mm_mul_ps(_mm_set1_ps((*m1)[i][2]), colum2),
							_mm_mul_ps(_mm_set1_ps((*m1)[i][3]), colum3)));
		_mm_store_ps(&result[i][0], row);
	}

	return result;
}
