#pragma once

#include <vector>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Shader.h"
#include "Mesh.h"

unsigned textureFromFile(const char* localPath, const std::string& directory, bool gamma = false);
unsigned int textureFromFile(const char* textureFile);
unsigned int cubeMapFromFile(const std::vector<const char*>& faces);

class Model
{
public:
	Model(const char* path, const std::vector<glm::mat4>* modelMatrices = NULL, const bool gamma = false);
	void draw(const Shader& shader) const;

private:
	//model data
	std::vector<Texture> loadedTextures;
	std::vector <Mesh > meshes;
	std::string directory;
	bool gammaCorrection;
	std::size_t numModelMatrices;

	void loadModel(const std::string& path);
	void processNode(const aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* material, aiTextureType type);
	void initInstancedModelMatrix(const std::vector<glm::mat4>* modelMatrices);
};

