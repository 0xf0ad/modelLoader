#include "../headers/ubo.h"
#include <stddef.h>
#include <string.h>


ubo::ubo(const char* pName, size_t pBufferSize){
	name = strndup(pName, strlen(pName));
	//buffer = malloc(bufferSize);
	bufferSize = pBufferSize;
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
 
void ubo::allocBuffer(size_t size){
	bufferSize = size;
	//buffer = malloc(size);
}

void ubo::bind(uint8_t pbindingPoint){
	this->bindingPoint = pbindingPoint;
	glBindBufferBase(GL_UNIFORM_BUFFER, pbindingPoint, UBO);
	glBindBufferRange(GL_UNIFORM_BUFFER, pbindingPoint, UBO, 0, bufferSize);
}

void ubo::editBuffer(size_t offset, size_t size, void* data){
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ubo::freeBuffer(){
	bufferSize = 0;
	//free(buffer);
	//buffer = nullptr;
}
