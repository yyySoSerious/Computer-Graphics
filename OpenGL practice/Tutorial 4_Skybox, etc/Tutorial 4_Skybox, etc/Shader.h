#pragma once

#include <string>
#include<glm/gtc/matrix_transform.hpp>

class Shader
{
public:
	Shader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile = "NULL");
	~Shader();
	void activateShader();
	unsigned int getProgramId() const;
	void setUniformFloat(const std::string& name, float value) const;
	void setUniformInt(const std::string& name, int value) const;
	void setUniformUInt(const std::string& name, unsigned int value) const;
	void setUniformBool(const std::string& name, bool value) const;
	void setUniformVec2(const std::string& name, const glm::vec2& value) const;
	void setUniformVec3(const std::string& name, const glm::vec3& value) const;
	void setUniformVec4(const std::string& name, const glm::vec4& value) const;
	void setUniformMatrix2(const std::string& name, const glm::mat2& value) const;
	void setUniformMatrix3(const std::string& name, const glm::mat3& value) const;
	void setUniformMatrix4(const std::string& name, const glm::mat4& value) const;

private:
	unsigned int programId;
	unsigned int vShaderId;
	unsigned int fShaderId;
	unsigned int gShaderId;
	bool hasGShader;

	void loadShaders(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile);
	void compileShader(unsigned int shaderId, const char* shaderCode, const std::string& shaderName);
	void attachAndLinkShaders();
};

