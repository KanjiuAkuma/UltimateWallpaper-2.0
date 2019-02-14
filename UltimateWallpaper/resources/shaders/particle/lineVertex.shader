#shader vertex
#version 330 core

flat out int id;

void main() {
	gl_Position = vec4(0.f, 0.f, 0.f, 1.f);
	id = gl_InstanceID;
}