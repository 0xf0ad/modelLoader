#pragma once

#include "libs/glad.h"
#include "libs/assimp_glm_helpers.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

class Shader{
public:
	//the program ID
	unsigned int ID;
	bool mapped = false;

	//constractor reads and build the shader
	Shader(const char* vertexPath, const char* fragmentPath);

	//use the shader
	void use();

	//store the uniform location
	std::unordered_map<std::string, GLint> uniformLocationCache;
	GLint getUniformLocation(const std::string& name);

	// utility uniform functions
	void setBool (const std::string &name, bool  value);
	void setInt  (const std::string &name, int   value);
	void setFloat(const std::string &name, float value);
	void setVec2 (const std::string &name, const glm::vec2 &value);
	void setVec2 (const std::string &name, float x, float y);
	void setVec3 (const std::string &name, const glm::vec3 &value);
	void setVec3 (const std::string &name, float x, float y, float z);
	void setVec4 (const std::string &name, const glm::vec4 &value);
	void setVec4 (const std::string &name, float x, float y, float z, float w);
	void setMat2 (const std::string &name, const glm::mat2 &mat);
	void setMat3 (const std::string &name, const glm::mat3 &mat);
	void setMat4 (const std::string &name, const glm::mat4 &mat);
};
