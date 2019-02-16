#shader vertex
#version 330 core

layout(location = 0) in vec2 v_position;
layout(location = 1) in float v_isGlowInner;
layout(location = 2) in float v_isGlowOuter;
layout(location = 3) in float v_isGlowCenter;

out vec4 color;

uniform mat4 u_mvp;

uniform vec2 u_audioAmplitude;
uniform float u_colorFilter;

uniform samplerBuffer u_variables;
uniform samplerBuffer u_colors;
uniform samplerBuffer u_positions;

vec2 rotate(vec2 v, float degrees) {
	float alpha = radians(degrees);

	float cs = cos(alpha);
	float sn = sin(alpha);

	return vec2(v.x * cs - v.y * sn, v.x * sn + v.y * cs);
}

vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
	// radius, glowSize, glowAmount
	vec3 variables = texelFetch(u_variables, gl_InstanceID).xyz;
	vec3 col = texelFetch(u_colors, gl_InstanceID).rgb;
	vec3 pos = texelFetch(u_positions, gl_InstanceID).xyz;

	vec2 position = v_position * variables.x;
	
	float alpha = 1.f;

	if (v_isGlowInner == 1) {
		alpha = variables.z;
	}
	else if (v_isGlowOuter == 1) {
		alpha = 0.f;
		if (v_isGlowCenter == 1) {
			// position += v_position * radius * glowSize;
			position *= variables.y * 2.f;
		}
		else {
			// position += v_position * radius * glowSize * 2.f;
			position *= variables.y * 4.f;
		}
	}

	// gl_Position = u_proj * u_view * vec4(position + vec2(px, py), 0.f, 1.f);
	position *= 1.f + u_audioAmplitude.x * (1.f + pow(1.f - col.b, u_colorFilter));
	position = rotate(position, pos.z);
	gl_Position = u_mvp * vec4(position + pos.xy, 0.f, 1.f);
	
	col.b *= 1.f + u_audioAmplitude.y * (1.f + pow(col.b, u_colorFilter));
	color = vec4(hsv2rgb(col), alpha);
}