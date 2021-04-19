attribute vec3 position;
attribute vec2 uv;

uniform mat4 m_model_view_projection;

varying vec2 v_uv;

void main() {
	v_uv = uv;
	gl_Position = m_model_view_projection * vec4(position, 1.0);
}