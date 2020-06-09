uniform mat4 matProj, matView, matModel;

uniform float time;

attribute vec3 position;
attribute vec3 color;

varying vec3 vColor;

void main() {
    vColor = (color * 0.75 + 0.25) + sin(time * 2.0) * 0.25;

    gl_Position = (matProj * matView * matModel) * vec4(position, 1.0);
}