#pragma once

#include <math.h>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <vector>

#include "bone.h"

#define BICUBIC_INTERPOLATION true

inline const glm::vec3 mix(const glm::vec3& a, const glm::vec3& b, const float t){
#if LINEAR_INTERPOLATION	
	return (a * (1-t)) + (b * t);
#elif OPTIMIZED_LINEAR_INTERPOLATION
	return a + ( t * ( b - a ));
#elif BICUBIC_INTERPOLATION
	const float Tsquare      = t * t;
	const float threeTsquare = 3 * Tsquare;
	const float twoTcube     = 2 * t * Tsquare;
	const glm::vec3 result   = ( a * ( twoTcube - threeTsquare + 1.0f ) ) + ( b * ( threeTsquare - twoTcube ));
	return result;
#endif
}

inline const glm::quat mix(const glm::quat& a, const glm::quat& b, const float t){
	const float Tsquare      = t * t;
	const float threeTsquare = 3 * Tsquare;
	//const float twoTminus3   = 2 * t - 3; 
	const float twoTcube     = 2 * t * Tsquare;
	const glm::quat result   = ( a * ( twoTcube - threeTsquare + 1.0f ) ) + ( b * ( threeTsquare - twoTcube ));
	//const glm::quat result   = ((a * (Tsquare * twoTminus3 + 1.0f)) - (b * Tsquare * twoTminus3));
	return result;
}

inline const glm::quat inverse(const glm::quat& q){
	return glm::quat((q.w), -(q.x), -(q.y), -(q.z));
}

inline const glm::quat log(const glm::quat& q){
	float a = acosf(q.w);
	const float s = sinf(a);

	if(!s){
		return glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
	}else{
		a /= s;
		return glm::quat(0.0f, (q.x * a), (q.y * a), (q.z * a));
	}
}

inline const glm::quat exp(const glm::quat& q){
	const float a = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z);
	float s = sinf(a);
	const float c = cosf(a);

	if (!a){
		return glm::quat( c, 0, 0, 0 );
	}else{
		s /= a;
		return glm::quat(c, q.x * s,  q.y * s,  q.z * s);
	}
}

inline const glm::quat intermediate(const glm::quat& previous, const glm::quat& current, const glm::quat& next){
	const glm::quat inv_quat = inverse(current);
	const glm::quat result = current * exp( (glm::log(next * inv_quat) + glm::log(previous * inv_quat)) * (-0.25f));

	return result;
}

inline const glm::quat squad(const glm::quat& q1, const glm::quat& q2, const glm::quat& s1, const glm::quat& s2, const float t){
	const glm::quat slerp1 = mix(q1, q2, t);
	const glm::quat slerp2 = mix(s1, s2, t);
	const float   t_factor = 2 * t * (1.0f-t);

	const glm::quat result = mix(slerp1, slerp2, t_factor);

	return result;
}

inline const glm::quat all_in_one_squad(const glm::quat& q0, const glm::quat& q1, const glm::quat& q2, const glm::quat& q3, const float t){
	const glm::quat inner1 = intermediate(q0, q1, q2);
	const glm::quat inner2 = intermediate(q1, q2, q3);

	const glm::quat result = squad(q1, q2, inner1, inner2, t);

	return result;
}

inline const glm::quat nlerp(const glm::quat& a, const glm::quat& b, float t){
	glm::quat tmp = a + t* (b - a);
	return glm::normalize(tmp);
}

inline const glm::quat squad_from_data(const std::vector<KeyRotation>& rotations, const uint32_t current_index, const float scalarFactor){
	const glm::quat* q_prev = &rotations[current_index-1].orientation;
	const glm::quat* q_curr = &rotations[ current_index ].orientation;
	const glm::quat* q_next = &rotations[current_index+1].orientation;
	const glm::quat* q_nex2 = &rotations[current_index+2].orientation;

	//const float t_fract = scalarFactor - (int)scalarFactor;

	const glm::quat result = all_in_one_squad(*q_prev, *q_curr, *q_next, *q_nex2, scalarFactor);

	return result;
}

inline const glm::quat SQUAD(const glm::quat& q0, const glm::quat& q1, const glm::quat& q2, const glm::quat& q3, float lambda){
	// Modify quaternions for shortest path
	const glm::quat q01 = glm::length(q1 - q0) < glm::length(q1 + q0) ? q0 : inverse(q0);
	const glm::quat q21 = glm::length(q1 - q2) < glm::length(q1 + q2) ? q2 : inverse(q2);
	const glm::quat q31 = glm::length(q21- q3) < glm::length(q21+ q3) ? q3 : inverse(q3);

	// Calculate helper quaternions
	glm::quat a = intermediate(q01, q1, q21);
	glm::quat b = intermediate(q1, q21, q31);

	return glm::slerp(glm::slerp(q1, q21, lambda), glm::slerp(a, b, lambda), 2.0f * lambda * (1.0f - lambda));
}

inline glm::mat4 mul_Mat4Mat4(const glm::mat4& m1, const glm::mat4& m2) {

	glm::mat4 result;
	__m128 row;
	__m128 colum0 = _mm_loadu_ps(&(m1)[0][0]);
	__m128 colum1 = _mm_loadu_ps(&(m1)[1][0]);
	__m128 colum2 = _mm_loadu_ps(&(m1)[2][0]);
	__m128 colum3 = _mm_loadu_ps(&(m1)[3][0]);

	for(uint8_t i = 0; i != 4; i++){
		row = _mm_add_ps(_mm_add_ps(
							_mm_mul_ps(_mm_set1_ps((m2)[i][0]), colum0),
							_mm_mul_ps(_mm_set1_ps((m2)[i][1]), colum1)),
						_mm_add_ps(
							_mm_mul_ps(_mm_set1_ps((m2)[i][2]), colum2),
							_mm_mul_ps(_mm_set1_ps((m2)[i][3]), colum3)));
	
		_mm_store_ps(&result[i][0], row);
	}

	return result;
}
