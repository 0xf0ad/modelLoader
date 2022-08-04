#pragma once

#include "libs/glad.h"
#include "libs/assimp_glm_helpers.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

class Shader{

public:
	//the program ID
	unsigned int ID;

	//constractor reads and build the shader
	Shader(const char* vertexPath, const char* fragmentPath);

	//use the shader
	const void use();

	// utility uniform functions
	const void setBool(const std::string &name, bool value);
	const void setInt(const std::string &name, int value);
	const void setFloat(const std::string &name, float value);
	const void setVec2(const std::string &name, const glm::vec2 &value);
	const void setVec2(const std::string &name, float x, float y);
	const void setVec3(const std::string &name, const glm::vec3 &value);
	const void setVec3(const std::string &name, float x, float y, float z);
	const void setVec4(const std::string &name, const glm::vec4 &value);
	const void setVec4(const std::string &name, float x, float y, float z, float w);
	const void setMat2(const std::string &name, const glm::mat2 &mat);
	const void setMat3(const std::string &name, const glm::mat3 &mat);
	const void setMat4(const std::string &name, const glm::mat4 &mat);
};
