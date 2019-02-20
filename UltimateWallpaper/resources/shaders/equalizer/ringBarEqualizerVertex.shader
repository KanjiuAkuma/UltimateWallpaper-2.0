#shader vertex
#version 330 core

layout(location = 0) in vec2 in_position;

out vec2 position;
flat out vec2 origin;
flat out float height;
flat out vec4 base_color;

uniform mat4 u_mvp;
uniform int u_bars;
uniform float u_barWidth;
uniform float u_colorValue;
uniform float u_alpha;
uniform float u_angleValue;
uniform float u_innerRadius;
uniform float u_outerRadius;
uniform float u_baseAmplitude;
uniform samplerBuffer u_spectrum;

float theta = radians(360.f / u_bars);
float dh = u_innerRadius * (1.f - cos(.5f * theta));
float barWidth = 2.f * u_innerRadius * sin(.5 * theta);
float dTheta = radians(360.f * u_angleValue);
float outerRadius = u_outerRadius - u_innerRadius;

vec2 rotate(vec2 v, float rds) {
	float cs = cos(rds);
	float sn = sin(rds);

	return vec2(v.x * cs - v.y * sn, v.x * sn + v.y * cs);
}

vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
	float angle = dTheta + theta * gl_InstanceID;
	float barPosX = (-.5f + in_position.x) * barWidth * u_barWidth;
	float amplitude = texelFetch(u_spectrum, gl_InstanceID).x;
	float amp = u_baseAmplitude + amplitude;
	
	vec2 pos = rotate(vec2(barPosX, u_innerRadius + amp * in_position.y * outerRadius - dh), angle);

	gl_Position = u_mvp * vec4(pos, 0.f, 1.f);
	position = pos;
	height = amp * outerRadius - dh;
	base_color = vec4(hsv2rgb(vec3(u_colorValue, 1.f, clamp(1.f - .7f * amplitude, 0.f, 1.f))), u_alpha);
}