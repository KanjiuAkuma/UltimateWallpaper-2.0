#shader vertex
#version 330 core

layout(location = 0) in vec2 position;

flat out vec4 base_color;

uniform mat4 u_mvp;
uniform int u_bars;
uniform float u_barWidth;
uniform float u_colorValue;
uniform float u_alpha;
uniform float u_baseAmplitude;
uniform samplerBuffer u_spectrum;

vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
	float barWidth = u_barWidth / u_bars;
	float barPosX = -.5f + (float(gl_InstanceID) + .5f * (1.f - u_barWidth)) / u_bars;
	float amplitude = texelFetch(u_spectrum, gl_InstanceID).x;
	float amp = u_baseAmplitude + amplitude;

	gl_Position = u_mvp * vec4(barPosX + barWidth * position.x, amp * position.y, 0.f, 1.f);
	base_color = vec4(hsv2rgb(vec3(u_colorValue, 1.f, clamp(1.f - .7f * amplitude, 0.f, 1.f))), u_alpha);
}