#version 130

in vec4 pos;
out vec4 visibility;

void main(void)
{
    visibility = clamp(vec4(-pos.y, -pos.y, -pos.y, 1), 0, 1);
}
