#version 330

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in mat4 instanceMatrix;

out VS_OUT {
    vec3 position;
    vec2 texCoord;
    vec3 normal;
} vs_out;

uniform mat4 rotationMatrix;
uniform mat4 model;
uniform mat4 view;

void main()
{
    gl_Position = view * model * vec4(aPosition, 1.0); //view * model * vec4(aPosition, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));
    vs_out.normal = normalize(vec3(view * rotationMatrix * vec4(aNormal, 0.0))); //in model-view space
    vs_out.position = vec3(model * vec4(aPosition, 1.0));
    vs_out.texCoord = aTexCoord;
}