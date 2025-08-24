#include "Model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Model::Model(const char* path, const std::vector<glm::mat4>* modelMatrices, const bool gamma) : numModelMatrices(1),
gammaCorrection(gamma) {
	loadModel(path);
	if (modelMatrices) initInstancedModelMatrix(modelMatrices);//change this if you want to use this!
}

void Model::draw(const Shader& shader) const{
	for (unsigned int i = 0; i < meshes.size(); ++i)
		meshes[i].draw(shader, numModelMatrices); //draw given number( of this mesh using instanced model matrix
}

//loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector
void Model::loadModel(const std::string& path) {
	//read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace| aiProcess_FlipUVs);

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
		glm::vec3 normal(0.0f);
		glm::vec2 texCoords(0.0f);
		glm::vec3 tangent(0.0f);
		glm::vec3 bitangent(0.0f);

		//checks if the mesh contains normals
		if (mesh->HasNormals()) {
			normal.x = mesh->mNormals[i].x;
			normal.y = mesh->mNormals[i].y;
			normal.z = mesh->mNormals[i].z;
		}

		//checks if the mesh contains texture coordinates
		if (mesh->mTextureCoords[0]) {
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			texCoords.x = mesh->mTextureCoords[0][i].x;
			texCoords.y = mesh->mTextureCoords[0][i].y;

			//tangent
			tangent.x = mesh->mTangents[i].x;
			tangent.y = mesh->mTangents[i].y;
			tangent.z = mesh->mTangents[i].z;

			//bitangent
			bitangent.x = mesh->mBitangents[i].x;
			bitangent.y = mesh->mBitangents[i].y;
			bitangent.z = mesh->mBitangents[i].z;
		}

		Vertex vertex(position, normal, texCoords, tangent, bitangent);
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

		//normal maps 
		//The wavefront object format (.obj) exports normal maps slightly different from Assimp's conventions
		//as aiTextureType_NORMAL doesn't load normal maps, while aiTextureType_HEIGHT does. So, we use this for now
		std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT);
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

		//height maps
		std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT);
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
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
			Texture texture(textureFromFile(localPath.C_Str(), directory, gammaCorrection), type, localPath.C_Str());
			textures.push_back(texture);
			loadedTextures.push_back(texture);
		}
	}
	return textures;
}


unsigned textureFromFile(const char* localPath, const std::string &directory, bool gammaCorrection) {
	std::string textureFile = directory + '/' + std::string(localPath);
	return textureFromFile(textureFile.c_str(), gammaCorrection);
}


//This function generates a texture object, sets its texture parameters and loads the texture image for the object.
//It returns the initalized texture object.
//Note: it expects the image to be RGB or RGBA format
unsigned int textureFromFile(const char* textureFile, bool gammaCorrection) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID); //binds texture to target, GL_TEXTURE_2D

	//load image from file
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(textureFile, &width, &height, &nrChannels, 0);

	//create texture image for the bound texture object and generate mipmaps from the now attached texture image
	if (data) {
		GLint internalFormat = GL_RGB;
		GLenum dataFormat = GL_RGB;
		switch (nrChannels) {
		case 1:
			internalFormat = dataFormat = GL_RED;
			break;
		case 3:
			internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
			dataFormat = GL_RGB;
			break;
		case 4:
			internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
			dataFormat = GL_RGBA;
			break;
		}

		glBindTexture(GL_TEXTURE_2D, textureID); //binds texture to target, GL_TEXTURE_2D
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//sets texture wrapping and filtering options for the currently bound texture object
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//When using transparent textures you don't want to repeat
	   // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	   // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//---------------------------------------------------------------------

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		std::cout << "ERROR: Cannot load texture file: " << textureFile << std::endl;
	}

	//free texture data
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0); //unbind texture
	return textureID;
}

//This function generates a texture object, sets its texture parameters and loads the texture image for the object.
//It returns the initalized texture object.
//Note: it expects the image to be RGB format of float type
unsigned int textureFromFile_f(const char* textureFile, GLint internalFormat) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID); //binds texture to target, GL_TEXTURE_2D

	//load image from file
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	float *data = stbi_loadf(textureFile, &width, &height, &nrChannels, 0);

	//create texture image for the bound texture object and generate mipmaps from the now attached texture image
	if (data) {
		glBindTexture(GL_TEXTURE_2D, textureID); //binds texture to target, GL_TEXTURE_2D
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGB, GL_FLOAT, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//sets texture wrapping and filtering options for the currently bound texture object
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//When using transparent textures you don't want to repeat
	   // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	   // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//---------------------------------------------------------------------

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		std::cout << "ERROR: Cannot load texture file: " << textureFile << std::endl;
	}

	//free texture data
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0); //unbind texture
	return textureID;
}

//This function generates a cubemap object, sets its texture parameters and loads the cubemap faces  for the object.
//It returns the initalized cubemap object.
//Note: it expects the image to be RGB or RGBA format
unsigned int cubeMapFromFile(const std::vector<const char*>& faces, bool gammaCorrection) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	for (unsigned int i = 0; i < faces.size(); ++i) {
		//load texture face
		unsigned char* data = stbi_load(faces[i], &width, &height, &nrChannels, 0);

		//create texture image for each face of the cubemap
		if (data) {
			GLint internalFormat = GL_RGB;
			GLenum dataFormat = GL_RGB;
			switch (nrChannels) {
			case 1:
				internalFormat = dataFormat = GL_RED;
				break;
			case 3:
				internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
				dataFormat = GL_RGB;
				break;
			case 4:
				internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
				dataFormat = GL_RGBA;
				break;
			}

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
		}
		else {
			std::cout << "ERROR: Cannot load cubemap texture file: " << faces[i] << std::endl;
		}
		//--------------------------------------------------------------------------------------------------------------------

		//set cubemap texture parameters
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		//------------------------------------------------------------------------------

		//free texture data
		stbi_image_free(data);
	}

	return textureID;
}

void Model::initInstancedModelMatrix(const std::vector<glm::mat4>* modelMatrices) {
	numModelMatrices = modelMatrices->size();
	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, numModelMatrices * sizeof(glm::mat4), modelMatrices->data(), GL_STATIC_DRAW);

	for (unsigned int i = 0; i < meshes.size(); ++i){
		glBindVertexArray(meshes[i].VAO);
		std::size_t vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}
}