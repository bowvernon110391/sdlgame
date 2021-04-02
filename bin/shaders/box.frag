#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D texture0;

varying vec2 vTexcoord;
varying vec3 vNormal;	// normal in view space
varying vec3 vSunDirection;	// sun direction in view space

// some scene data
uniform vec4 scene_ambient_color;
uniform vec4 sun_diffuse_color;
uniform vec4 sun_specular_color;

// material data?
uniform float material_shininess;

vec4 computeFinalColor(vec4 baseDiffuse) {
	float dp = max(0.0, dot(vNormal, vSunDirection));
	
	// half vector of sun?
	vec3 halfVec = normalize((vSunDirection + vec3(0.0,0.0,1.0)) * 0.5);
	
	// specular term
	float spec = pow( max(0.0, dot(halfVec, vNormal)), material_shininess );
	
	// final is (amb + diff) * color + spec * sunspec
	vec4 specularTerm = sun_specular_color * spec;
	vec4 finalColor = (scene_ambient_color + (sun_diffuse_color * dp) ) * baseDiffuse + specularTerm;
	return finalColor;
}

void main() {
	vec4 baseColor = texture2D(texture0, vTexcoord);
	gl_FragColor = computeFinalColor(baseColor);
}