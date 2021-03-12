#define MAX_LIGHTS 2

// scene config
uniform mat4 matMVP;
uniform mat4 matModelview;
uniform vec3 scale;

attribute vec3 position;
attribute vec3 normal;
attribute vec2 uv;

varying vec2 vTexcoord;
varying vec3 vNormal;
varying vec3 vVertexPos[MAX_LIGHTS];

void main() {
	vec3 scaledPos = position * scale;

	vTexcoord = uv;
	vNormal = mat3(matModelview) * normal;
	
	// store light dir in viewspace (not normalized)
	for (int i=0; i<MAX_LIGHTS; i++) {
		vVertexPos[i] = vec3(matModelview * vec4(position, 1.0));
	}
	
    gl_Position = matMVP * vec4(scaledPos, 1.0);
}