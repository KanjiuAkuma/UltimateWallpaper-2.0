#shader vertex
#version 330 core

layout(location = 0) in vec2 v_position;
layout(location = 1) in vec2 v_texCoords;

out vec2 texCoords;
flat out float transition;

uniform mat4 u_mvp;
uniform int u_cellsX;
uniform int u_cellsY;
uniform float u_transition;
uniform samplerBuffer u_diffuse;

void main() {
	float idy = float(gl_InstanceID / u_cellsX);
	float idx = float(gl_InstanceID - idy * u_cellsX);
	float tx = idx + v_texCoords.x;
	float ty = idy + v_texCoords.y;
	idx += v_position.x;
	idy += v_position.y;

	float posX = -1.f + 2.f * (idx / float(u_cellsX));
	float posY = -1.f + 2.f * (idy / float(u_cellsY));
	float texX = tx / float(u_cellsX);
	float texY = ty / float(u_cellsY);

	transition = clamp(u_transition - texelFetch(u_diffuse, gl_InstanceID).x, 0.f, 1.f);

	gl_Position = u_mvp * vec4(posX, posY, 0.f, 1.f);
	texCoords = vec2(texX, texY);
};
