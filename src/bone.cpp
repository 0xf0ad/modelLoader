#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../headers/bone.h"
#include <glm/detail/type_quat.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_exponential.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/gtx/quaternion.hpp>

#include <chrono>
#include <memory>

#define BINARYSEARCHINDEX true

static uint numPositions, numRotations, numScalings;
extern bool Q_squad;

static glm::mat4 localTransform;

#define BICUBIC_INTERPOLATION true

// reads keyframes from aiNodeAnim
Bone::Bone(const char* name, int ID, const aiNodeAnim* channel){
	mName          = strdup(name);
	mID            = ID;
	numPositions   = channel->mNumPositionKeys;
	numScalings    = channel->mNumScalingKeys;
	numRotations   = channel->mNumRotationKeys;
	localTransform = glm::mat4(1.0f);

	mPositions = (KeyPosition*) malloc(sizeof(KeyPosition[numPositions]));
	mRotations = (KeyRotation*) malloc(sizeof(KeyRotation[numRotations]));
	mScales    = (KeyScale*)    malloc(sizeof(KeyScale[numScalings]));

	for (uint i = 0; i != numPositions; i++){
		mPositions[i].position = assimpVec2glm(channel->mPositionKeys[i].mValue);
		mPositions[i].timeStamp = channel->mPositionKeys[i].mTime;
	}

	for (uint i = 0; i != numRotations; i++){
		mRotations[i].orientation = assimpQuat2glm(channel->mRotationKeys[i].mValue);
		mRotations[i].timeStamp = channel->mRotationKeys[i].mTime;
	}

	for (uint i = 0; i != numScalings; i++){
		mScales[i].scale = assimpVec2glm(channel->mScalingKeys[i].mValue);
		mScales[i].timeStamp = channel->mScalingKeys[i].mTime;
	}
}

Bone::~Bone(){
	//printf("i am a bone and my name is %s\tI live in adress %p\n", mName, mName);

/*	if(mName){
		free((void*)mName);
		mName = NULL;
	}else
		printf("L7WA\n");
	free((void*)mName);*/
	//assert(false);

//	if(mPositions)
//		free(mPositions);
//	if(mRotations)
//		free(mRotations);
//	if(mScales)
//		free(mScales);
}

// Gets normalized value for Lerp & Slerp
inline float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime){
	return (animationTime - lastTimeStamp) / (nextTimeStamp - lastTimeStamp);
}

// math shit
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

