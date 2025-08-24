#include "Shader.h"

#include "Shader.h"
#include<glad/glad.h>
#include <iostream>
#include <fstream>
#include <limits>
#include <filesystem>


//returns the file as a string, if it is not valid, it outputs error message, when required and returns NULL
char* readFile(const char* fileName) {
	if (fileName == NULL)
		return NULL;
	std::ifstream file(fileName, std::ios::binary);
	if (!file) {
		std::cerr << "ERROR: Cannot open file: " << fileName << std::endl;
		return NULL;
	}

	file.seekg(0, file.end);
	std::streamoff fileLength = file.tellg();
	file.seekg(0, file.beg);

	//int bufferSize = fileLength + 1;
	char * buffer = new char[fileLength + 1];
	file.read(buffer, fileLength);

	if (!file) {
		std::cerr << "ERROR: Cannot read file: "<< fileName << std::endl;
		return NULL;
	}
	buffer[fileLength] = '\0';
	file.close();
	return buffer;
}

Shader::Shader(const char* vShaderFile, const char* fshaderFile, const char* gShaderFile) : hasGShader(true){
	loadShaders(vShaderFile, fshaderFile, gShaderFile);
}

Shader::~Shader() {
	glDetachShader(programId, vShaderId);
	glDetachShader(programId, fShaderId);
	if (hasGShader)	glDetachShader(programId, gShaderId);

	glDeleteShader(vShaderId);
	glDeleteShader(fShaderId);
	if (hasGShader)	glDeleteShader(gShaderId);
	glDeleteProgram(programId);
}

void Shader::loadShaders(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile) {
	//create shader objects
	vShaderId = glCreateShader(GL_VERTEX_SHADER);
	fShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	//--------------------

	//load the shader code as strings
	char* vShaderCode = readFile(vShaderFile);
	char* fShaderCode = readFile(fShaderFile);

	std::string vertexShaderName = "Vertex Shader: ";
	vertexShaderName += vShaderFile;
	std::string fragmentShaderName = "Fragment Shader: ";
	fragmentShaderName += fShaderFile;

	if (vShaderCode == NULL){
		std::cerr << "SHADER ERROR: " << vertexShaderName << std::endl;
		return;
	}

	if (fShaderCode == NULL) {
		std::cerr << "SHADER ERROR: " << fragmentShaderName << std::endl;
		return;
	}

	//compile vertex and fragment shader code
	compileShader(vShaderId, vShaderCode, vertexShaderName);
	compileShader(fShaderId, fShaderCode, fragmentShaderName);
	//---------------------------------------------------------------

	delete[] vShaderCode;
	delete[] fShaderCode;

	//load geometry shader if it exists
	if (gShaderFile != "NULL") {
		hasGShader = true;
		gShaderId = glCreateShader(GL_GEOMETRY_SHADER);
		char* gShaderCode = readFile(gShaderFile);

		std::string geometryShaderName = "Geometry Shader: ";
		geometryShaderName += gShaderFile;

		if (gShaderCode == NULL) {
			std::cerr << "SHADER ERROR: " << geometryShaderName << std::endl;
			return;
		}
		compileShader(gShaderId, gShaderCode, geometryShaderName);
		delete[] gShaderCode;
	}
	//-------------------------------------------------------------
	attachAndLinkShaders();
}

void Shader::compileShader(unsigned int shaderId, const char* shaderCode, const std::string& shaderName) {
	//store the shader code in the shader objects
	glShaderSource(shaderId, 1, &shaderCode, NULL);
	glCompileShader(shaderId);
	//----------------------------

	//outputs error message if there is a compilation error
	char infoLog[512];
	int status;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
		std::cerr << "COMPILATION ERROR FOR: " << shaderName << std::endl << infoLog << std::endl;
	};
}

void Shader::attachAndLinkShaders() {
	char infoLog[512];
	int status;

	programId = glCreateProgram();
	glAttachShader(programId, vShaderId);
	glAttachShader(programId, fShaderId);
	if(hasGShader) glAttachShader(programId, gShaderId);
	glLinkProgram(programId);

	//outputs error message if there is a Linking error
	glGetProgramiv(programId, GL_LINK_STATUS, &status);
	if (!status)
	{
		glGetProgramInfoLog(programId, 512, NULL, infoLog);
		std::cout << "LINKING ERROR\n" << infoLog << std::endl;
	}
	//-------------------------------
}

void Shader::activateShader() {
	glUseProgram(programId);
}

unsigned int Shader::getProgramId() const{
	return programId;
}

void Shader::setUniformFloat(const std::string& name, float value) const{
	glUniform1f(glGetUniformLocation(programId, name.c_str()), value);
}

void Shader::setUniformInt(const std::string& name, int value) const{
	glUniform1i(glGetUniformLocation(programId, name.c_str()), value);
}

void Shader::setUniformUInt(const std::string& name, unsigned int value) const{
	glUniform1ui(glGetUniformLocation(programId, name.c_str()), value);
}

void Shader::setUniformBool(const std::string& name, bool value) const{
	glUniform1ui(glGetUniformLocation(programId, name.c_str()), value);
}

void Shader::setUniformVec2(const std::string& name, const glm::vec2& value) const{
	glUniform2fv(glGetUniformLocation(programId, name.c_str()), 1, &value[0]);
}
void Shader::setUniformVec3(const std::string& name, const glm::vec3& value) const{
	glUniform3fv(glGetUniformLocation(programId, name.c_str()), 1, &value[0]);
}

void Shader::setUniformVec4(const std::string& name, const glm::vec4& value) const{
	glUniform4fv(glGetUniformLocation(programId, name.c_str()), 1, &value[0]);
}

void Shader::setUniformMatrix2(const std::string& name, const glm::mat2& value) const{
	glUniformMatrix2fv(glGetUniformLocation(programId, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

void Shader::setUniformMatrix3(const std::string& name, const glm::mat3& value) const{
	glUniformMatrix3fv(glGetUniformLocation(programId, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

void Shader::setUniformMatrix4(const std::string& name, const glm::mat4& value) const{
	glUniformMatrix4fv(glGetUniformLocation(programId, name.c_str()), 1, GL_FALSE, &value[0][0]);
}
