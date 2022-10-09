#include "../headers/bone.h"
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_exponential.hpp>
#include <glm/fwd.hpp>
#include <glm/gtx/quaternion.hpp>

int m_NumPositions, m_NumRotations, m_NumScalings;
//extern bool Q_squad = 0;

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

glm::vec3 mix(const glm::vec3& a, const glm::vec3& b, const float& t){
	//return (a * (1-t)) + (b * t);
	//return a + ( t * ( b - a ));
	float Tsquare      = t * t;
	float threeTsquare = 3 * Tsquare;
	float twoTcube     = 2 * t * Tsquare;
	return ( a * ( twoTcube - threeTsquare + 1.0f ) ) + ( b * ( threeTsquare - twoTcube ));
}

glm::quat qmix(const glm::quat& a, const glm::quat& b, const float& t){
	float Tsquare      = t * t;
	float threeTsquare = 3 * Tsquare;
	float twoTcube     = 2 * t * Tsquare;
	return ( a * ( twoTcube - threeTsquare + 1.0f ) ) + ( b * ( threeTsquare - twoTcube ));
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
		mix(m_Positions[index].position, m_Positions[index + 1].position,
			GetScaleFactor(m_Positions[index].timeStamp, m_Positions[index + 1].timeStamp, animationTime)));
}

// figures out which rotations keys to interpolate b/w and performs the interpolation 
// and returns the rotation matrix
glm::mat4 InterpolateRotation(float animationTime, const std::vector<KeyRotation>& m_Rotations){
	if (m_NumRotations == 1){
		return glm::toMat4(glm::normalize(m_Rotations[0].orientation));
	}

	int index = GetRotationIndex(animationTime, m_Rotations);
	//glm::quat q = glm::quat( -m_Rotations[index].orientation.x, -m_Rotations[index].orientation.y, -m_Rotations[index].orientation.z, m_Rotations[index].orientation.w );

	float scalarFactor = GetScaleFactor(m_Rotations[index].timeStamp, m_Rotations[index+1].timeStamp, animationTime);

	//glm::quat O_returned;
	glm::quat N_returned;

	/*
	 * WHY DOES CUBIC INTERPOLATION SEEMS LIKE THE LINEAR ONE IT SHOULD NT BE THAT WAY
	 */
	
	//if(Q_squad){
		/*O_returned = glm::squad(m_Rotations[index].orientation, m_Rotations[index+1].orientation,
			    //   m_Rotations[index].orientation * glm::exp((glm::log(q * m_Rotations[index+1].orientation) + glm::log(q * m_Rotations[index-1].orientation)) * -0.25f),
				//   m_Rotations[index+1].orientation * glm::exp((glm::log(q * m_Rotations[index].orientation) + glm::log(q * m_Rotations[index+2].orientation)) * -0.25f),
			       glm::intermediate(m_Rotations[index-1].orientation, m_Rotations[index].orientation, m_Rotations[index+1].orientation),
				   glm::intermediate(m_Rotations[index].orientation, m_Rotations[index+1].orientation, m_Rotations[index+2].orientation),
				   scalarFactor);*/
		N_returned = qmix(m_Rotations[index].orientation, m_Rotations[index+1].orientation, scalarFactor);
		/*O_returned = qmix(N_returned , qmix(glm::intermediate(m_Rotations[index-1].orientation, m_Rotations[index].orientation, m_Rotations[index+1].orientation)
		                                   ,glm::intermediate(m_Rotations[index].orientation, m_Rotations[index+1].orientation, m_Rotations[index+2].orientation), scalarFactor)
						 ,2*scalarFactor*(1-scalarFactor));*/
	//}else{
		
	//}

		//printf("looool : %f\n", (N_returned.x));
		//printf("%f\t%f\t%f\t%f\n", N_returned.x, N_returned.y, N_returned.z, N_returned.w);
	
	//if(Q_squad){
		return glm::toMat4(glm::normalize(N_returned));
	/*}else{
		return glm::toMat4(glm::normalize(O_returned));
	}*/
}


// figures out which scaling keys to interpolate b/w and performs the interpolation 
// and returns the scale matrix
glm::mat4 InterpolateScaling(float animationTime, const std::vector<KeyScale>& m_Scales){
	if (m_NumScalings == 1){
		return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);
	}
	
	int index = GetScaleIndex(animationTime, m_Scales);

	return glm::scale(glm::mat4(1.0f),
		mix(m_Scales[index].scale, m_Scales[index + 1].scale,
			GetScaleFactor(m_Scales[index].timeStamp, m_Scales[index + 1].timeStamp, animationTime)));
}

// interpolates  b/w positions,rotations & scaling keys based on the curren time of
// the animation and prepares the local transformation matrix by combining all keys tranformations
void Bone::Update(float animationTime){
	m_LocalTransform = InterpolatePosition(animationTime, Bone::m_Positions)*
	                   InterpolateRotation(animationTime, Bone::m_Rotations)*
	                   InterpolateScaling (animationTime, Bone::m_Scales);
}

glm::mat4* Bone::GetLocalTransform() const { return &m_LocalTransform; }
