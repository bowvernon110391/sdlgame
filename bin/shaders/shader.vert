struct PointLight {
	vec3 pos;
	vec3 color;
	vec3 radAttenInfluence;
};

uniform mat4 matMVP;
uniform mat4 matModelview;
uniform float time;
uniform vec3 scale;

uniform PointLight point;

attribute vec3 position;
attribute vec3 normal;
attribute vec2 uv;

varying vec2 vTexcoord;
varying vec3 vNormal;
varying vec3 vLightdir;

void main() {
	vec3 scaledPos = position * scale;

	vTexcoord = uv;
	vNormal = mat3(matModelview) * normal;
	vLightdir = point.pos - vec3(matModelview * vec4(scaledPos, 1.0));
	
    gl_Position = matMVP * vec4(scaledPos, 1.0);
}