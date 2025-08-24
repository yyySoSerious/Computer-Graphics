#include "Model.h"

Model::Model(const char* path, const bool gamma) : gammaCorrection(gamma) {//not sure what this inheritance is {
	loadModel(path);
}

void Model::draw(const Shader& shader) const{
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].draw(shader);
}

//loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector
void Model::loadModel(const std::string& path) {
	//read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace);// | aiProcess_FlipUVs);

	//checks whether the scene or the root node of the scene is null. 
	//It also checks if the returned data is incomplete, that is, if the AI_SCENE_FLAGS_INCOMPLETE flag is set.
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
}

//process a node in recursive fashion. Processes each individual mesh 
//locaed at then node and repeats this process on its children nodes (if anyP
void Model::processNode(const aiNode* node, const aiScene* scene) {
	//process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	//process node's children
	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	//process vertex poistions, normals and texture coordinates
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		glm::vec3 position(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		glm::vec3 normal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		glm::vec2 texCoords(0.0f);

		//checks if the mesh contains texture coordinates
		if (mesh->mTextureCoords[0]) {
			texCoords.x = mesh->mTextureCoords[0][i].x;
			texCoords.y = mesh->mTextureCoords[0][i].y;
		}

		Vertex vertex(position, normal, texCoords);
		vertices.push_back(vertex);
	}

	//process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
			indices.push_back(face.mIndices[j]);
	}
	//process material
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		//normal maps and height should be added here!
	}
	return Mesh(vertices, indices, textures);
}


std::vector<Texture> Model::loadMaterialTextures(aiMaterial* material, aiTextureType type) {
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < material->GetTextureCount(type);  ++i) {
		aiString localPath;
		material->GetTexture(type, i, &localPath);
		bool skip = false;
		for (unsigned int j = 0; j < loadedTextures.size(); ++j) {
			if (std::strcmp(loadedTextures[j].localPath.c_str(), localPath.C_Str()) == 0) {
				textures.push_back(loadedTextures[j]);
				skip = true;
				break;
			}
		}
		if (!skip) {
			Texture texture(textureFromFile(localPath.C_Str(), directory), type, localPath.C_Str());
			textures.push_back(texture);
			loadedTextures.push_back(texture);
		}
	}
	return textures;
}


unsigned textureFromFile(const char* localPath, const std::string &directory, bool gamma) {
	std::string textureFile = directory + '/' + std::string(localPath);

	unsigned int textureId;
	glGenTextures(1, &textureId);

	//load image from file
	int width, height, nrChannels;
	unsigned char* data = stbi_load(textureFile.c_str(), &width, &height, &nrChannels, 0);

	//create texture image for the bound texture object and generate mipmaps from the now attached texture image
	if (data) {
		GLenum format = GL_RGB;
		switch (nrChannels) {
		case 1:
			format = GL_RED;
			break;
		case 4:
			format = GL_RGBA;
			break;
		}

		glBindTexture(GL_TEXTURE_2D, textureId); //binds texture to target, GL_TEXTURE_2D
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//sets texture wrapping and filtering options for the currently bound texture object
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		std::cout << "ERROR: Cannot load texture file: " << textureFile << std::endl;
	}

	//free texture data
	stbi_image_free(data);
	return textureId;
}