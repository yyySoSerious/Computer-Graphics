#version 330 core
out vec4 FragColor;

in vec3 vertexColor;
in vec2 TexCoord;

uniform vec4 ourColor;
uniform float factor;
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
   // FragColor = vec4(vertexColor, 1.0f);
   // FragColor = ourColor; //vertexColor; //vec4(1.0f, 0.5f, 0.2f, 1.0f);
   //FragColor = texture(texture1, TexCoord) * vec4(vertexColor, 1.0); //samples the texture color for the fragment
   FragColor = mix(texture(texture1, TexCoord), texture(texture2, vec2(1 - TexCoord.x, TexCoord.y)), factor);
}