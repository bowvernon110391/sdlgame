#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D tex0;

varying vec3 vColor;
varying vec2 vTexcoord;

void main() {
    gl_FragColor = vec4(vColor, 1.0) * texture2D(tex0, vTexcoord);
}