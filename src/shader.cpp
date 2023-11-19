#include "../headers/shader.h"
#include <cstdio>

#define IWASNOTCOOL         false // cuz I am cool, and i use C
#define CACHSHADERLOCATIONS  true // idk why but it just give me worst results

#if IWASNOTCOOL
Shader::Shader(const char* vertexPath, const char* fragmentPath){
	// 1 - retrieve the vertex and fragment source code from filepath
	// --------------------------------------------------------------
	std::string   vertexCode , fragmentCode;
	std::ifstream vShaderFile, fShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
	try{
		// open file
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		//read file's buffer content into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		//close files
		vShaderFile.close();
		fShaderFile.close();
		//convert buffer streams to strings
		vertexCode   = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}catch(std::ifstream::failure e){
		std::cout << "ERROR : cannot read shader file \n";
	}

	compile_n_link(vertexCode.c_str(), fragmentCode.c_str());
}
#else /* I WAS COOL */

Shader::Shader(const char* vertexPath, const char* fragmentPath){

	// 1 - retrieve the vertex and fragment source code from filepath
	// --------------------------------------------------------------
	char *vertexCode,   *fragmentCode;
	FILE *v_ShaderFile, *f_ShaderFile;
	uint  v_StreamSize,  f_StreamSize;

	// open file
	v_ShaderFile = fopen(vertexPath  , "r");
	f_ShaderFile = fopen(fragmentPath, "r");

	// error handlling
	if(!(v_ShaderFile && f_ShaderFile)){
		if(!v_ShaderFile)
			fprintf(stderr, "could not open vertex shader file, check if it exists on %s\n", vertexPath);
		if(!f_ShaderFile)
			fprintf(stderr, "could not open fragment shader file, check if it exists on %s\n", fragmentPath);
		return;

	}else{

		// getting the size of the vertex shader file
		fseek(v_ShaderFile, 0L, SEEK_END);
		v_StreamSize = ftell(v_ShaderFile);
		rewind(v_ShaderFile);
		// getting the size of the fragment shader file
		fseek(f_ShaderFile, 0L, SEEK_END);
		f_StreamSize = ftell(f_ShaderFile);
		rewind(f_ShaderFile);

		// allocating the heap to be filed by the streams
		vertexCode   = (char*)malloc(v_StreamSize);
		fragmentCode = (char*)malloc(f_StreamSize);

		//read file's buffer content into streams
		uint32_t i = 0;
		while(fgets((vertexCode + i), v_StreamSize, v_ShaderFile))
			i = ftell(v_ShaderFile);
		i = 0;
		while(fgets((fragmentCode + i), f_StreamSize, f_ShaderFile))
			i = ftell(f_ShaderFile);

		// close files
		fclose(v_ShaderFile);
		fclose(f_ShaderFile);
	}
	#ifdef PRINTPARSSEDCODE
	printf("%s\n\n", vertexCode);
	printf("%s\n\n", fragmentCode);
	#endif

	compile_n_link(vertexCode, fragmentCode);

	free(vertexCode);
	free(fragmentCode);
}
#endif /* 'C'(see) I told you i was cool */

void Shader::compile_n_link(const char *vertexCode, const char *fragmentCode){
	
	uint32_t vertex, fragment;
	int success;
	char infoLog[512];

	// compile the vertex shader and free it's string as long as we dont need it
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexCode, nullptr);
	glCompileShader(vertex);
	// check for errors durring compiling the vertex shader
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(vertex, sizeof(infoLog), nullptr, infoLog);
		fprintf(stderr, "ERROR : failed to compile the vertex shader : \n\n%s\n", infoLog);
	}

	// compile the fragment shader and free it's string as long as we dont need it
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentCode, nullptr);
	glCompileShader(fragment);
	// check for errors durring compiling the fragment shader
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(fragment, sizeof(infoLog), nullptr, infoLog);
		fprintf(stderr, "ERROR : failed to compile the fragment shader : \n\n%s\n", infoLog);
	}


	// 3 - attach Program
	// --------------------------------------------------------------
	// attach the shaders to the program ID
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);

	// check linking errors
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if(!success){
		glGetProgramInfoLog(ID, sizeof(infoLog), nullptr, infoLog);
		fprintf(stderr, "ERROR : failed to link the shaders to the program : \n%s\n", infoLog);
	}

	// delete shader after linking them
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}


