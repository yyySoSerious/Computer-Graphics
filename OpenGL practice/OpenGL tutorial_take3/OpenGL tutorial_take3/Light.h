#ifndef LIGHT_H
#define LIGHT_H

#include<glm/glm.hpp>

struct Light {
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;

	Light(const glm::vec3& ambientVal, const glm::vec3& diffuseVal, const glm::vec3& specularVal) :
		ambient(ambientVal), diffuse(diffuseVal), specular(specularVal) {}

	virtual ~Light() = 0 {};
};

struct DirLight : Light {
	glm::vec3 direction;

	//default set to a white directional light
	DirLight(const glm::vec3& dir = glm::vec3(-0.2f, -1.0f, -0.3f),
		const glm::vec3& ambientVal = glm::vec3(0.2f, 0.2f, 0.2f),
		const glm::vec3& diffuseVal = glm::vec3(1.0f, 1.0f, 1.0f),
		const glm::vec3& specularVal = glm::vec3(1.0f, 1.0f, 1.0f)) :

		Light(ambientVal, diffuseVal, specularVal), direction(dir) {}

	virtual ~DirLight() {}
};

struct PointLight : Light {
	glm::vec3 position;

	//attenuation parameters
	float constant;
	float linear;
	float quadratic;

	//default set to white, point light, placed at origin with a range of 50
	PointLight(const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
		const float constVal = 1.0f, const float linearVal = 0.09f, const float quadVal = 0.032f,
		const glm::vec3& ambientVal = glm::vec3(0.2f, 0.2f, 0.2f),
		const glm::vec3& diffuseVal = glm::vec3(1.0f, 1.0f, 1.0f),
		const glm::vec3& specularVal = glm::vec3(1.0f, 1.0f, 1.0f)) :

		Light(ambientVal, diffuseVal, specularVal), position(pos), constant(constVal), linear(linearVal),
		quadratic(quadVal) {}

	virtual ~PointLight() {}
};

struct SpotLight : PointLight {
	glm::vec3 spotDirection;
	float cutOff;
	float outerCutOff;

	//default set to a white flash light, looking at origin with a range of 7 and a cut-off angle of 12.5 degrees
	SpotLight(const glm::vec3& pos = glm::vec3(3.0f, 3.0f, 3.0f),
		const float constVal = 1.0f, const float linearVal = 0.7f, const float quadVal = 1.8f,
		const glm::vec3& spotDir = glm::vec3(-3.0f, -3.0f, -3.0f), 
		const float cutOffVal= glm::cos(glm::radians(12.5f)), const float outerCutOffVal= glm::cos(glm::radians(17.5f)),
		const glm::vec3& ambientVal = glm::vec3(0.2f, 0.2f, 0.2f),
		const glm::vec3& diffuseVal = glm::vec3(1.0f, 1.0f, 1.0f),
		const glm::vec3& specularVal = glm::vec3(1.0f, 1.0f, 1.0f)) :

		PointLight(pos, constVal, linearVal, quadVal, ambientVal, diffuseVal, specularVal),
		spotDirection(spotDir), cutOff(cutOffVal), outerCutOff(outerCutOffVal) {}

	virtual ~SpotLight() {}
};

#endif