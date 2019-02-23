#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

uniform vec4 u_gridColor;

void main() {
	color = u_gridColor;
}