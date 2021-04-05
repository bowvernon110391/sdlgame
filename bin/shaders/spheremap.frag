#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D texture0;
uniform vec4 material_diffuse;

varying vec2 vUv;
varying vec3 vNormal;
varying vec3 vEye;

void main() {
	vec3 eyeVector = normalize(-vEye);
	// reflect it?
	vec3 nNormal = normalize(vNormal);
	vec3 sc = -eyeVector + nNormal * (2.0 * dot(nNormal, eyeVector));
	
	//vec4 refMap = texture2D(texture0, sc.xy);
	//vec4 finalColor = refMap;
	
	gl_FragColor = texture2D(texture0, sc.xy * 0.5 + 0.5);
}