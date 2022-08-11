#include "../headers/bone.h"

int m_NumPositions, m_NumRotations, m_NumScalings;

glm::mat4 m_LocalTransform;

// reads keyframes from aiNodeAnim
Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel){
	Bone::m_Name     = name;
	Bone::m_ID       = ID;
	m_NumPositions   = channel->mNumPositionKeys;
	m_NumScalings    = channel->mNumScalingKeys;
	m_NumRotations   = channel->mNumRotationKeys;
	m_LocalTransform = glm::mat4(1.0f);

	for (int i = 0; i < m_NumPositions; i++){
		KeyPosition data;
		data.position = AssimpGLMHelpers::GetGLMVec(channel->mPositionKeys[i].mValue);
		data.timeStamp = channel->mPositionKeys[i].mTime;
		Bone::m_Positions.push_back(data);
	}

	for (int i = 0; i < m_NumRotations; i++){
		KeyRotation data;
		data.orientation = AssimpGLMHelpers::GetGLMQuat(channel->mRotationKeys[i].mValue);
		data.timeStamp = channel->mRotationKeys[i].mTime;
		Bone::m_Rotations.push_back(data);
	}

	for (int i = 0; i < m_NumScalings; i++){
		KeyScale data;
		data.scale = AssimpGLMHelpers::GetGLMVec(channel->mScalingKeys[i].mValue);
		data.timeStamp = channel->mScalingKeys[i].mTime;
		Bone::m_Scales.push_back(data);
	}
}

// Gets normalized value for Lerp & Slerp
float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime){
	return (animationTime - lastTimeStamp) / (nextTimeStamp - lastTimeStamp);
}

// Gets the current index on mKeyPositions to interpolate to based on
// the current animation time
int GetPositionIndex(float animationTime, const std::vector<KeyPosition>& m_Positions){
	for (int i = 0; i < (m_NumPositions - 1); i++){
		if (animationTime < m_Positions[i+1].timeStamp){
			return i;
		}
	}
	assert(false);
}

// Gets the current index on mKeyRotations to interpolate to based on the
// current animation time
int GetRotationIndex(float animationTime, const std::vector<KeyRotation>& m_Rotations){
	for (int i = 0; i < (m_NumRotations - 1); i++){
		if (animationTime < m_Rotations[i+1].timeStamp){
			return i;
		}
	}
	assert(false);
}

// Gets the current index on mKeyScalings to interpolate to based on the
// current animation time
int GetScaleIndex(float animationTime, const std::vector<KeyScale>& m_Scales){
	for (int i = 0; i < (m_NumScalings - 1); i++){
		if (animationTime < m_Scales[i+1].timeStamp){
			return i;
		}
	}
	assert(false);
}

// figures out which position keys to interpolate b/w and performs the interpolation 
// and returns the translation matrix
glm::mat4 InterpolatePosition(float animationTime, const std::vector<KeyPosition>& m_Positions){
	if (m_NumPositions == 1){
		return glm::translate(glm::mat4(1.0f), m_Positions[0].position);
	}

	int index = GetPositionIndex(animationTime, m_Positions);
	return glm::translate(glm::mat4(1.0f),
	                      glm::mix(m_Positions[index].position,
	                               m_Positions[index + 1].position,
	                               GetScaleFactor(m_Positions[index].timeStamp,
	                                              m_Positions[index + 1].timeStamp,
	                                              animationTime)));
}

// figures out which rotations keys to interpolate b/w and performs the interpolation 
// and returns the rotation matrix
glm::mat4 InterpolateRotation(float animationTime, const std::vector<KeyRotation>& m_Rotations){
	if (m_NumRotations == 1){
		return glm::toMat4(glm::normalize(m_Rotations[0].orientation));
	}
	int index = GetRotationIndex(animationTime, m_Rotations);
	return glm::toMat4(glm::normalize(glm::slerp(m_Rotations[index].orientation,
	                                             m_Rotations[index + 1].orientation,
	                                             GetScaleFactor(m_Rotations[index].timeStamp,
	                                                            m_Rotations[index + 1].timeStamp,
	                                                            animationTime))));
}

// figures out which scaling keys to interpolate b/w and performs the interpolation 
// and returns the scale matrix
glm::mat4 InterpolateScaling(float animationTime, const std::vector<KeyScale>& m_Scales){
	if (m_NumScalings == 1){
		return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);
	}

	int index = GetScaleIndex(animationTime, m_Scales);
	return glm::scale(glm::mat4(1.0f),
	                  glm::mix(m_Scales[index].scale,
	                           m_Scales[index + 1].scale,
	                           GetScaleFactor(m_Scales[index].timeStamp,
	                                          m_Scales[index + 1].timeStamp,
	                                          animationTime)));
}

// interpolates  b/w positions,rotations & scaling keys based on the curren time of
// the animation and prepares the local transformation matrix by combining all keys tranformations
void Bone::Update(float animationTime){
	m_LocalTransform = InterpolatePosition(animationTime, Bone::m_Positions)*
	                   InterpolateRotation(animationTime, Bone::m_Rotations)*
	                   InterpolateScaling (animationTime, Bone::m_Scales);
}

glm::mat4* Bone::GetLocalTransform() const { return &m_LocalTransform; }
