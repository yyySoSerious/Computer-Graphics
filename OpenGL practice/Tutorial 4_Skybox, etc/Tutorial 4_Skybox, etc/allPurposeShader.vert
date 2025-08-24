#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out vec3 position;
out vec3 normal;
out vec2 texCoord;

uniform mat4 rotationMatrix;
uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;


void main()
{
    position = (model * vec4(aPos, 1.0)).xyz;
    normal = mat3(rotationMatrix) * aNormal; //mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(position, 1.0);
    texCoord = aTexCoord;
}