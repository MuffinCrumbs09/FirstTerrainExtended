// VERTEX SHADER

#version 330

// Light declarations
struct AMBIENT
{
	vec3 color;
};

struct DIRECTIONAL
{
	vec3 direction;
	vec3 diffuse;
};

// Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

// Lights
uniform AMBIENT lightAmbient;
uniform DIRECTIONAL lightDir;

in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoord;

out vec4 color;
out vec4 position;
out vec3 normal;

// Texture Coords
out vec2 texCoord0;

void CalcNormal()
{
	normal = normalize(mat3(matrixModelView) * aNormal);
}

vec4 AmbientLight(AMBIENT light)
{
	// Calculate Ambient Light
	return vec4(materialAmbient * light.color, 1);
}

vec4 DirectionalLight(DIRECTIONAL light)
{
	// Calculate Dir Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(mat3(matrixView) * light.direction);
	float NdotL = dot(normal, L);
	color += vec4(materialDiffuse * light.diffuse, 1) * max(NdotL, 0);
	return color;
}

void main(void)
{
CalcNormal();
// calculate position
position = matrixModelView * vec4(aVertex, 1.0);
gl_Position = matrixProjection * position;

// calculate light
color = vec4(0, 0, 0, 1);

color += AmbientLight(lightAmbient);

color += DirectionalLight(lightDir);

texCoord0 = aTexCoord;
}