// FRAGMENT SHADER

#version 330

// LIGHTING
struct POINT
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

// Matrices
uniform mat4 matrixView;

// Light
uniform POINT lightPoint, lightPoint1, lightPoint2, lightPoint3;

// Tectures
uniform sampler2D texture0;
uniform sampler2D texture1;

in vec4 position;
in vec3 normal;

in vec4 color;
out vec4 outColor;

in vec2 texCoord0;

vec4 PointLight(POINT light)
{
	// Calculate Point Light
	vec4 pColor = vec4(0, 0, 0, 0);

	vec3 lightPos = (matrixView * vec4(light.position, 1.0)).xyz;
	vec3 L = normalize(lightPos - vec3(position));

	float NdotL = dot(normal, L);
	pColor += vec4(materialDiffuse * light.diffuse, 1) * max(NdotL, 0);

	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);

	pColor += vec4(materialSpecular * light.specular * pow(max(RdotV, 0), shininess), 1);

	return pColor;
}

void main(void) 
{
	outColor = color;

	outColor += PointLight(lightPoint);
	outColor += PointLight(lightPoint1);
	outColor += PointLight(lightPoint2);
	outColor += PointLight(lightPoint3);

	outColor *= texture(texture0, texCoord0);
	outColor *= texture(texture1, texCoord0);
}