inline const glm::quat inverse (const glm::quat& q){
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

inline const glm::quat squad_from_data(const std::vector<KeyRotation>& rotations, const unsigned int current_index, const float scalarFactor){
	const glm::quat* q_prev = &rotations[current_index-1].orientation;
	const glm::quat* q_curr = &rotations[ current_index ].orientation;
	const glm::quat* q_next = &rotations[current_index+1].orientation;
	const glm::quat* q_nex2 = &rotations[current_index+2].orientation;

	//const float t_fract = scalarFactor - (int)scalarFactor;

	const glm::quat result = all_in_one_squad(*q_prev, *q_curr, *q_next, *q_nex2, scalarFactor);

	return result;
}

inline uint binarySearchPosIndex(const float animationTime, const KeyPosition* positions, uint offset, uint end){
	if(offset >= end)
		return positions[offset].timeStamp <= animationTime ? offset+1 : offset;

	int mid = (offset + end) >> 1;
	if(positions[mid].timeStamp < animationTime)
		return binarySearchPosIndex(animationTime, positions, mid+1, end);

	else
		return binarySearchPosIndex(animationTime, positions, offset, mid);
}

inline uint binarySearchRotIndex(const float animationTime, const KeyRotation* rotations, uint offset, uint end){
	if(offset >= end)
		return rotations[offset].timeStamp <= animationTime ? offset+1 : offset;

	int mid = (offset + end) >> 1;
	if(rotations[mid].timeStamp < animationTime)
		return binarySearchRotIndex(animationTime, rotations, mid+1, end);

	else
		return binarySearchRotIndex(animationTime, rotations, offset, mid);
}

inline uint binarySearchScaIndex(const float animationTime, const KeyScale* scalings, uint offset, uint end){
	if(offset >= end)
		return scalings[offset].timeStamp <= animationTime ? offset+1 : offset;

	int mid = (offset + end) >> 1;
	if(scalings[mid].timeStamp < animationTime)
		return binarySearchScaIndex(animationTime, scalings, mid+1, end);

	else
		return binarySearchScaIndex(animationTime, scalings, offset, mid);
}


// Gets the current index on mKeyPositions to interpolate to based on
// the current animation time
inline uint GetPositionIndex(const float animationTime, const KeyPosition* m_Positions){
#if BINARYSEARCHINDEX
		return binarySearchPosIndex(animationTime, m_Positions, 0, numPositions) - 1;
#else
		for (int i = 0; i != (numPositions - 1); i++){
			if (animationTime < m_Positions[i+1].timeStamp){
				return i;
			}
		}
		assert(false);
#endif
}

// Gets the current index on mKeyRotations to interpolate to based on the
// current animation time
inline uint GetRotationIndex(const float animationTime, const KeyRotation* rotations){
#if BINARYSEARCHINDEX
		return binarySearchRotIndex(animationTime, rotations, 0, numRotations) - 1;
#else
	for (int i = 0; i != (numRotations - 1); i++){
		if (animationTime < rotations[i+1].timeStamp){
			return i;
		}
	}
	assert(false);
#endif
}

// Gets the current index on mKeyScalings to interpolate to based on the
// current animation time
inline uint GetScaleIndex(const float animationTime, const KeyScale* scales){
#if BINARYSEARCHINDEX
	return binarySearchScaIndex(animationTime, scales, 0, numScalings) - 1;
#else
	for (int i = 0; i != (numScalings - 1); i++){
		if (animationTime < scales[i+1].timeStamp){
			return i;
		}
	}
	assert(false);
#endif
}

const glm::quat SQUAD(const glm::quat& q0, const glm::quat& q1, const glm::quat& q2, const glm::quat& q3, float lambda){
	// Modify quaternions for shortest path
	const glm::quat q01 = glm::length(q1 - q0) < glm::length(q1 + q0) ? q0 : inverse(q0);
	const glm::quat q21 = glm::length(q1 - q2) < glm::length(q1 + q2) ? q2 : inverse(q2);
	const glm::quat q31 = glm::length(q21- q3) < glm::length(q21+ q3) ? q3 : inverse(q3);

	// Calculate helper quaternions
	glm::quat a = intermediate(q01, q1, q21);
	glm::quat b = intermediate(q1, q21, q31);

	return glm::slerp(glm::slerp(q1, q21, lambda), glm::slerp(a, b, lambda), 2.0f * lambda * (1.0f - lambda));
}

inline auto startTime(){
	return std::chrono::high_resolution_clock::now();
}
inline long long timerStop(std::chrono::time_point<std::chrono::high_resolution_clock>& startTime){
	auto endTime = std::chrono::high_resolution_clock::now();
	auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime).time_since_epoch().count();
	auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();

	return end - start;
}

// figures out which position keys to interpolate b/w and performs the interpolation 
// and returns the translation matrix
inline const glm::mat4 InterpolatePosition(const float animationTime, const KeyPosition* positions){
	if (numPositions == 1){
		return glm::translate(glm::mat4(1.0f), positions[0].position);
	}

	uint index = GetPositionIndex(animationTime, positions);
	float scaleFactor = GetScaleFactor(positions[index].timeStamp, positions[index+1].timeStamp, animationTime);

	return glm::translate(glm::mat4(1.0f),
		mix(positions[index].position, positions[index + 1].position, scaleFactor));
}

