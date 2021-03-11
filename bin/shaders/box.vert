#define MAX_LIGHTS 2

// scene config
uniform mat4 matMVP;
uniform mat4 matModelview;
uniform vec3 scale;

// light data
uniform vec3 lightPos[MAX_LIGHTS]; // already in eye space

attribute vec3 position;
attribute vec3 normal;
attribute vec2 uv;

varying vec2 vTexcoord;
varying vec3 vNormal;
varying vec3 vLightdir[MAX_LIGHTS];

void main() {
	vec3 scaledPos = position * scale;

	vTexcoord = uv;
	vNormal = mat3(matModelview) * normal;
	
	// store light dir in viewspace (not normalized)
	for (int i=0; i<MAX_LIGHTS; i++) {
		vec3 viewVertexPos = vec3(matModelview * vec4(position, 1.0));
		vLightdir[i] = lightPos[i] - viewVertexPos;
	}
	
    gl_Position = matMVP * vec4(scaledPos, 1.0);
}