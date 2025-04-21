#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/bone.h"
#include "../headers/timer.h"
#include "../headers/mymath.h"

#define BINARYSEARCHINDEX     true
#define BICUBIC_INTERPOLATION true

static uint32_t numPositions, numRotations, numScalings;
extern bool G_squad;

// reads keyframes from aiNodeAnim
Bone::Bone(const char* name, int ID, const aiNodeAnim* channel){
#if BONENAME
	mName          = strdup(name);
#endif
	mID            = ID;
	numPositions   = channel->mNumPositionKeys;
	numScalings    = channel->mNumScalingKeys;
	numRotations   = channel->mNumRotationKeys;
	localTransform = glm::mat4(1.0f);

	mPositions = (KeyPosition*) malloc(sizeof(KeyPosition[numPositions]));
	mRotations = (KeyRotation*) malloc(sizeof(KeyRotation[numRotations]));
	mScales    = (KeyScale*)    malloc(sizeof(KeyScale[numScalings]));

	for (uint32_t i = 0; i != numPositions; i++){
		mPositions[i].position = assimpVec2glm(channel->mPositionKeys[i].mValue);
		mPositions[i].timeStamp = channel->mPositionKeys[i].mTime;
	}

	for (uint32_t i = 0; i != numRotations; i++){
		mRotations[i].orientation = assimpQuat2glm(channel->mRotationKeys[i].mValue);
		mRotations[i].timeStamp = channel->mRotationKeys[i].mTime;
	}

	for (uint32_t i = 0; i != numScalings; i++){
		mScales[i].scale = assimpVec2glm(channel->mScalingKeys[i].mValue);
		mScales[i].timeStamp = channel->mScalingKeys[i].mTime;
	}
}

Bone::Bone(Bone& other){
	printf("\t\tBONE : i am coppied from %p, and i live in %p\n", this, &other);
	#if BONENAME
	other.mName = strdup(this->mName);
	#endif
	other.mPositions = (KeyPosition*) malloc(sizeof(KeyPosition[numPositions]));
	other.mRotations = (KeyRotation*) malloc(sizeof(KeyRotation[numRotations]));
	other.mScales    = (KeyScale*)    malloc(sizeof(KeyScale[numScalings]));
}

Bone::~Bone(){

	#if BONENAME
	free((void*)mName);
	#endif

	free(mPositions);
	free(mRotations);
	free(mScales);
}

// Gets normalized value for Lerp & Slerp
inline float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime){
	return (animationTime - lastTimeStamp) / (nextTimeStamp - lastTimeStamp);
}

inline uint32_t binarySearchIndex(const float animationTime, const KeyPosition* positions, uint32_t offset, uint32_t end){
	if(offset >= end)
		return positions[offset].timeStamp <= animationTime ? offset+1 : offset;

	uint32_t mid = (offset + end) >> 1;
	if(positions[mid].timeStamp < animationTime)
		return binarySearchIndex(animationTime, positions, mid+1, end);

	else
		return binarySearchIndex(animationTime, positions, offset, mid);
}

inline uint32_t binarySearchIndex(const float animationTime, const KeyRotation* rotations, uint32_t offset, uint32_t end){
	if(offset >= end)
		return rotations[offset].timeStamp <= animationTime ? offset+1 : offset;

	uint32_t mid = (offset + end) >> 1;
	if(rotations[mid].timeStamp < animationTime)
		return binarySearchIndex(animationTime, rotations, mid+1, end);

	else
		return binarySearchIndex(animationTime, rotations, offset, mid);
}

inline uint32_t binarySearchIndex(const float animationTime, const KeyScale* scalings, uint32_t offset, uint32_t end){
	if(offset >= end)
		return scalings[offset].timeStamp <= animationTime ? offset+1 : offset;

	uint32_t mid = (offset + end) >> 1;
	if(scalings[mid].timeStamp < animationTime)
		return binarySearchIndex(animationTime, scalings, mid+1, end);

	else
		return binarySearchIndex(animationTime, scalings, offset, mid);
}


// Gets the current index on mKeyPositions to interpolate to based on
// the current animation time
inline uint32_t getIndex(const float animationTime, const KeyPosition* positions){

	static uint32_t prevResult = 0;

	if(prevResult < numPositions || animationTime > positions[prevResult].timeStamp)
		#if BINARYSEARCHINDEX
			prevResult = binarySearchIndex(animationTime, positions, 0, numPositions) - 1;
		#else
			for (uint32_t i = 0; i != (numRotations - 1); i++){
				if (animationTime < rotations[i+1].timeStamp){
					prevResult = i;
					return i;
				}
			}
		#endif

	return prevResult;
}

