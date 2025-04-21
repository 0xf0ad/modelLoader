#pragma once

#include "model.h"
#include "libs/assimp_glm_helpers.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#define BONENAME true

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
	KeyPosition* mPositions;
	KeyRotation* mRotations;
	KeyScale* mScales;

	glm::mat4 localTransform;

#if BONENAME
	const char* mName;
#endif
	uint32_t mID;

	// reads keyframes from aiNodeAnim
	Bone() = default;
	Bone(const char* name, int ID, const aiNodeAnim* channel);
	Bone(Bone& other);
	~Bone();

	// interpolates  b/w positions,rotations & scaling keys based on the curren time of
	// the animation and prepares the local transformation matrix by combining all keys tranformations
	void Update(float animationTime);
};
