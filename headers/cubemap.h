#pragma once

#include "libs/stbi/stb_image.h"
#include "camera.h"
#include <stdio.h>


class CubeMap{
public:

	unsigned int cubeMapVAO, cubeMapVBO, cubemapTexture;

	CubeMap();

	void drawCubeMap();

};