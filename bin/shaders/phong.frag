#ifdef GL_ES
precision mediump float;
#endif

varying vec3 vNormal;	// normal in view space
varying vec3 vSunDirection;	// sun direction in view space
varying vec3 vEye;

// some scene data
uniform vec4 scene_ambient_color;
uniform vec4 sun_color;
uniform float sun_intensity;

// material data?
uniform float material_glossiness;
uniform float material_fresnel0;
uniform vec4 material_specular;
uniform vec4 material_diffuse;

const float MAX_SHININESS = 255.0;
const float GAMMA = 2.2;
const float PI = 3.141592653;

vec4 gammaEncode(vec4 color) {
	return vec4( pow(color.rgb, vec3(1.0/GAMMA)), color.a);
}

float schlickFresnel(float NDotH, float F0) {
	return F0 + (1.0 - F0) * pow(1.0-NDotH, 5.0);
}

vec3 blinnPhongConservative(vec3 albedo, vec3 ambient, vec3 sunColor, vec3 N, vec3 L, vec3 V, float gloss, float F0) {
	// for now, assume dielectric
	vec3 specColor = sunColor;
	
	// half vector
	vec3 H = normalize(L + V);
	
	// some terms
	float NDotL = max(dot(N, L), 0.0);
	float NDotH = max(dot(N, H), 0.0);
	
	// m in range [0..255]
	float m = max(0.0001, gloss * MAX_SHININESS);
	
	// blend to total diffuse when glossiness falls below F0
	float modifier = smoothstep(0.0, max(0.0001, F0), gloss);
	
	// to conserve energy better, acommodate fresnel factor
	float Rfh = schlickFresnel(NDotH, F0) * modifier;
	// and blinn phong normalization factor
	float normFactor = (m + 8.0) / 8.0;
	
	vec3 ambientTerm = ambient * albedo;
	vec3 diffuseTerm = albedo;
	vec3 specularTerm = specColor * (Rfh * pow(NDotH, m) * normFactor);
	
	// ambient term is mixed by (1.0-NDotL) amount (exist when no lighting occurs)
	vec3 finalColor = sunColor * ( (ambientTerm * (1.0-NDotL)) + ((diffuseTerm + specularTerm) * NDotL));
	
	return finalColor;
}

void main() {
	vec3 N = normalize(vNormal);
	vec3 V = normalize(vEye);
	vec3 L = vSunDirection;

	vec4 finalColor = vec4(1.0);
	finalColor.rgb = blinnPhongConservative(material_diffuse.rgb, scene_ambient_color.rgb, sun_color.rgb, N, L, V, material_glossiness, material_fresnel0);
	// finalColor.rgb = finalColor.rgb / (finalColor.rgb + vec3(1.0));
	// gamma encode
	gl_FragColor = gammaEncode(finalColor);
}