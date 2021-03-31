// scene config
uniform mat4 matMVP;

attribute vec3 position;
attribute vec2 uv;
attribute vec2 uv2;

varying vec2 vTexcoord;
varying vec2 vTexcoord2;

void main() {
	vTexcoord = uv;
	vTexcoord2 = uv2;
		
    gl_Position = matMVP * vec4(position, 1.0);
}