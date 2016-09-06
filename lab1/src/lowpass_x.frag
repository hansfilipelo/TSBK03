#version 150

#define radius 5

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;
float weights[radius] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main(void)
{
    vec2 pixel_size = 1.0/textureSize(texUnit, 0);
    vec4 temp_out = vec4(0.0, 0.0, 0.0, 0.0);
    

    temp_out += weights[0] * texture(texUnit, outTexCoord);
    for ( int i = 1; i < radius; i++){
        temp_out += weights[i] * texture(texUnit, outTexCoord-vec2(pixel_size.x*i, 0));
        temp_out += weights[i] * texture(texUnit, outTexCoord+vec2(pixel_size.x*i, 0));
    }

    out_Color = temp_out;
}
