#version 130

in vec3 aPosition;
out vec4 bPosition;
out vec4 screenPos;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 anchorBase;
uniform vec3 eyePos;

in vec3 aNormal;
out vec3 bNormal;

void main(void)
{
    bPosition = anchorBase * vec4(aPosition, 1);
    gl_Position = projectionMatrix * modelMatrix * bPosition;
    bPosition -= vec4(eyePos, 0);
    screenPos = gl_Position;
    bNormal = vec3(anchorBase * vec4(aNormal, 0));
}