#if CACHSHADERLOCATIONS
GLint Shader::getUniformLocation(const char* name){
	// if the uniform location is already cached
	if(Shader::uniformLocationCache.find(name) != Shader::uniformLocationCache.end()){
		return Shader::uniformLocationCache[name];
	}else{
		// we store the uniform location if its not already
		GLint location = glGetUniformLocation(ID, name);
		Shader::uniformLocationCache[name] = location;
		return location;
	}
}
#endif

void Shader::bind_ubo(ubo* uniformBuffer){
	GLuint index = glGetUniformBlockIndex(ID, uniformBuffer->name);
	glUniformBlockBinding(ID, index, uniformBuffer->bindingPoint);
	glBindBufferBase(GL_UNIFORM_BUFFER, uniformBuffer->bindingPoint, uniformBuffer->UBO);
}

// activate the shader
// ------------------------------------------------------------------------
void Shader::use(){
	glUseProgram(ID);
}


// utility uniform functions
// ------------------------------------------------------------------------

void Shader::setBool(const char* name, bool value){

	GLint location;
#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
#endif
		location = glGetUniformLocation(ID, name);
	glUniform1i(location, (int)value);
}

// ------------------------------------------------------------------------
void Shader::setInt(const char* name, int value){
	GLint location;
#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
#endif
		location = glGetUniformLocation(ID, name);
	glUniform1i(location, value);
}

// ------------------------------------------------------------------------
void Shader::setFloat(const char* name, float value){
	GLint location;
#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
#endif
		location = glGetUniformLocation(ID, name);
	glUniform1f(location, value);
}

// ------------------------------------------------------------------------
void Shader::setVec2(const char* name, const glm::vec2 &value){
	GLint location;
#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
#endif
		location = glGetUniformLocation(ID, name);
	glUniform2fv(location, 1, &value[0]);
}

void Shader::setVec2(const char* name, float x, float y){
	GLint location;
#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
#endif
		location = glGetUniformLocation(ID, name);
	glUniform2f(location, x, y);
}

// ------------------------------------------------------------------------
void Shader::setVec3(const char* name, const glm::vec3 &value){
	GLint location;
#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
#endif
		location = glGetUniformLocation(ID, name);
	glUniform3fv(location, 1, &value[0]);
}

void Shader::setVec3(const char* name, float x, float y, float z){
	GLint location;
#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
#endif
		location = glGetUniformLocation(ID, name);
	glUniform3f(location, x, y, z);
}

// ------------------------------------------------------------------------
void Shader::setVec4(const char* name, const glm::vec4 &value){
	GLint location;
#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
#endif
		location = glGetUniformLocation(ID, name);
	glUniform4fv(location, 1, &value[0]);
}

void Shader::setVec4(const char* name, float x, float y, float z, float w){
	GLint location;
#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
#endif
		location = glGetUniformLocation(ID, name);
	glUniform4f(location, x, y, z, w);
}

// ------------------------------------------------------------------------
void Shader::setMat2(const char* name, const glm::mat2 &mat){
	GLint location;
#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
#endif
		location = glGetUniformLocation(ID, name);
	glUniformMatrix2fv(location, 1, GL_FALSE, &mat[0][0]);
}

// ------------------------------------------------------------------------
void Shader::setMat3(const char* name, const glm::mat3 &mat){
	GLint location;
#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
#endif
		location = glGetUniformLocation(ID, name);
	glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
}

//-------------------------------------------------------------------------
void Shader::setMat4(const char* name, const glm::mat4 &mat){
	GLint location;
#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
#endif
		location = glGetUniformLocation(ID, name);
	glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
}
