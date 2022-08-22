#pragma once

#include "libs/glad.h"
#include "libs/stb_image.h"
#include "shader.h"
#include "camera.h"
#include <glm/fwd.hpp>
#include <stdio.h>
#include <vector>
#include <string>

/*
class CubeMap{
public:

	CubeMap();

	void Draw(Camera* camera);

	void Clean();

};*/

unsigned int loadCubemap(std::vector<std::string> faces);

void initCubeMap();

void drawCubeMap();