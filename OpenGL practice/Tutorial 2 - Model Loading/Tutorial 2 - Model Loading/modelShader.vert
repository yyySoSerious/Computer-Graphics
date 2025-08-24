#version 330

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 fragPos;
out vec2 texCoord;
out vec3 normal;

uniform mat4 rotationMatrix;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPosition, 1.0);
    normal = vec3(rotationMatrix * vec4(aNormal, 1.0));
    fragPos = vec3(model * vec4(aPosition, 1.0));
    texCoord = aTexCoord;
}