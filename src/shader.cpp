#include "../headers/shader.h"
#include <unordered_map>

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

// activate the shader
// ------------------------------------------------------------------------
const void Shader::use(){
	glUseProgram(ID);
}
// utility uniform functions
// ------------------------------------------------------------------------
const void Shader::setBool(const std::string &name, bool value){
	GLint location;
	if (mapped)
		location = getUniformLocation(name);
	else
		location =glGetUniformLocation(ID, name.c_str());
	glUniform1i(location, (int)value);
}
// ------------------------------------------------------------------------
const void Shader::setInt(const std::string &name, int value){
	GLint location;
	if (mapped)
		location = getUniformLocation(name);
	else
		location =glGetUniformLocation(ID, name.c_str());
	glUniform1i(location, value);
}
// ------------------------------------------------------------------------
const void Shader::setFloat(const std::string &name, float value){
	GLint location;
	if (mapped)
		location = getUniformLocation(name);
	else
		location =glGetUniformLocation(ID, name.c_str());
	glUniform1f(location, value);
}
// ------------------------------------------------------------------------
const void Shader::setVec2(const std::string &name, const glm::vec2 &value){
	GLint location;
	if (mapped)
		location = getUniformLocation(name);
	else
		location =glGetUniformLocation(ID, name.c_str());
	glUniform2fv(location, 1, &value[0]);
}
const void Shader::setVec2(const std::string &name, float x, float y){
	GLint location;
	if (mapped)
		location = getUniformLocation(name);
	else
		location =glGetUniformLocation(ID, name.c_str());
	glUniform2f(location, x, y);
}
// ------------------------------------------------------------------------
const void Shader::setVec3(const std::string &name, const glm::vec3 &value){
	GLint location;
	if (mapped)
		location = getUniformLocation(name);
	else
		location =glGetUniformLocation(ID, name.c_str());
	glUniform3fv(location, 1, &value[0]);
}
const void Shader::setVec3(const std::string &name, float x, float y, float z){
	GLint location;
	if (mapped)
		location = getUniformLocation(name);
	else
		location =glGetUniformLocation(ID, name.c_str());
	glUniform3f(location, x, y, z);
}
// ------------------------------------------------------------------------
const void Shader::setVec4(const std::string &name, const glm::vec4 &value){
	GLint location;
	if (mapped)
		location = getUniformLocation(name);
	else
		location =glGetUniformLocation(ID, name.c_str());
	glUniform4fv(location, 1, &value[0]);
}
const void Shader::setVec4(const std::string &name, float x, float y, float z, float w){
	GLint location;
	if (mapped)
		location = getUniformLocation(name);
	else
		location =glGetUniformLocation(ID, name.c_str());
	glUniform4f(location, x, y, z, w);
}
// ------------------------------------------------------------------------
const void Shader::setMat2(const std::string &name, const glm::mat2 &mat){
	GLint location;
	if (mapped)
		location = getUniformLocation(name);
	else
		location =glGetUniformLocation(ID, name.c_str());
	glUniformMatrix2fv(location, 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
const void Shader::setMat3(const std::string &name, const glm::mat3 &mat){
	GLint location;
	if (mapped)
		location = getUniformLocation(name);
	else
		location =glGetUniformLocation(ID, name.c_str());
	glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
}
//-------------------------------------------------------------------------
const void Shader::setMat4(const std::string &name, const glm::mat4 &mat){
	GLint location;
	if (mapped)
		location = getUniformLocation(name);
	else
		location =glGetUniformLocation(ID, name.c_str());
	glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
}
