#pragma once

#include "libs/stbi/stb_image.h"
#include "camera.h"
#include <stdio.h>


class CubeMap{
public:

	uint32_t cubeMapVAO, cubeMapVBO, cubemapTexture;

	CubeMap();

	void drawCubeMap();

};