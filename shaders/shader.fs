#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D tex;
uniform vec3 onColor;
uniform vec3 offColor;

void main()
{
    float intensity = 255*texture(tex, TexCoord).r;
    vec3 color = (intensity * onColor) + ((1.0 - intensity) * offColor);
    FragColor = vec4(color, 1.0);
}