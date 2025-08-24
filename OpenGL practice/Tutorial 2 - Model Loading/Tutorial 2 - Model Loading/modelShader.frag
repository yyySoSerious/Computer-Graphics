#version 330 core

out vec4 FragColor;

struct Material{
	sampler2D diffuseMap;
	sampler2D specularMap;
	sampler2D emissionMap;
	float shininess;
};

struct DirLight{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight{
	vec3 position;

	//attenuation
	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight{
	vec3 position;
	vec3 spotDirection;
	float cutOff;
	float outerCutOff;

	//attenuation
	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct LightComponents{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 fragPos;
in vec2 texCoord;
in vec3 normal;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight; 

void lightComponentsCalc(const LightComponents lc, const vec3 lightDir, out vec3 ambient, out vec3 diffuse, out vec3 specular);
vec3 directionalLight(const DirLight light);
vec3 pointLight(const PointLight light);
vec3 spotLight(const SpotLight light);

void main()
{
	vec4 diffuseTexture = texture(material.diffuseMap, texCoord);
	vec3 result = directionalLight(dirLight);
	FragColor = vec4(result, diffuseTexture.a);
}

void lightComponentsCalc(const LightComponents lc, const vec3 lightDir, out vec3 ambient, out vec3 diffuse, out vec3 specular){
	vec3 norm = normalize(normal);
	vec4 diffuseTexture = texture(material.diffuseMap, texCoord);
	vec4 specularTexture = texture(material.specularMap, texCoord);
	vec4 emissionTexture = vec4(0.0f);//texture(material.emissionMap, texCoord);

	// Calculates ambient color
	//float ambientStrength = 0.1;
	ambient = lc.ambient * diffuseTexture.rgb;//* material.ambient; // ambientStrength * lightColor;
	//---------------------------

	// Calculates diffusion color
	float diff = max(dot(norm, lightDir), 0.0);
	diffuse = lc.diffuse * ((diff  * diffuseTexture.rgb) + (1-diff) * emissionTexture.rgb);//* material.diffuse); //diff * lightColor;
	//-----------------------------

	// Calculates specular color
	//float specularStrength = 1; 
	vec3 viewDir = normalize(viewPos - fragPos); //from fragment to viewer's location 
	vec3 reflectDir = normalize(-lightDir + 2 * dot(lightDir, norm) * norm); //reflect(-lightDir, norm); //reflected light direction
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);//material.shininess);
	specular = lc.specular * (spec * specularTexture.rgb); //specularStrength * spec * lightColor;
	//-----------------------------------
}

vec3 directionalLight(const DirLight light){
	vec3 ambient, diffuse, specular;
	vec3 lightDir = normalize(-light.direction); //from object towards directional light
	LightComponents lc;
	lc.ambient = light.ambient;
	lc.diffuse = light.diffuse;
	lc.specular = light.specular;
	lightComponentsCalc(lc, lightDir, ambient, diffuse, specular);

	return vec3(ambient + diffuse + specular); 
}

vec3 pointLight(const PointLight light){
	vec3 ambient, diffuse, specular;

	vec3 lightDir = light.position - fragPos; //from object towards positional light source
	float distance = length(lightDir);
	lightDir = normalize(lightDir); 
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	LightComponents lc;
	lc.ambient = light.ambient;
	lc.diffuse = light.diffuse;
	lc.specular = light.specular;

	lightComponentsCalc(lc, lightDir, ambient, diffuse, specular);

	return vec3((ambient + diffuse + specular) * attenuation);
}

vec3 spotLight(const SpotLight light){
	vec3 ambient, diffuse, specular;

	vec3 lightDir = light.position - fragPos; //from object towards positional light source
	float distance = length(lightDir);
	lightDir = normalize(lightDir); 
	float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	LightComponents lc;
	lc.ambient = light.ambient;
	lc.diffuse = light.diffuse;
	lc.specular = light.specular;

	float thetaAsCosine = dot(-lightDir, normalize(light.spotDirection)); //direction reversed because we want the light direction to be towards object
	float epsilonAsCosine = light.cutOff - light.outerCutOff;
	float intensity = clamp((thetaAsCosine - light.outerCutOff) / epsilonAsCosine, 0.0, 1.0);
	lightComponentsCalc(lc, lightDir, ambient, diffuse, specular);
	diffuse *= intensity;
	specular *= intensity;
	return vec3((ambient + diffuse + specular) * attenuation); 
}