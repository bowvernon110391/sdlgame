#ifdef GL_ES
precision mediump float;
#endif

#define MAX_LIGHTS 2

// scene dependent
uniform vec3 ambientColor;
// uniform float sceneShininess;

// light data
uniform vec3 lightPos[MAX_LIGHTS]; // already in eye space
uniform vec3 lightColor[MAX_LIGHTS];
uniform vec3 lightAttenuation[MAX_LIGHTS];

uniform sampler2D texture0;

varying vec2 vTexcoord;
varying vec3 vNormal;
varying vec3 vVertexPos[MAX_LIGHTS];

// compute a single point light diffuse color
vec3 computeDiffuseColor(vec3 lightdir, vec3 normal, vec3 lightcolor, vec3 attenuation) {
	float dist = length(lightdir);
	vec3 normLightdir = normalize(lightdir);
	float dp = max(0.0, dot(normLightdir, normal));
	float atten = 1.0 / (attenuation.r + dist * attenuation.g + dist * dist * attenuation.b);
	
	return lightcolor * (dp * atten);
}

vec3 computeDiffuseAndSpecularTerm(vec3 vpos, vec3 lightpos, vec3 normal, vec3 attenuation) {
	// light direction
	vec3 lightdir = lightpos-vpos;
	float dist = length(lightdir);
	vec3 normLightdir = normalize(lightdir);
	float dp = max(0.0, dot(normLightdir, normal));
	float atten = 1.0 / (attenuation.r + dist * attenuation.g + dist * dist * attenuation.b);
	
	// compute specular term
	// compute half vec
	vec3 halfvec = normalize(-vpos);
	halfvec = normalize(halfvec + normLightdir);
	float baseSpec = max(0.0, dot(halfvec, normal)) * step(0.0, dp);
	
	// raise to say, 64
	float shininess = 255.0;
	float sp = pow(baseSpec, shininess);
	
	// store r = diffuse, g = specular
	vec3 diffspec = vec3(dp, sp, 0.0) * atten;
	
	return diffspec;
}

// compute final color?
vec4 computeFinalColor(vec4 baseColor) {
	// ambient + diffuse * baseColor + specular
	
	
	vec3 accumDiffuse = vec3(0.0);
	vec3 accumSpecular = vec3(0.0);
	
	for (int i=0; i<MAX_LIGHTS; i++) {
		// diffuseColor += computeDiffuseColor(lightPos[i]-vVertexPos[i], vNormal, lightColor[i], lightAttenuation[i]);
		vec3 diffspec = computeDiffuseAndSpecularTerm(vVertexPos[i], lightPos[i], normalize(vNormal), lightAttenuation[i]);
		
		accumDiffuse += lightColor[i] * diffspec.r;
		accumSpecular += lightColor[i] * diffspec.g;
	}
	
	vec4 finalAmbient = vec4(ambientColor, 1.0);
	vec4 finalDiffuse = vec4(accumDiffuse, 1.0);
	vec4 finalSpecular = vec4(accumSpecular, 1.0);
	
	return (finalAmbient + finalDiffuse) * baseColor + finalSpecular;
}

void main() {
	vec4 baseColor = texture2D(texture0, vTexcoord);
	// vec4 baseColor = vec4(1.0);
	gl_FragColor = computeFinalColor(baseColor);
}