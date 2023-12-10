#pragma once

#include "libs/glad/glad.h"
#include "libs/assimp_glm_helpers.h"
#include <unordered_map>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "ubo.h"

struct strequal_to{
	bool operator()(const char* s1, const char* s2) const{
		return (!strcmp(s1, s2));
	}
};

struct strHash{
	int operator()(const char* str) const{
		uint64_t hash = 0;

		while(*str){
			hash = (hash * hash) + (*str);
			str++;
		}

		return hash & (0x7FFFFFFF);
	}
};

class Shader{
public:
	// the program ID
	uint32_t ID;
	bool mapped = false;

	// constractor reads and build the shader
	Shader(const char* vertexPath, const char* fragmentPath);
	void compile_n_link(const char* vertexCode, const char* fragmentCode);

	// use the shader
	void use();
	void bind_ubo(ubo* uniformBuffer);

	//store the uniform location
	std::unordered_map<const char*, GLint, strHash, strequal_to> uniformLocationCache;

	GLint getUniformLocation(const char* name);

	// utility uniform functions
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
