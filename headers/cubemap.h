#pragma once

#include "libs/glad.h"
#include "libs/stb_image.h"
#include "camera.h"
#include <stdio.h>


class CubeMap{

	unsigned int cubeMapVAO, cubeMapVBO, cubemapTexture;

public:

	CubeMap();

	void drawCubeMap();

};