#version 130

in vec3 aPosition;
out vec2 texCoord;

void main(void)
{
    gl_Position = vec4(aPosition.xy, 0, 1);
    texCoord = aPosition.xy*0.5+vec2(0.5, 0.5);
}
