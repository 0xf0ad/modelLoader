#ifndef _UBO_H
#define _UBO_H
#include "libs/glad/glad.h"
#include <stdint.h>
#include <stdlib.h>

class ubo{
public:

	GLuint UBO;
	uint8_t bindingPoint;
	uint32_t bufferSize;
	const char* name;
	void* buffer;

	ubo(const char* pName, size_t pBufferSize);

	~ubo(){
		freeBuffer();
		free((void*)name);
	}

	void bind(uint8_t bindingPoint);
	void allocBuffer(size_t size);
	void editBuffer(size_t offset, size_t size, void* data);
	void freeBuffer();
};
#endif