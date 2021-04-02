uniform mat4 m_model_view_projection;

attribute vec3 position;

const float scale = 0.2;

void main() {
	vec3 scaledPos = position * scale;	
    gl_Position = m_model_view_projection * vec4(scaledPos, 1.0);
}