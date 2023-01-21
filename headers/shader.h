#pragma once

#include "libs/glad.h"
#include "libs/assimp_glm_helpers.h"
#include <unordered_map>

#if IWANNAUSESTD__STRING

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#else

#include <bits/types/FILE.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#endif

struct strequal_to{
	bool operator()(const char* s1, const char* s2) const{
		return (!strcmp(s1, s2));
	}
};

struct strHash{
	int operator()(const char* str) const{
		unsigned int long hash = 0;

		while(*str){
			hash = (hash * hash) + *str;
			str++;	
		}

		return hash & (0x7FFFFFFF);
	}
};

class Shader{
public:
	// the program ID
	unsigned int ID;
	bool mapped = false;

	// constractor reads and build the shader
	Shader(const char* vertexPath, const char* fragmentPath);

	// use the shader
	void use();

	//store the uniform location
#if IWANNAUSESTD__STRING
	std::unordered_map<std::string, GLint> uniformLocationCache;
#else
	std::unordered_map<const char*, GLint, strHash, strequal_to> uniformLocationCache;
#endif

	GLint getUniformLocation(const char* name);

	// utility uniform functions
#if IWANNAUSESTD__STRING
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
#endif

	void setBool (const char* name, bool  value);
	void setInt  (const char* name, int   value);
	void setFloat(const char* name, float value);
	void setVec2 (const char* name, const glm::vec2 &value);
	void setVec2 (const char* name, float x, float y);
	void setVec3 (const char* name, const glm::vec3 &value);
	void setVec3 (const char* name, float x, float y, float z);
	void setVec4 (const char* name, const glm::vec4 &value);
	void setVec4 (const char* name, float x, float y, float z, float w);
	void setMat2 (const char* name, const glm::mat2 &mat);
	void setMat3 (const char* name, const glm::mat3 &mat);
	void setMat4 (const char* name, const glm::mat4 &mat);
};
