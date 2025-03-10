// FRAGMENT SHADER

#version 330

// LIGHTING
struct POINT
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};

struct SPOT
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	vec3 direction;
	float cutOff;
	float attenuation;
	float intensity;
	mat4 matrix;
};

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;

// Matrices
uniform mat4 matrixView;

// Light
uniform POINT lightPoint, lightPoint1, lightPoint2, lightPoint3;
uniform SPOT spotlight;
uniform float globalIntensity;

// Tectures
uniform sampler2D texture0;
uniform sampler2D texture1;

in vec4 position;
in vec3 normal;

in vec4 color;
out vec4 outColor;

in vec2 texCoord0;

in vec4 shadowCoord;
uniform sampler2DShadow shadowMap;

vec4 PointLight(POINT light, float intensity)
{
	// Calculate Point Light
	vec4 pColor = vec4(0, 0, 0, 1);

	// diffuse
	vec3 L = normalize(matrixView * vec4(light.position, 1) - position).xyz;
	float NdotL = dot(L, normal.xyz);
	pColor += vec4(light.diffuse * materialDiffuse, 1) * max(NdotL, 0);

	// specular
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal.xyz);
	float RdotV = dot(R, V);
	if (RdotV > 0)
		pColor += vec4(light.specular * materialSpecular * pow(max(RdotV, 0), materialShininess), 1) * intensity;

	// attenuation
	float distance = length(matrixView * vec4(light.position, 1) - position);
	float att = 1 / (0.03 * distance * distance);

	return pColor * att;
}

vec4 SpotLight(SPOT light)
{
	vec4 pColor = vec4(0, 0, 0, 1);

	// diffuse
	vec3 L = normalize(light.matrix * vec4(light.position, 1) - position).xyz;
	float NdotL = dot(L, normal.xyz);
	pColor += vec4(light.diffuse * materialDiffuse, 1) * max(NdotL, 0);

	// specular
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal.xyz);
	float RdotV = dot(R, V);
	if (RdotV > 0)
		pColor += vec4(light.specular * materialSpecular * pow(max(RdotV, 0), materialShininess), 1) * light.intensity;

	// spot factor
	vec3 D = normalize((mat3(light.matrix) * light.direction));
	float s1 = -dot(L, D);
	float angle = acos(s1);

	float spotFactor = (angle <= light.cutOff) ? pow(s1, light.attenuation) : 0.0;

	return spotFactor * pColor;
}


void main(void) 
{
	outColor = color;

	outColor += PointLight(lightPoint, globalIntensity);
	outColor += PointLight(lightPoint1, globalIntensity);
	outColor += PointLight(lightPoint2, globalIntensity);
	outColor += PointLight(lightPoint3, globalIntensity);

	outColor += SpotLight(spotlight);

	outColor *= texture(texture0, texCoord0);
	outColor *= texture(texture1, texCoord0);

	float shadow = 1.0;
	if (shadowCoord.w > 0)
		shadow = 0.5 + 0.5 * textureProj(shadowMap, shadowCoord);

	outColor *= shadow;
}
