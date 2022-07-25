#include "../headers/bone.h"
#if TEST

std::vector<KeyPosition> m_Positions;
std::vector<KeyRotation> m_Rotations;
std::vector<KeyScale> m_Scales;
int m_NumPositions;
int m_NumRotations;
int m_NumScalings;

glm::mat4 m_LocalTransform;
std::string m_Name;
int m_ID;

// reads keyframes from aiNodeAnim
Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel){
	m_Name           = name;
	m_ID             = ID;
	m_NumPositions   = channel->mNumPositionKeys;
	m_NumScalings    = channel->mNumScalingKeys;
	m_NumRotations   = channel->mNumRotationKeys;
	m_LocalTransform = glm::mat4(1.0f);
	
	for (int i = 0; i < m_NumPositions; i++){
		KeyPosition data;
		data.position = AssimpGLMHelpers::GetGLMVec(channel->mPositionKeys[i].mValue);
		data.timeStamp = channel->mPositionKeys[i].mTime;
		m_Positions.push_back(data);
	}

	for (int i = 0; i < m_NumRotations; i++){
		KeyRotation data;
		data.orientation = AssimpGLMHelpers::GetGLMQuat(channel->mRotationKeys[i].mValue);
		data.timeStamp = channel->mRotationKeys[i].mTime;
		m_Rotations.push_back(data);
	}

	for (int i = 0; i < m_NumScalings; i++){
		KeyScale data;
		data.scale = AssimpGLMHelpers::GetGLMVec(channel->mScalingKeys[i].mValue);
		data.timeStamp = channel->mScalingKeys[i].mTime;
		m_Scales.push_back(data);
	}
}

// Gets normalized value for Lerp & Slerp
float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime){
	return (animationTime - lastTimeStamp) / (nextTimeStamp - lastTimeStamp);
}

// Gets the current index on mKeyPositions to interpolate to based on
// the current animation time
int GetPositionIndex(float animationTime){
	for (int i = 0; i < (m_NumPositions-1); i++){
		if (animationTime < m_Positions[i+1].timeStamp){
			return i;
		}
	}
	assert(false);
}

// Gets the current index on mKeyRotations to interpolate to based on the
// current animation time
int GetRotationIndex(float animationTime){
	for (int i = 0; i < (m_NumRotations-1); i++){
		if (animationTime < m_Rotations[i+1].timeStamp){
			return i;
	}}
	assert(false);
}

// Gets the current index on mKeyScalings to interpolate to based on the
// current animation time
int GetScaleIndex(float animationTime){
	for (int i = 0; i < (m_NumScalings-1); i++){
		if (animationTime < m_Scales[i+1].timeStamp){
			return i;
		}
	}
	assert(false);
}

// figures out which position keys to interpolate b/w and performs the interpolation 
// and returns the translation matrix
glm::mat4 InterpolatePosition(float animationTime){
	if (m_NumPositions == 1){
		return glm::translate(glm::mat4(1.0f), m_Positions[0].position);
	}

	return glm::translate(glm::mat4(1.0f),
	                      glm::mix(m_Positions[GetPositionIndex(animationTime)].position,
	                               m_Positions[GetPositionIndex(animationTime) + 1].position,
	                               GetScaleFactor(m_Positions[GetPositionIndex(animationTime)].timeStamp,
	                                              m_Positions[GetPositionIndex(animationTime) + 1].timeStamp,
	                                              animationTime)));
}

// figures out which rotations keys to interpolate b/w and performs the interpolation 
// and returns the rotation matrix
glm::mat4 InterpolateRotation(float animationTime){
	if (m_NumRotations == 1){
		return glm::toMat4(glm::normalize(m_Rotations[0].orientation));
	}
	return glm::toMat4(glm::normalize(glm::slerp(m_Rotations[GetRotationIndex(animationTime)].orientation,
	                                             m_Rotations[GetRotationIndex(animationTime) + 1].orientation,
	                                             GetScaleFactor(m_Rotations[GetRotationIndex(animationTime)].timeStamp,
	                                                            m_Rotations[GetRotationIndex(animationTime) + 1].timeStamp,
	                                                            animationTime))));
}

// figures out which scaling keys to interpolate b/w and performs the interpolation 
// and returns the scale matrix
glm::mat4 InterpolateScaling(float animationTime){
	if (m_NumScalings == 1){
		return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);
	}

	return glm::scale(glm::mat4(1.0f),
	                  glm::mix(m_Scales[GetScaleIndex(animationTime)].scale,
	                           m_Scales[GetScaleIndex(animationTime) + 1].scale,
	                           GetScaleFactor(m_Scales[GetScaleIndex(animationTime)].timeStamp,
	                                          m_Scales[GetScaleIndex(animationTime) + 1].timeStamp,
	                                          animationTime)));
}

// interpolates  b/w positions,rotations & scaling keys based on the curren time of
// the animation and prepares the local transformation matrix by combining all keys tranformations
void Bone::Update(float animationTime){
	m_LocalTransform = InterpolatePosition(animationTime)*
	                   InterpolateRotation(animationTime)*
	                   InterpolateScaling(animationTime);
}

glm::mat4 Bone::GetLocalTransform() { return m_LocalTransform; }
std::string Bone::GetBoneName() const { 
	return m_Name;
}
int Bone::GetBoneID() { 
	return m_ID;
}

#endif
