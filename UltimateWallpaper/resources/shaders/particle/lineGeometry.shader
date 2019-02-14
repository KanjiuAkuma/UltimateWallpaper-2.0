#shader geometry
#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 18) out;

flat in int id[];

out vec4 color;

uniform mat4 u_mvp;

uniform int u_pCount;

uniform float u_maxLineDistance;
uniform float u_lineWidth;

uniform samplerBuffer u_variables;
uniform samplerBuffer u_colors;
uniform samplerBuffer u_positions;

void triangle(vec4 p0, vec4 p1, vec4 p2, vec4 c0, vec4 c1, vec4 c2) {
	color = c0;
	gl_Position = p0;
	EmitVertex();

	color = c1;
	gl_Position = p1;
	EmitVertex();

	color = c2;
	gl_Position = p2;
	EmitVertex();

	EndPrimitive();
}

void main() {
	int p0Id = 0;
	int id = id[0] + 1;
	int c = u_pCount - 1;
	while (c < id) {
		p0Id++;
		id -= c;
		c--;
	}
	int p1Id = p0Id + id;

	// get particle positions
	vec4 p0 = u_mvp * vec4(texelFetch(u_positions, p0Id).xy, 0.f, 1.f);

	vec4 p1 = u_mvp * vec4(texelFetch(u_positions, p1Id).xy, 0.f, 1.f);

	if (distance(p0.xy, p1.xy) < u_maxLineDistance) {
		// get particle colors
		vec4 col0 = vec4(texelFetch(u_colors, p0Id).rgb, 0.75f);
		vec4 col1 = vec4(texelFetch(u_colors, p1Id).rgb, 0.75f);

		// calculate points
		vec2 para = normalize(p1.xy - p0.xy);
		vec2 perp = vec2(-para.y, para.x);
		vec4 corner = vec4(perp * .5f, 0.f, 0.f);

		vec4 c0 = p0 + corner * u_lineWidth;
		vec4 c1 = p0 - corner * u_lineWidth;
		vec4 c2 = p1 - corner * u_lineWidth;
		vec4 c3 = p1 + corner * u_lineWidth;

		// draw line
		triangle(c0, c1, c2, col0, col0, col1);
		triangle(c2, c3, c0, col1, col1, col0);
	}
}