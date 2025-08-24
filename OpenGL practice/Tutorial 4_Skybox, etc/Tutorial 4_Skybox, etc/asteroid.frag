#version 330 core
out vec4 FragColor;

struct Material{
	sampler2D diffuseMap;
	sampler2D specularMap;
	sampler2D emissionMap;
	float shininess;
};

in vec2 texCoord;

uniform Material material;

void main()
{
	FragColor =  texture(material.diffuseMap, texCoord);
}