// Gets the current index on mKeyRotations to interpolate to based on the
// current animation time
inline uint32_t getIndex(const float animationTime, const KeyRotation* rotations){

	static uint32_t prevResult = 0;

	if(prevResult < numRotations || animationTime > rotations[prevResult].timeStamp)
		#if BINARYSEARCHINDEX
			prevResult = binarySearchIndex(animationTime, rotations, 0, numPositions) - 1;
		#else
			for (uint32_t i = 0; i != (numRotations - 1); i++){
				if (animationTime < rotations[i+1].timeStamp){
					prevResult = i;
					return i;
				}
			}
		#endif

	return prevResult;
}

// Gets the current index on mKeyScalings to interpolate to based on the
// current animation time
inline uint32_t getIndex(const float animationTime, const KeyScale* scales){

	static uint32_t prevResult = 0;

	if(prevResult < numScalings || animationTime > scales[prevResult].timeStamp)
		#if BINARYSEARCHINDEX
			prevResult = binarySearchIndex(animationTime, scales, 0, numPositions) - 1;
		#else
			for (uint32_t i = 0; i != (numRotations - 1); i++){
				if (animationTime < rotations[i+1].timeStamp){
					prevResult = i;
					return i;
				}
			}
		#endif

	return prevResult;
}

// figures out which position keys to interpolate b/w and performs the interpolation 
// and returns the translation matrix
inline const glm::mat4 interpolate(const float animationTime, const KeyPosition* positions){
	static glm::mat4 result = glm::translate(glm::mat4(1.0f), positions[0].position);

	if (numScalings == 1)
		return glm::translate(glm::mat4(1.0f), positions[0].position);

	uint32_t index = getIndex(animationTime, positions);
	float scalarFactor = GetScaleFactor(positions[index].timeStamp, positions[index + 1].timeStamp, animationTime);
	result = glm::translate(glm::mat4(1.0f),mix(positions[index].position, positions[index + 1].position, scalarFactor));

	return result;
}

// figures out which rotations keys to interpolate b/w and performs the interpolation 
// and returns the rotation matrix
inline const glm::mat4 interpolate(const float animationTime, const KeyRotation* rotations){
	static glm::mat4 result;

	if (numRotations == 1)
		return glm::toMat4(glm::normalize(rotations[0].orientation));

	uint32_t index = getIndex(animationTime, rotations);

	float scalarFactor = GetScaleFactor(rotations[index].timeStamp, rotations[index + 1].timeStamp, animationTime);

	glm::quat O_returned;
	glm::quat N_returned;

	/*
	* WHY DOES CUBIC INTERPOLATION SEEMS LIKE THE LINEAR ONE IT SHOULD NOT BE THAT WAY
	*/

	/*
	* FUCK IT I AM ONLY GOING TO USE THE SLERP, SQUAD IS JUST UNNECESSARY HEADACH
	*/

	if(G_squad){
		N_returned = glm::slerp(rotations[index].orientation, rotations[index+1].orientation, scalarFactor);
		/*O_returned = mix(N_returned , mix(glm::intermediate(m_Rotations[index-1].orientation, m_Rotations[index].orientation, m_Rotations[index+1].orientation)
										,glm::intermediate(m_Rotations[index].orientation, m_Rotations[index+1].orientation, m_Rotations[index+2].orientation), scalarFactor)
						,2*scalarFactor*(1-scalarFactor));*/

		//O_returned = squad_from_data(m_Rotations, index, scalarFactor);
		result = glm::toMat4(N_returned);
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
		result = glm::toMat4(glm::normalize(O_returned));
	}

	return result;
}


// figures out which scaling keys to interpolate b/w and performs the interpolation 
// and returns the scale matrix
inline const glm::mat4 interpolate(float animationTime, const KeyScale* scales){
	static glm::mat4 result;

	if (numScalings == 1){
		static glm::mat4 constResult = glm::scale(glm::mat4(1.0f), scales[0].scale);
		return constResult;
	}

	uint32_t index = getIndex(animationTime, scales);
	float scalarFactor = GetScaleFactor(scales[index].timeStamp, scales[index + 1].timeStamp, animationTime);

	result = glm::scale(glm::mat4(1.0f), mix(scales[index].scale, scales[index+1].scale, scalarFactor));


	return result;
}

// interpolates  b/w positions,rotations & scaling keys based on the curren time of
// the animation and prepares the local transformation matrix by combining all keys tranformations
void Bone::Update(float animationTime){
	localTransform = mul_Mat4Mat4(
		mul_Mat4Mat4(
			interpolate(animationTime, mPositions),
			interpolate(animationTime, mRotations)),
			interpolate(animationTime, mScales));
}
