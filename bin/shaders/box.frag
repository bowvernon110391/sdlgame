#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D texture0;

varying vec2 vTexcoord;
varying vec3 vNormal;	// normal in view space
varying vec3 vSunDirection;	// sun direction in view space
varying vec3 vEye;

// some scene data
uniform vec4 scene_ambient_color;
uniform vec4 sun_diffuse_color;
uniform vec4 sun_specular_color;

// material data?
uniform float material_shininess;
uniform vec4 material_specular;

const float GAMMA = 2.2;

vec4 computeFinalColor(vec4 baseDiffuse) {
	// gamma decode our color from texture
	vec4 gamma_decoded = pow(baseDiffuse, vec4(vec3(GAMMA), 1.0));
	
	vec3 n_normal = normalize(vNormal);
	float dp = max(0.0, dot(n_normal, vSunDirection));
	
	// half vector of sun?
	vec3 halfVec = normalize((vSunDirection + vEye) * 0.5);
	
	// specular term
	float spec = pow( max(0.0, dot(halfVec, n_normal)), material_shininess );
	
	// final is (amb + diff) * color + spec * sunspec
	vec4 specularTerm = (sun_specular_color * spec) * material_specular;
	vec4 finalColor = (scene_ambient_color + (sun_diffuse_color * dp) ) * gamma_decoded + specularTerm;
	
	return pow(finalColor, vec4(vec3(1.0/GAMMA), 1.0));
}

void main() {
	vec4 baseColor = texture2D(texture0, vTexcoord);
	gl_FragColor = computeFinalColor(baseColor);
}