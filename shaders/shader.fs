#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform usampler2D tex;
uniform vec3 onColor;
uniform vec3 offColor;

void main()
{
    uint value = texture(tex, TexCoord).r;
    vec3 color = offColor;
    switch (value) {
        case 1u:
            color = onColor;
            break;
        case 2u:
            break;
        case 3u:
            break;
    }
    FragColor = vec4(color, 1.0);
}