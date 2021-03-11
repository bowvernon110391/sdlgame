uniform mat4 matMVP;

attribute vec3 position;

const float scale = 0.2;

void main() {
	vec3 scaledPos = position * scale;	
    gl_Position = matMVP * vec4(scaledPos, 1.0);
}