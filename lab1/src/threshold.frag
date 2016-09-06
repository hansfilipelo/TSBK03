#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;

void main(void)
{
    vec4 color = texture(texUnit, outTexCoord);

    out_Color.r = max(color.r - 1.0, 0);
    out_Color.g = max(color.g - 1.0, 0);
    out_Color.b = max(color.b - 1.0, 0);
}
