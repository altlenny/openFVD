#version 130

in vec3 aPosition;
out vec2 rasterCoord;
out vec2 floorCoord;
out vec4 screenCoord;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform vec3 eyePos;

void main(void)
{
    vec3 eye = eyePos.xyz;
    eye.y = 0;
    gl_Position = projectionMatrix * modelMatrix * vec4(aPosition + eye, 1);
    rasterCoord = 0.1*(aPosition.xz+eye.xz+vec2(5, 5));
    floorCoord = (aPosition.xz+eye.xz+vec2(220, 220))/440.;
    screenCoord = gl_Position;
}
