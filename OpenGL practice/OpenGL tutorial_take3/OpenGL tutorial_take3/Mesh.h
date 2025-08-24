#pragma once

#include<glad/glad.h>
#include<glm/glm.hpp>
#include <string>
#include <vector>

#include "Shader.h"
#include <assimp/scene.h>


struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	//glm::vec3 tangent;
	//glm::vec3 bitangent;

	Vertex(const glm::vec3& pos = glm::vec3(0.0f), const glm::vec3& norm = glm::vec3(0.0f),
		const glm::vec2& tCoords = glm::vec3(0.0f)) : position(pos), normal(norm), texCoords(tCoords) {}
};

struct Texture {
	unsigned int id;
	aiTextureType type;
	std::string localPath;

	Texture(const unsigned int idVal = 0, const aiTextureType typeVal = aiTextureType_DIFFUSE, const std::string& path = ""):
	id(idVal), type(typeVal), localPath(path){}
};

class Mesh
{
public:
	//mesh data
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	Mesh(const std::vector<Vertex>& verticesVal, const std::vector<unsigned int> indicesVal,
		const std::vector<Texture>& texturesVal);
	void draw(const Shader& shader) const;

private:
	//render data
	unsigned int VAO, VBO, EBO;
	void setupMesh();
};

