#ifdef GL_ES
precision mediump float;
#endif

uniform vec4 material_diffuse;

void main() {
	gl_FragColor = material_diffuse;
}