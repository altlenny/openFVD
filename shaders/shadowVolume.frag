#version 130

uniform float uFill;
in vec4 pos;

out vec4 visibility;

void main(void)
{
    if(uFill > 0.5) visibility = vec4(uFill, uFill, uFill, 1);
    else visibility = vec4(pos.y/200, pos.y/200, pos.y/200, 1);
}
