uniform mat4 matProj, matView, matModel;

uniform float time;

attribute vec3 position;
attribute vec3 color;
attribute vec2 uv;

varying vec3 vColor;
varying vec2 vTexcoord;

void main() {
    vColor = (color * 0.75 + 0.25) + sin(time * 2.0) * 0.25;
	vTexcoord = uv;
    gl_Position = (matProj * matView * matModel) * vec4(position, 1.0);
}