// figures out which rotations keys to interpolate b/w and performs the interpolation 
// and returns the rotation matrix
inline const glm::mat4 InterpolateRotation(const float animationTime, const KeyRotation* rotations){

	if (numRotations == 1){
		return glm::toMat4(glm::normalize(rotations[0].orientation));
	}

	uint index = GetRotationIndex(animationTime, rotations);
	float scalarFactor = GetScaleFactor(rotations[index].timeStamp, rotations[index+1].timeStamp, animationTime);

	glm::quat O_returned;
	glm::quat N_returned;

	/*
	 * WHY DOES CUBIC INTERPOLATION SEEMS LIKE THE LINEAR ONE IT SHOULD NOT BE THAT WAY
	 */

	 /*
	  * FUCK IT I AM ONLY GOING TO USE THE SLERP, SQUAD IS JUST UNNECESSARY HEADACH
	  */

	if(Q_squad){
		N_returned = glm::slerp(rotations[index].orientation, rotations[index+1].orientation, scalarFactor);
		/*O_returned = mix(N_returned , mix(glm::intermediate(m_Rotations[index-1].orientation, m_Rotations[index].orientation, m_Rotations[index+1].orientation)
		                                   ,glm::intermediate(m_Rotations[index].orientation, m_Rotations[index+1].orientation, m_Rotations[index+2].orientation), scalarFactor)
						 ,2*scalarFactor*(1-scalarFactor));*/

		//O_returned = squad_from_data(m_Rotations, index, scalarFactor);
		return glm::toMat4(N_returned);
	}else{
		/*O_returned = glm::squad(m_Rotations[index].orientation, m_Rotations[index+1].orientation,
			    //   m_Rotations[index].orientation * glm::exp((glm::log(q * m_Rotations[index+1].orientation) + glm::log(q * m_Rotations[index-1].orientation)) * -0.25f),
				//   m_Rotations[index+1].orientation * glm::exp((glm::log(q * m_Rotations[index].orientation) + glm::log(q * m_Rotations[index+2].orientation)) * -0.25f),
			       intermediate(m_Rotations[index-1].orientation, m_Rotations[index].orientation, m_Rotations[index+1].orientation),
				   intermediate(m_Rotations[index].orientation, m_Rotations[index+1].orientation, m_Rotations[index+2].orientation),
				   scalarFactor);*/
		O_returned = all_in_one_squad(rotations[index-1].orientation, rotations[index].orientation, rotations[index+1].orientation, rotations[index+2].orientation, scalarFactor);
		//O_returned = nlerp(m_Rotations[index].orientation, m_Rotations[index+1].orientation, scalarFactor);
		/*O_returned.x = glm::smoothstep(rotations[index].orientation.x, rotations[index+1].orientation.x, scalarFactor);
		O_returned.y = glm::smoothstep(rotations[index].orientation.y, rotations[index+1].orientation.y, scalarFactor);
		O_returned.z = glm::smoothstep(rotations[index].orientation.z, rotations[index+1].orientation.z, scalarFactor);
		O_returned.w = glm::smoothstep(rotations[index].orientation.w, rotations[index+1].orientation.w, scalarFactor);*/
		return glm::toMat4(glm::normalize(O_returned));
	}
}


// figures out which scaling keys to interpolate b/w and performs the interpolation 
// and returns the scale matrix
inline const glm::mat4 InterpolateScaling(float animationTime, const KeyScale* m_Scales){
	if (numScalings == 1){
		return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);
	}

	uint index = GetScaleIndex(animationTime, m_Scales);
	float scalarFactor = GetScaleFactor(m_Scales[index].timeStamp, m_Scales[index + 1].timeStamp, animationTime);

	return glm::scale(glm::mat4(1.0f), mix(m_Scales[index].scale, m_Scales[index+1].scale, scalarFactor));
}

// interpolates  b/w positions,rotations & scaling keys based on the curren time of
// the animation and prepares the local transformation matrix by combining all keys tranformations
void Bone::Update(float animationTime){
	localTransform = InterpolatePosition(animationTime, mPositions) *
	                 InterpolateRotation(animationTime, mRotations) *
	                 InterpolateScaling (animationTime, mScales);
}

glm::mat4* Bone::GetLocalTransform() const { return &localTransform; }
