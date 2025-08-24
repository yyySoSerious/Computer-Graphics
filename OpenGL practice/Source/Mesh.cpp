#include "Mesh.h"

Mesh::Mesh(const std::vector<Vertex>& verticesVal, const std::vector<unsigned int> indicesVal,
	const std::vector<Texture>& texturesVal) : vertices(verticesVal), indices(indicesVal), textures(texturesVal){

	setupMesh();
}

//draw given number of this mesh using instanced model matrix
void Mesh::draw(const Shader& shader, unsigned int num) const {
	unsigned int diffuseIndex = 1;
	unsigned int specularIndex = 1;
	for (unsigned int texUnit = 0; texUnit < textures.size(); ++texUnit) {
		glActiveTexture(GL_TEXTURE0 + texUnit); //activates the right texture unit
		glBindTexture(GL_TEXTURE_2D, textures[texUnit].id);
		
		std::string samplerName = "material.";

		switch (textures[texUnit].type) {
		case aiTextureType_DIFFUSE:
			samplerName += "diffuseMap";// +std::to_string(diffuseIndex++);
			break;
		case aiTextureType_SPECULAR:
			samplerName += "specularMap";// +std::to_string(specularIndex++);
			break;
		case aiTextureType_HEIGHT:
			samplerName += "heightMap";// +std::to_string(specularIndex++);
			break;
		}

		shader.setUniformInt(samplerName, texUnit);
	}
	glActiveTexture(GL_TEXTURE0); //reset back to default unit

	//draw mesh
	glBindVertexArray(VAO);
	glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, num);
	glBindVertexArray(0); //unbinds vao
}

void Mesh::setupMesh() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	//link position attribute in the vertex data to the shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	//link normal attribute in the vertex data to the shader
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));

	//link text coords attribute in the vertex data to the shader
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoords));
	glEnableVertexAttribArray(2);

	//link tangent attribute in the vertex data to the shader
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
	glEnableVertexAttribArray(3);

	//link bitangent attribute in the vertex data to the shader
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
	glEnableVertexAttribArray(4);

	glBindVertexArray(0);
}