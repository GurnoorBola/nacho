#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform usampler2D tex;
uniform vec3 baseColor1;
uniform vec3 baseColor2;
uniform vec3 highlight;
uniform vec3 offColor;

void main()
{
    uint value = texture(tex, TexCoord).r;
    vec3 color = offColor;
    switch (value) {
        case 1u:
            color = baseColor1;
            break;
        case 2u:
            color = baseColor2;
            break;
        case 3u:
            color = highlight;
            break;
    }
    FragColor = vec4(color, 1.0);
}