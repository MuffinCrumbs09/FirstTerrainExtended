// VERTEX SHADER

#version 330

// Bone Transforms
#define MAX_BONES 100
uniform mat4 bones[MAX_BONES];

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
in ivec4 aBoneId;
in vec4 aBoneWeight;

out vec4 color;
out vec4 position;
out vec3 normal;

// Texture Coords
out vec2 texCoord0;

void CalcNormal(mat4 matrixBone)
{
	normal = normalize(mat3(matrixModelView) * mat3(matrixBone) * aNormal);
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
	mat4 matrixBone;
	if(aBoneWeight[0] == 0.0)
		matrixBone = mat4(1);
	else
		matrixBone = (bones[aBoneId[0]] * aBoneWeight[0] +
			bones[aBoneId[1]] * aBoneWeight[1] +
			bones[aBoneId[2]] * aBoneWeight[2] +
			bones[aBoneId[3]] * aBoneWeight[3]);

	CalcNormal(matrixBone);
	// calculate position
	position = matrixModelView * matrixBone * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;

	// calculate light
	color = vec4(0, 0, 0, 1);

	color += AmbientLight(lightAmbient);

	color += DirectionalLight(lightDir);

	texCoord0 = aTexCoord;
}