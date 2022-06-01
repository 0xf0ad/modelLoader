#include "headers/shader.hpp"

Shader::Shader(const char* vertexPath, const char* fragmentPath){
	// 1 - retrieve the vertex and fragment source code from filepath
	// --------------------------------------------------------------
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	//ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
	try{
		//open file
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
	//compile the vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, true, &vShaderCode, NULL);
	glCompileShader(vertex);
	//check for errors durring compiling the vertex shader
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR : failed to compile the vertex shader : " << infoLog << '\n';
	}
	//compile the fragment shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, true, &fShaderCode, NULL);
	glCompileShader(fragment);
	//check for errors durring compiling the fragment shader
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if(!success){
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "ERROR : failed to compile the vertex shader : " << infoLog << '\n';
	}
	//shader Program
	//attach the shaders to the program ID
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	//check linking errors
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if(!success){
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR : failed to link the shaders to the program : " << infoLog << '\n';
	}
	//delete shader after linking them
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

// activate the shader
// ------------------------------------------------------------------------
const void Shader::use(){ 
	glUseProgram(ID); 
}
// utility uniform functions
// ------------------------------------------------------------------------
const void Shader::setBool(const std::string &name, bool value){         
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
const void Shader::setInt(const std::string &name, int value){ 
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
const void Shader::setFloat(const std::string &name, float value){ 
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
// ------------------------------------------------------------------------
const void Shader::setVec2(const std::string &name, const glm::vec2 &value){ 
	glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
const void Shader::setVec2(const std::string &name, float x, float y){ 
	glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}
// ------------------------------------------------------------------------
const void Shader::setVec3(const std::string &name, const glm::vec3 &value){ 
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
const void Shader::setVec3(const std::string &name, float x, float y, float z){ 
	glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}
// ------------------------------------------------------------------------
const void Shader::setVec4(const std::string &name, const glm::vec4 &value){ 
	glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
const void Shader::setVec4(const std::string &name, float x, float y, float z, float w){ 
	glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}
// ------------------------------------------------------------------------
const void Shader::setMat2(const std::string &name, const glm::mat2 &mat){
	glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
const void Shader::setMat3(const std::string &name, const glm::mat3 &mat){
	glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
const void Shader::setMat4(const std::string &name, const glm::mat4 &mat){
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
