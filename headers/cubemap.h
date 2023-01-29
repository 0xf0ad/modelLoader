#pragma once

#include "libs/glad.h"
#include "libs/stb_image.h"
#include "camera.h"
#include <stdio.h>


class CubeMap{
public:

	unsigned int cubeMapVAO, cubeMapVBO, cubemapTexture;

	CubeMap();

	void drawCubeMap();

};