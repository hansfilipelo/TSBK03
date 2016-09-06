#version 150

#define radius 21

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;
float weights[radius] = float[] (0.057138, 0.056559, 0.054856, 0.052132, 0.048544, 0.044292, 0.039597, 0.034685, 0.02977, 0.025037, 0.020631, 0.016658, 0.013178, 0.010216, 0.007759, 0.005774, 0.004211, 0.003009, 0.002106, 0.001445, 0.000971);

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
