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
const float PI = 3.14159235;

vec4 gammaDecode(vec4 color) {
	return vec4(pow(color.xyz, vec3(GAMMA)), color.a);
}

vec4 gammaEncode(vec4 color) {
	return vec4(pow(color.xyz, vec3(1.0/GAMMA)), color.a);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

// compute our final color, baseDiffuse and baseSpecular assumed to be in linear space
vec3 computeFinalColor(vec3 albedo, vec3 ambient, float glossiness, float F0) {
	// INPUTS
	//----------------------------------------------------------------------	
	// light?
	vec3 L = vSunDirection;
	// N = normal
	vec3 N = normalize(vNormal);
	// V = eye vector
	vec3 V = normalize(vEye);
	// H = half vector between eye and sun dir
	vec3 H = normalize(L + V);
	
	
	// roughness
	float roughness = 1.0 - glossiness;
	
	// metal?
	float metallic = 0.0;
	
	// radiance 
	vec3 radiance = sun_color.rgb * sun_intensity;
	
	// cook torrance
	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0), vec3(F0));
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;
	
	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = numerator / max(denominator, 0.001);
	
	float NDotL = max(dot(N, L), 0.0);
	vec3 Lo = (kD * albedo / PI + specular) * radiance * NDotL;
	
	vec3 color = albedo * ambient + Lo;
	
	// some rim lighting?
	float NDotV = max(dot(N, V), 0.0);
	float rimFactor = (glossiness + 1.0)/(F0 + 1.0);
	vec3 rimLighting = pow(fresnelSchlick(NDotV, vec3(F0)), vec3(rimFactor)) * (roughness * roughness);
	
	color += rimLighting * radiance;
	
	color = color / (color + vec3(1.0));
	
	return color;
}

void main() {
	vec4 finalColor = vec4( computeFinalColor(material_diffuse.rgb, scene_ambient_color.rgb, material_glossiness, material_fresnel0), 1.0 );
	// gamma encode
	gl_FragColor = gammaEncode(finalColor);
}