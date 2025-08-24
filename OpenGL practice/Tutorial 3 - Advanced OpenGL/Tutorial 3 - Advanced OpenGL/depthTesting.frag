#version 330 core

out vec4 FragColor;

float near = 0.1;
float far = 100.0;

float linearizeDepth(float depth){
	return (near * far) / (far + depth * (near - far));
}

in vec3 fragPos;
in vec2 texCoord;

uniform sampler2D texture1;

void main()
{
	float depth = linearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    FragColor = vec4(vec3(depth), 1.0);
	//FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
	vec4 texColor = texture(texture1, texCoord);
	//if(texColor.a < 0.1)
		//discard;
	FragColor = texColor;
}