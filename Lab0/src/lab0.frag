/*
 * random comment here
 * makes syntax highlight appaer
 * colors like springs sprouts
 */

#version 150

in float shade;

out vec4 out_Color;
uniform vec4 light;
uniform vec3 light_pos;

void main(void)
{
	out_Color=vec4(shade,shade,shade,1.0);
}

