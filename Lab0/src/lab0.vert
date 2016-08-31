/*
 * random comment here
 * makes syntax highlight appaer
 * colors like springs sprouts
 */

#version 150

in  vec3  in_Position;
in  vec3  in_Normal;
in  vec2  in_TexCoord;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out float shade;

void main(void)
{
	shade = (mat3(viewMatrix)*in_Normal).z; // Fake shading
	gl_Position=projectionMatrix*viewMatrix*vec4(in_Position, 1.0);
}

