#version 330 core
out vec4 FragColor;
  
in vec2 texCoord;

uniform sampler2D screenTexture;

void main()
{ 
    FragColor = texture(screenTexture, texCoord);

    //inversion
    //FragColor = vec4(vec3(1 - FragColor), 1.0);

    //grayscale
    float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
    FragColor = vec4(average, average, average, 1.0);
}