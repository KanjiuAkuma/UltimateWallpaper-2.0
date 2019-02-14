#shader vertex
#version 330 core

layout(location = 0) in vec2 v_position;
layout(location = 1) in vec2 v_texCoords;

out vec2 texCoords;

uniform mat4 u_mvp;

void main() {
	gl_Position = u_mvp * vec4(v_position, 0.f, 1.f);
	texCoords = v_texCoords;
};
