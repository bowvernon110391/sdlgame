#ifdef GL_ES
precision mediump float;
#endif

#define MAX_LIGHTS 2

uniform vec3 ambientColor;
uniform vec3 lightColor[MAX_LIGHTS];
uniform vec3 lightAttenuation[MAX_LIGHTS];

uniform sampler2D texture0;

varying vec2 vTexcoord;
varying vec3 vNormal;
varying vec3 vLightdir[MAX_LIGHTS];

// compute a single point light diffuse color
vec3 computeDiffuseColor(vec3 lightdir, vec3 normal, vec3 lightcolor, vec3 attenuation) {
	float dist = length(lightdir);
	vec3 normLightdir = normalize(lightdir);
	float dp = max(0.0, dot(normLightdir, normal));
	float atten = 1.0 / (attenuation.r + dist * attenuation.g + dist * dist * attenuation.b);
	
	return lightcolor * (dp * atten);
}

// compute final color?
vec4 computeFinalColor(vec4 baseColor) {
	// ambient + diffuse * baseColor + specular
	vec3 diffuseColor = vec3(0.0);
	
	for (int i=0; i<MAX_LIGHTS; i++) {
		diffuseColor += computeDiffuseColor(vLightdir[i], vNormal, lightColor[i], lightAttenuation[i]);
	}
	
	vec4 finalAmbient = vec4(ambientColor, 1.0);
	vec4 finalDiffuse = vec4(diffuseColor, 1.0);
	
	return (finalAmbient + finalDiffuse) * baseColor;
}

void main() {
	gl_FragColor = computeFinalColor(texture2D(texture0, vTexcoord));
}