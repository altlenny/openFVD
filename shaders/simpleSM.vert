#version 130

in vec3 aPosition;
out vec4 pos;
uniform vec3 lightDir;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 anchorBase;

void main(void)
{
    pos = anchorBase * vec4(aPosition, 1);
    vec4 position = pos;
    position.xyz -= position.y*lightDir/lightDir.y;
    gl_Position = projectionMatrix * modelMatrix * position;
}
