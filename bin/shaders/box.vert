attribute vec3 position;
attribute vec3 normal;
attribute vec2 uv;

// scene config
uniform mat4 m_model_view_projection;
uniform mat4 m_view;
uniform mat3 m_normal;
// convert light data to view space first
uniform vec3 sun_direction;

varying vec2 vTexcoord;
varying vec3 vNormal;
varying vec3 vSunDirection;

void main() {
	vTexcoord = uv;
	vSunDirection = mat3(m_view) * sun_direction;
	vNormal = m_normal * normal;
	
    gl_Position = m_model_view_projection * vec4(position, 1.0);
}