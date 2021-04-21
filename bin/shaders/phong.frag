#ifdef GL_ES
precision mediump float;
#endif

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
uniform vec4 material_diffuse;

vec4 computeFinalColor(vec4 baseDiffuse) {
	// gamma decode our color from texture	
	vec3 n_normal = normalize(vNormal);
	float dp = max(0.0, dot(n_normal, vSunDirection));
	
	// half vector of sun?
	vec3 halfVec = normalize((vSunDirection + vEye) * 0.5);
	
	// specular term (diffuse corrected too)
	float diffFactor = smoothstep(dp, 0.0, 0.1);
	float spec = pow( max(0.0, dot(halfVec, n_normal)), material_shininess ) * diffFactor;
	
	// final is (amb + diff) * color + spec * sunspec
	vec4 specularTerm = clamp((sun_specular_color * spec) * material_specular, 0.0, 1.0);
	vec4 finalColor = clamp((scene_ambient_color + (sun_diffuse_color * dp) ) * baseDiffuse + specularTerm, 0.0, 1.0);
	
	return finalColor;
}

void main() {
	gl_FragColor = computeFinalColor(material_diffuse);
}