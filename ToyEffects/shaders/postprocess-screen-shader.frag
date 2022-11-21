#version 450 core

// todo: 目前版本针对中期演示制作。

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform int effect;

const float offset = 1.0 / 300.0;

void main() {

    vec2 offsets[9] = vec2[] (
        vec2(-offset, offset), // top-left
        vec2( 0.0f, offset), // top-center
        vec2( offset, offset), // top-right
        vec2(-offset, 0.0f), // center-left
        vec2( 0.0f, 0.0f), // center-center
        vec2( offset, 0.0f), // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f, -offset), // bottom-center
        vec2( offset, -offset) // bottom-right
    );


    vec3 col = texture(screenTexture, TexCoords).rgb;
    
    if (effect == 0) {

        FragColor = vec4(col, 1.0);

    } else if (effect == 1) {
        
        FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);

    } else if (effect == 2) {

        FragColor = texture(screenTexture, TexCoords);
        float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
        FragColor = vec4(average, average, average, 1.0);

    } else {

        float kernel[9];

        if (effect == 3) {

            kernel = float[](
                -1, -1, -1,
                -1, 9, -1,
                -1, -1, -1
            );

            

        } else if (effect == 4) {

            kernel = float[](
                1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0,
                1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0,
                1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0
            );

        } else if (effect == 5) {

            kernel = float[](
                0, -1, 0,
                -1, 5, -1,
                0, -1, 0
            );

        } else if (effect == 6) {

        }

        vec3 sampleTex[9];
        for(int i = 0; i < 9; i++)
        {
            sampleTex[i] = vec3(texture(screenTexture, TexCoords.st +
            offsets[i]));
        }
        vec3 col = vec3(0.0);

        for(int i = 0; i < 9; i++)
            col += sampleTex[i] * kernel[i];

        FragColor = vec4(col, 1.0);
    }
}
