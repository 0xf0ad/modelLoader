#pragma once

#include <vector>
#include <assimp/scene.h>
#include <list>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <map>
#include "mesh.h"
#include "libs/assimp_glm_helpers.h"

struct KeyPosition{
	glm::vec3 position;
	float timeStamp;
};

struct KeyRotation{
	glm::quat orientation;
	float timeStamp;
};

struct KeyScale{
	glm::vec3 scale;
	float timeStamp;
};

class Bone{
    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;
	
    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;

public:

	// reads keyframes from aiNodeAnim
	Bone(const std::string& name, int ID, const aiNodeAnim* channel)
		: m_Name(name), m_ID(ID), m_LocalTransform(1.0f){

		m_NumPositions = channel->mNumPositionKeys;
		m_NumScalings  = channel->mNumScalingKeys;
		m_NumRotations = channel->mNumRotationKeys;
		
		for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex){
			KeyPosition data;
			data.position = AssimpGLMHelpers::GetGLMVec(channel->mPositionKeys[positionIndex].mValue);
			data.timeStamp = channel->mPositionKeys[positionIndex].mTime;
			m_Positions.push_back(data);
		}

		for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex){
			KeyRotation data;
			data.orientation = AssimpGLMHelpers::GetGLMQuat(channel->mRotationKeys[rotationIndex].mValue);
			data.timeStamp = channel->mRotationKeys[rotationIndex].mTime;
			m_Rotations.push_back(data);
		}

		for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex){
			KeyScale data;
			data.scale = AssimpGLMHelpers::GetGLMVec(channel->mScalingKeys[keyIndex].mValue);
			data.timeStamp = channel->mScalingKeys[keyIndex].mTime;
			m_Scales.push_back(data);
		}
	}

	// interpolates  b/w positions,rotations & scaling keys based on the curren time of 
	// the animation and prepares the local transformation matrix by combining all keys 
	// tranformations
	void Update(float animationTime){
		m_LocalTransform = InterpolatePosition(animationTime)*
		                   InterpolateRotation(animationTime)*
		                   InterpolateScaling(animationTime) ;
	}

	glm::mat4 GetLocalTransform() { return m_LocalTransform; }
	std::string GetBoneName() const { return m_Name; }
	int GetBoneID() { return m_ID; }

	// Gets the current index on mKeyPositions to interpolate to based on 
	// the current animation time
	int GetPositionIndex(float animationTime){
		for (int i = 0; i < m_NumPositions - 1; ++i){
			if (animationTime < m_Positions[i+1].timeStamp)
				return i;
		}
		assert(0);
	}

    // Gets the current index on mKeyRotations to interpolate to based on the 
    // current animation time
	int GetRotationIndex(float animationTime){
		for (int i = 0; i < m_NumRotations - 1; ++i){
			if (animationTime < m_Rotations[i+1].timeStamp)
				return i;
		}
		assert(0);
	}

	// Gets the current index on mKeyScalings to interpolate to based on the 
	// current animation time
	int GetScaleIndex(float animationTime){
		for (int i = 0; i < m_NumScalings - 1; ++i){
			if (animationTime < m_Scales[i+1].timeStamp){
				return i;
			}
		}
		assert(0);
	}

private:

	// Gets normalized value for Lerp & Slerp
	float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime){
		return (animationTime - lastTimeStamp) / (nextTimeStamp - lastTimeStamp);
	}

	// figures out which position keys to interpolate b/w and performs the interpolation 
	// and returns the translation matrix
	glm::mat4 InterpolatePosition(float animationTime){
		if (m_NumPositions == 1){
			return glm::translate(glm::mat4(1.0f), m_Positions[0].position);
		}

		int p0Index = GetPositionIndex(animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp, m_Positions[p1Index].timeStamp, animationTime);
		glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position , m_Positions[p1Index].position , scaleFactor);
		return glm::translate(glm::mat4(1.0f), finalPosition);
		// return glm::translate(glm::mat4(1.0f), glm::mix(m_Positions[GetPositionIndex(animationTime)].position, m_Positions[GetPositionIndex(animationTime) + 1].position, GetScaleFactor(m_Positions[GetPositionIndex(animationTime)].timeStamp, m_Positions[GetPositionIndex(animationTime) + 1].timeStamp, animationTime));
	}

    // figures out which rotations keys to interpolate b/w and performs the interpolation 
    // and returns the rotation matrix
	glm::mat4 InterpolateRotation(float animationTime){
		if (m_NumRotations == 1){
			return glm::toMat4(glm::normalize(m_Rotations[0].orientation));
		}

		int p0Index = GetRotationIndex(animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp, m_Rotations[p1Index].timeStamp, animationTime);
		glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation, scaleFactor);
		finalRotation = glm::normalize(finalRotation);
		return glm::toMat4(finalRotation);
	}

    // figures out which scaling keys to interpolate b/w and performs the interpolation 
    // and returns the scale matrix
	glm::mat4 InterpolateScaling(float animationTime){
		if (m_NumScalings == 1){
			return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);
		}

		int p0Index = GetScaleIndex(animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp, m_Scales[p1Index].timeStamp, animationTime);
		glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale, scaleFactor);
		return glm::scale(glm::mat4(1.0f), finalScale);
	}
};