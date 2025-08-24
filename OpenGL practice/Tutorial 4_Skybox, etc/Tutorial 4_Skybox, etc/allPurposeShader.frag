#version 330 core

out vec4 FragColor;

in vec3 position;
in vec3 normal;
in vec2 texCoord;

uniform vec3 cameraPos;
uniform sampler2D texture1;
uniform samplerCube skybox;
uniform bool useSkybox;

void main()
{
	float refractedIndexRatio = 1.00 / 1.52 ; //air R.I / glass R.I.
	vec3 n = normalize(normal); //normalized normal
	vec3 I = normalize(cameraPos - position); //incidence vector (from fragPosition to viewer's position)
	vec3 R = -I + (2 * (dot(I, n) * n)); //reflected vector
	
	//my refraction algorithm, slower than regular refract because of trig functions
	float cosAngle = dot(-I, -n);
	float alpha = acos(cosAngle);
	float beta = asin(refractedIndexRatio * sin(alpha));

	if(beta > 90.0f){
		R = vec3(0.0f); //sets the refracted vector to point to the origin if vector turns to a reflection vector
	}
	else{
		vec3 x = -n * cosAngle; //projection of incidence vector onto the normal vector
		vec3 y = -I - x; //vector from end of x vector to end of incidence vector
		vec3 z = normalize(y) * length(x) * tan(beta); // vector from end of x vector to end of refraction vector (derived from 
											  // trig ratios and vector addition
		R = x + z; //refract(-I, n, refractedIndexRatio)	//refracted vector
	}
	//----------------------------------------------------------------------------------------------------

	if(useSkybox){
		FragColor = vec4(texture(skybox, R).rgb, 1.0); //reflected the skybox on cube
	}
	else
		FragColor = vec4(texture(texture1, texCoord));
}