
attribute vec3 position;
attribute vec3 normal;
attribute vec2 uv;

uniform mat4 m_model_view_projection;
uniform mat4 m_model_view;
uniform mat3 m_normal;

varying vec2 vUv;
varying vec3 vNormal;
varying vec3 vEye;

void main() {
	vUv = uv;
	vNormal = m_normal * normal;
	vEye = vec3(m_model_view * vec4(position, 1.0));
	gl_Position = m_model_view_projection * vec4(position, 1.0);
}