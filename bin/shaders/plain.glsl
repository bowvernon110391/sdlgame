//------------------------------------VERTEX SHADER---------------------------------------------------
#ifdef VERTEX_SHADER

uniform mat4 m_model_view_projection;

attribute vec3 position;
attribute vec2 uv;

varying vec2 v_texcoord;

void main() {
	v_texcoord = uv;
	gl_Position = m_model_view_projection * vec4(position, 1.0);
}

#endif//VERTEX_SHADER

//------------------------------------FRAGMENT SHADER---------------------------------------------------
#ifdef FRAGMENT_SHADER
#ifdef GL_ES
precision mediump float;
#endif//GL_ES

uniform sampler2D texture0;

#ifdef HAS_AMBIENT
	uniform vec4 scene_ambient_color;
#endif//HAS_AMBIENT

varying vec2 v_texcoord;

void main() {
	vec4 texColor = texture2D(texture0, v_texcoord);
	
	#ifdef HAS_ALPHA_CLIP
	if (texColor.a < 0.5) discard;
	#endif//HAS_ALPHA_CLIP
	
	#ifdef HAS_AMBIENT
	gl_FragColor = texColor * scene_ambient_color;
	#else
	gl_FragColor = texColor;
	#endif
}

#endif//FRAGMENT_SHADER