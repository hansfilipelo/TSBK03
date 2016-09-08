#version 150

in vec2 outTexCoord;
uniform sampler2D image;
uniform sampler2D glow;
out vec4 out_Color;

void main(void)
{
    out_Color = 1*texture(image, outTexCoord) + 4*texture(glow, outTexCoord);
}
