#ifdef GL_ES
precision mediump float;
#endif

struct PointLight {
	vec3 pos;
	vec3 color;
	vec3 radAttenInfluence;
};

uniform sampler2D tex0;
uniform PointLight point;

varying vec2 vTexcoord;
varying vec3 vNormal;
varying vec3 vLightdir;	// lightpos - pos in viewspace not normalized

vec4 computeFinalColor(vec4 col, vec3 normal, vec3 lightdir, PointLight l) {
	float d = length(lightdir);
	lightdir = normalize(lightdir);
	
	float diffuse = max(0, dot(normal, lightdir));
	
	// compute attenuation
	float r = l.radAttenInfluence.r;
	float denom = max((r-d)/r, 0);
	// denom = denom * denom;
	
	vec4 diffuseColor = vec4(l.color * (diffuse * denom), 1.0);
	
	return col * diffuseColor;
}

void main() {
	vec4 baseColor = texture2D(tex0, vTexcoord);
    gl_FragColor = computeFinalColor(baseColor, vNormal, vLightdir, point);
}