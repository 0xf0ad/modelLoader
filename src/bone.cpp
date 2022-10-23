#include "../headers/bone.h"
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_exponential.hpp>
#include <glm/fwd.hpp>
#include <glm/gtx/quaternion.hpp>
#include <math.h>

static int m_NumPositions, m_NumRotations, m_NumScalings;
extern bool Q_squad = false;

static glm::mat4 m_LocalTransform;

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
// math shit
const glm::vec3 mix(const glm::vec3& a, const glm::vec3& b, const float t){
	//return (a * (1-t)) + (b * t);
	//return a + ( t * ( b - a ));
	const float Tsquare      = t * t;
	const float threeTsquare = 3 * Tsquare;
	const float twoTcube     = 2 * t * Tsquare;
	const glm::vec3 result   = ( a * ( twoTcube - threeTsquare + 1.0f ) ) + ( b * ( threeTsquare - twoTcube ));
	return result;
}

const glm::quat mix(const glm::quat& a, const glm::quat& b, const float t){
	const float Tsquare      = t * t;
	const float threeTsquare = 3 * Tsquare;
	const float twoTminus3   = 2 * t - 3; 
	//const float twoTcube     = 2 * t * Tsquare;
	//const glm::quat result   = ( a * ( twoTcube - threeTsquare + 1.0f ) ) + ( b * ( threeTsquare - twoTcube ));
	const glm::quat result   = ((a * (Tsquare * twoTminus3 + 1.0f)) - (b * Tsquare * twoTminus3));
	return result;
}

const glm::quat inverse (const glm::quat& q){
	glm::quat result;
	result.w =  (q.w);
	result.x = -(q.x);
	result.y = -(q.y);
	result.z = -(q.z);
	return result;
}

const glm::quat log(const glm::quat& q){
	float a = acosf(q.w);
	const float s = sinf(a);

	if(!s) return glm::quat(0.0f, 0.0f, 0.0f, 0.0f);

	a /= s;

	const glm::quat result = glm::quat(0.0f, (q.x * a), (q.y * a), (q.z * a));
	return result;
}

const glm::quat exp(const glm::quat& q){
	const float a = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z);
	float s = sinf(a);
	const float c = cosf(a);

	if (!a) return glm::quat( c, 0, 0, 0 );

	s /= a;

	const glm::quat result = glm::quat(c, q.x * s,  q.y * s,  q.z * s);
	return result;
}

const glm::quat intermediate(const glm::quat& previous, const glm::quat& current, const glm::quat& next){
	const glm::quat inv_quat = inverse(current);
	const glm::quat result = current * exp( (glm::log(next * inv_quat) + glm::log(previous * inv_quat)) * (-0.25f));

	return result;
}

const glm::quat squad(const glm::quat& q1, const glm::quat& q2, const glm::quat& s1, const glm::quat& s2, const float t){
	const glm::quat slerp1 = mix(q1, q2, t);
	const glm::quat slerp2 = mix(s1, s2, t);
	const float   t_factor = 2 * t * (1.0f-t);

	const glm::quat result = mix(slerp1, slerp2, t_factor);
	
	return result;
}

const glm::quat all_in_one_squad(const glm::quat& q0, const glm::quat& q1, const glm::quat& q2, const glm::quat& q3, const float t){
	const glm::quat inner1 = glm::intermediate(q0, q1, q2);
	const glm::quat inner2 = glm::intermediate(q1, q2, q3);

	const glm::quat result = glm::squad(q1, q2, inner1, inner2, t);

	return result;
}

const glm::quat squad_from_data(const std::vector<KeyRotation>& rotations, const unsigned int current_index, const float scalarFactor){
	const glm::quat* q_prev = &rotations[current_index-1].orientation;
	const glm::quat* q_curr = &rotations[ current_index ].orientation;
	const glm::quat* q_next = &rotations[current_index+1].orientation;
	const glm::quat* q_nex2 = &rotations[current_index+2].orientation;

	//const float t_fract = scalarFactor - (int)scalarFactor;

	const glm::quat result = all_in_one_squad(*q_prev, *q_curr, *q_next, *q_nex2, scalarFactor);

	return result;
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
	float scalarFactor = GetScaleFactor(m_Rotations[index].timeStamp, m_Rotations[index+1].timeStamp, animationTime);

	glm::quat O_returned;
	glm::quat N_returned;

	/*
	 * WHY DOES CUBIC INTERPOLATION SEEMS LIKE THE LINEAR ONE IT SHOULD NOT BE THAT WAY
	 */
	
	if(Q_squad){
		N_returned = mix(m_Rotations[index].orientation, m_Rotations[index+1].orientation, scalarFactor);
		/*O_returned = mix(N_returned , mix(glm::intermediate(m_Rotations[index-1].orientation, m_Rotations[index].orientation, m_Rotations[index+1].orientation)
		                                   ,glm::intermediate(m_Rotations[index].orientation, m_Rotations[index+1].orientation, m_Rotations[index+2].orientation), scalarFactor)
						 ,2*scalarFactor*(1-scalarFactor));*/
		
		//O_returned = squad_from_data(m_Rotations, index, scalarFactor);
	}else{
		O_returned = squad(m_Rotations[index].orientation, m_Rotations[index+1].orientation,
			    //   m_Rotations[index].orientation * glm::exp((glm::log(q * m_Rotations[index+1].orientation) + glm::log(q * m_Rotations[index-1].orientation)) * -0.25f),
				//   m_Rotations[index+1].orientation * glm::exp((glm::log(q * m_Rotations[index].orientation) + glm::log(q * m_Rotations[index+2].orientation)) * -0.25f),
			       intermediate(m_Rotations[index-1].orientation, m_Rotations[index].orientation, m_Rotations[index+1].orientation),
				   intermediate(m_Rotations[index].orientation, m_Rotations[index+1].orientation, m_Rotations[index+2].orientation),
				   scalarFactor);
	}
	
	if(Q_squad){
		return glm::toMat4(glm::normalize(N_returned));
	}else{
		return glm::toMat4(glm::normalize(O_returned));
	}
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
