#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

flat in vec4 base_color;

void main() {
	color = base_color;
}