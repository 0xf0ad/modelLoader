#pragma once

#include "model.h"
#include "libs/assimp_glm_helpers.h"
#include <glm/gtx/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
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

#if BONENAME
	const char* mName;
#endif
	unsigned int mID;

	// reads keyframes from aiNodeAnim
	Bone() = default;
	Bone(const char* name, int ID, const aiNodeAnim* channel);
	Bone(Bone& other);
	~Bone();

	// interpolates  b/w positions,rotations & scaling keys based on the curren time of
	// the animation and prepares the local transformation matrix by combining all keys tranformations
	void Update(float animationTime);

	// getting the local transofmation outside of the class
	glm::mat4* GetLocalTransform() const;
};
