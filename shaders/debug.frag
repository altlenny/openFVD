#version 130

out vec4 oFragColor;
uniform sampler2D tex;
in vec2 texCoord;

void main(void)
{
    oFragColor = vec4(texture(tex, texCoord).rgb, 1);
}
