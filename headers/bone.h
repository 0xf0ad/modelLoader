#pragma once
#include <list>
#include "model.h"
#include "libs/assimp_glm_helpers.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

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
public:
	std::vector<KeyPosition> m_Positions;
	std::vector<KeyRotation> m_Rotations;
	std::vector<KeyScale> m_Scales;
	std::string m_Name;
	int m_ID;

	// reads keyframes from aiNodeAnim
	Bone(const std::string& name, int ID, const aiNodeAnim* channel);

	// interpolates  b/w positions,rotations & scaling keys based on the curren time of
	// the animation and prepares the local transformation matrix by combining all keys tranformations
	void Update(float animationTime);

	int GetPositionIndex(float animationTime);

	int GetRotationIndex(float animationTime);

	int GetScaleIndex(float animationTime);

	glm::mat4 InterpolatePosition(float animationTime);

	glm::mat4 InterpolateRotation(float animationTime);

	glm::mat4 InterpolateScaling(float animationTime);

	glm::mat4 GetLocalTransform();
};
