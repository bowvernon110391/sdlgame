#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D texture0;
uniform sampler2D texture1;

varying vec2 vTexcoord;
varying vec2 vTexcoord2;

void main() {
	vec4 baseColor = texture2D(texture0, vTexcoord);
	vec4 lightColor = texture2D(texture1, vTexcoord2);

	gl_FragColor = baseColor * lightColor;
	// gl_FragColor = vec4(1.0);
}