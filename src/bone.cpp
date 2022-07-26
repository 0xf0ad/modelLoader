#include "../headers/bone.h"
int m_NumPositions;
int m_NumRotations;
int m_NumScalings;

glm::mat4 m_LocalTransform;

std::vector<KeyPosition> p_Positions;
std::vector<KeyRotation> p_Rotations;
std::vector<KeyScale> p_Scales;

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
	//p_Positions = m_Positions;

	for (int i = 0; i < m_NumRotations; i++){
		KeyRotation data;
		data.orientation = AssimpGLMHelpers::GetGLMQuat(channel->mRotationKeys[i].mValue);
		data.timeStamp = channel->mRotationKeys[i].mTime;
		Bone::m_Rotations.push_back(data);
	}
	//p_Rotations = m_Rotations;

	for (int i = 0; i < m_NumScalings; i++){
		KeyScale data;
		data.scale = AssimpGLMHelpers::GetGLMVec(channel->mScalingKeys[i].mValue);
		data.timeStamp = channel->mScalingKeys[i].mTime;
		Bone::m_Scales.push_back(data);
	}
	//p_Scales = m_Scales;
}

// Gets normalized value for Lerp & Slerp
float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime){
	return (animationTime - lastTimeStamp) / (nextTimeStamp - lastTimeStamp);
}

// Gets the current index on mKeyPositions to interpolate to based on
// the current animation time
int Bone::GetPositionIndex(float animationTime){
	for (int i = 0; i < (m_NumPositions-1); i++){
		if (animationTime < Bone::m_Positions[i+1].timeStamp){
			return i;
		}
	}
	assert(false);
}

// Gets the current index on mKeyRotations to interpolate to based on the
// current animation time
int Bone::GetRotationIndex(float animationTime){
	for (int i = 0; i < (m_NumRotations-1); i++){
		if (animationTime < Bone::m_Rotations[i+1].timeStamp){
			return i;
		}	
	}
	assert(false);
}

// Gets the current index on mKeyScalings to interpolate to based on the
// current animation time
int Bone::GetScaleIndex(float animationTime){
	for (int i = 0; i < (m_NumScalings-1); i++){
		if (animationTime < Bone::m_Scales[i+1].timeStamp){
			return i;
		}
	}
	assert(false);
}

// figures out which position keys to interpolate b/w and performs the interpolation 
// and returns the translation matrix
glm::mat4 Bone::InterpolatePosition(float animationTime){
	if (m_NumPositions == 1){
		return glm::translate(glm::mat4(1.0f), Bone::m_Positions[0].position);
	}

	int index = Bone::GetPositionIndex(animationTime);
	return glm::translate(glm::mat4(1.0f),
	                      glm::mix(Bone::m_Positions[index].position,
	                               Bone::m_Positions[index + 1].position,
	                               GetScaleFactor(Bone::m_Positions[index].timeStamp,
	                                              Bone::m_Positions[index + 1].timeStamp,
	                                              animationTime)));
}

// figures out which rotations keys to interpolate b/w and performs the interpolation 
// and returns the rotation matrix
glm::mat4 Bone::InterpolateRotation(float animationTime){
	if (m_NumRotations == 1){
		return glm::toMat4(glm::normalize(Bone::m_Rotations[0].orientation));
	}
	int index = Bone::GetRotationIndex(animationTime);
	return glm::toMat4(glm::normalize(glm::slerp(Bone::m_Rotations[index].orientation,
	                                             Bone::m_Rotations[index + 1].orientation,
	                                             GetScaleFactor(Bone::m_Rotations[index].timeStamp,
	                                                            Bone::m_Rotations[index + 1].timeStamp,
	                                                            animationTime))));
}

// figures out which scaling keys to interpolate b/w and performs the interpolation 
// and returns the scale matrix
glm::mat4 Bone::InterpolateScaling(float animationTime){
	if (m_NumScalings == 1){
		return glm::scale(glm::mat4(1.0f), Bone::m_Scales[0].scale);
	}

	int index = GetScaleIndex(animationTime);
	return glm::scale(glm::mat4(1.0f),
	                  glm::mix(Bone::m_Scales[index].scale,
	                           Bone::m_Scales[index + 1].scale,
	                           GetScaleFactor(Bone::m_Scales[index].timeStamp,
	                                          Bone::m_Scales[index + 1].timeStamp,
	                                          animationTime)));
}

// interpolates  b/w positions,rotations & scaling keys based on the curren time of
// the animation and prepares the local transformation matrix by combining all keys tranformations
void Bone::Update(float animationTime){
	m_LocalTransform = Bone::InterpolatePosition(animationTime)*
	                   Bone::InterpolateRotation(animationTime)*
	                   Bone::InterpolateScaling(animationTime);
}

glm::mat4 Bone::GetLocalTransform() { return m_LocalTransform; }
std::string Bone::GetBoneName() const { return Bone::m_Name; }
int Bone::GetBoneID() { return Bone::m_ID; }
