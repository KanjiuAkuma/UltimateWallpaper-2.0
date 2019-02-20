#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 position;
flat in vec2 origin;
flat in float height;
flat in vec4 base_color;

uniform int u_innerRounding;
uniform int u_outerRounding;
uniform float u_innerRadius;

void main() {
	float l = length(position);
	if (u_innerRounding != 0 && l < u_innerRadius) discard; // color = vec4(1.f, 0.f, 0.f, 1.f);
	if (u_outerRounding != 0 && u_innerRadius + height < l) discard;
	color = base_color;
}