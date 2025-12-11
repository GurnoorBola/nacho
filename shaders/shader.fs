#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform usampler2D tex;

uniform vec3 color0;
uniform vec3 color1;
uniform vec3 color2;
uniform vec3 color3;
uniform vec3 color4;
uniform vec3 color5;
uniform vec3 color6;
uniform vec3 color7;
uniform vec3 color8;
uniform vec3 color9;
uniform vec3 color10;
uniform vec3 color11;
uniform vec3 color12;
uniform vec3 color13;
uniform vec3 color14;
uniform vec3 color15;

void main()
{
    uint value = texture(tex, TexCoord).r;
    vec3 color = color0;
    switch (value) {
        case 1u:
            color = color1;
            break;
        case 2u:
            color = color2;
            break;
        case 3u:
            color = color3;
            break;
        case 4u:
            color = color3;
            break;
        case 5u:
            color = color5;
            break;
        case 6u:
            color = color6;
            break;
        case 7u:
            color = color7;
            break;
        case 8u:
            color = color8;
            break;
        case 9u:
            color = color9;
            break;
        case 10u:
            color = color10;
            break;
        case 11u:
            color = color11;
            break;
        case 12u:
            color = color12;
            break;
        case 13u:
            color = color13;
            break;
        case 14u:
            color = color14;
            break;
        case 15u:
            color = color15;
            break;
    }
    FragColor = vec4(color, 1.0);
}