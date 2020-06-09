#version 100

uniform mat4 matProj, matView, matWorld;

attribute vec3 position;
attribute vec3 color;

varying vec3 vColor;

void main() {
    vColor = color;

    gl_Position = matProj * matView * matWorld * vec4(position, 1.0);
}