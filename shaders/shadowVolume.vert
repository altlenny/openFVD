#version 130

in vec3 aPosition;
uniform vec3 lightDir;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 anchorBase;
out vec4 pos;

void main(void)
{
    pos = anchorBase * vec4(aPosition, 1);
    gl_Position = projectionMatrix * modelMatrix * pos;
}
