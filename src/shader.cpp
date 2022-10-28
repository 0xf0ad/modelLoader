#include "../headers/shader.h"
#include <bits/types/FILE.h>
#include <cstdlib>
#include <stdio.h>
#include <unordered_map>

#define IWASNOTCOOL         false // cuz I am cool, and i use C
#define CACHSHADERLOCATIONS false // idk why but it just give me worst results

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
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	// 2 - compile shaders
	// -------------------
	unsigned int vertex, fragment;
	int success;
	char infoLog[512];
	// compile the vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, true, &vShaderCode, NULL);
	glCompileShader(vertex);
	// check for errors durring compiling the vertex shader
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR : failed to compile the vertex shader : \n" <<infoLog<<'\n';
	}
	// compile the fragment shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, true, &fShaderCode, NULL);
	glCompileShader(fragment);
	// check for errors durring compiling the fragment shader
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "ERROR : failed to compile the fragment shader : \n" <<infoLog<<'\n';
	}
	// shader Program
	// attach the shaders to the program ID
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	// check linking errors
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if(!success){
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR : failed to link the shaders to the program : \n" <<infoLog<<'\n';
	}
	// delete shader after linking them
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}
#else /* I WAS COOL */

Shader::Shader(const char* vertexPath, const char* fragmentPath){
	// 1 - retrieve the vertex and fragment source code from filepath
	// --------------------------------------------------------------
	char *vertexCode, *fragmentCode;
	FILE *v_ShaderFile, *f_ShaderFile;
	uint v_StreamSize, f_StreamSize;

	
	// open file
	v_ShaderFile = fopen(vertexPath  , "r");
	f_ShaderFile = fopen(fragmentPath, "r");

	// error handlling
	if(!(v_ShaderFile && f_ShaderFile)){
		if(!v_ShaderFile)
			fprintf(stderr, "could not open vertex shader file, check if it exists on %s\n", vertexPath);
		else if(!f_ShaderFile)
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
		unsigned int i = 0;
		while(fgets((vertexCode + i), v_StreamSize, v_ShaderFile))
			i = ftell(v_ShaderFile);
		i = 0;
		while(fgets((fragmentCode + i), f_StreamSize, f_ShaderFile))
			i = ftell(f_ShaderFile);

		// close files
		fclose(v_ShaderFile);
		fclose(f_ShaderFile);
	}

	// 2 - compile shaders
	// -------------------
	unsigned int vertex, fragment;
	int success;
	char infoLog[512];

	// compile the vertex shader and free it's string as long as we dont need it
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, true, &vertexCode, NULL);
	free(vertexCode);
	glCompileShader(vertex);
	// check for errors durring compiling the vertex shader
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		fprintf(stderr, "ERROR : failed to compile the vertex shader : \n%s\n", infoLog);
	}

	// compile the fragment shader and free it's string as long as we dont need it
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, true, &fragmentCode, NULL);
	free(fragmentCode);
	glCompileShader(fragment);
	// check for errors durring compiling the fragment shader
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		fprintf(stderr, "ERROR : failed to compile the fragment shader : \n%s\n", infoLog);
	}


	// 3 - attach Program
	//-------------------
	// attach the shaders to the program ID
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);

	// check linking errors
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if(!success){
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		fprintf(stderr, "ERROR : failed to link the shaders to the program : \n%s\n", infoLog);
	}

	// delete shader after linking them
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}
#endif /* 'C'(see) I told you i was cool */

#if CACHSHADERLOCATIONS
GLint const Shader::getUniformLocation(const std::string& name){
	// if the uniform location is already cached
	if(Shader::uniformLocationCache.find(name) != Shader::uniformLocationCache.end()){
		return Shader::uniformLocationCache[name];
	}else{
		// we store the uniform location if its not already
		GLint location = glGetUniformLocation(ID, name.c_str());
		Shader::uniformLocationCache[name] = location;
		return location;
	}
}
#endif


// activate the shader
// ------------------------------------------------------------------------
const void Shader::use(){
	glUseProgram(ID);
}


// utility uniform functions
// ------------------------------------------------------------------------
const void Shader::setBool(const std::string &name, bool value){
	GLint location;
	#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
	#endif
		location = glGetUniformLocation(ID, name.c_str());
	glUniform1i(location, (int)value);
}

// ------------------------------------------------------------------------
const void Shader::setInt(const std::string &name, int value){
	GLint location;
	#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
	#endif
		location = glGetUniformLocation(ID, name.c_str());
	glUniform1i(location, value);
}

// ------------------------------------------------------------------------
const void Shader::setFloat(const std::string &name, float value){
	GLint location;
	#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
	#endif
		location = glGetUniformLocation(ID, name.c_str());
	glUniform1f(location, value);
}

// ------------------------------------------------------------------------
const void Shader::setVec2(const std::string &name, const glm::vec2 &value){
	GLint location;
	#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
	#endif
		location = glGetUniformLocation(ID, name.c_str());
	glUniform2fv(location, 1, &value[0]);
}

const void Shader::setVec2(const std::string &name, float x, float y){
	GLint location;
	#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
	#endif
		location = glGetUniformLocation(ID, name.c_str());
	glUniform2f(location, x, y);
}

// ------------------------------------------------------------------------
const void Shader::setVec3(const std::string &name, const glm::vec3 &value){
	GLint location;
	#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
	#endif
		location = glGetUniformLocation(ID, name.c_str());
	glUniform3fv(location, 1, &value[0]);
}

const void Shader::setVec3(const std::string &name, float x, float y, float z){
	GLint location;
	#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
	#endif
		location = glGetUniformLocation(ID, name.c_str());
	glUniform3f(location, x, y, z);
}

// ------------------------------------------------------------------------
const void Shader::setVec4(const std::string &name, const glm::vec4 &value){
	GLint location;
	#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
	#endif
		location = glGetUniformLocation(ID, name.c_str());
	glUniform4fv(location, 1, &value[0]);
}

const void Shader::setVec4(const std::string &name, float x, float y, float z, float w){
	GLint location;
	#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
	#endif
		location = glGetUniformLocation(ID, name.c_str());
	glUniform4f(location, x, y, z, w);
}

// ------------------------------------------------------------------------
const void Shader::setMat2(const std::string &name, const glm::mat2 &mat){
	GLint location;
	#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
	#endif
		location = glGetUniformLocation(ID, name.c_str());
	glUniformMatrix2fv(location, 1, GL_FALSE, &mat[0][0]);
}

// ------------------------------------------------------------------------
const void Shader::setMat3(const std::string &name, const glm::mat3 &mat){
	GLint location;
	#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
	#endif
		location = glGetUniformLocation(ID, name.c_str());
	glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
}

//-------------------------------------------------------------------------
const void Shader::setMat4(const std::string &name, const glm::mat4 &mat){
	GLint location;
	#if CACHSHADERLOCATIONS
	if (mapped)
		location = getUniformLocation(name);
	else
	#endif
		location = glGetUniformLocation(ID, name.c_str());
	glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
}
