#version 130

out vec4 oFragColor;
uniform samplerCube skyTex;
in vec3 texCoord;

void main(void)
{
    oFragColor = vec4(texture(skyTex, normalize(texCoord)).rgb, 1);
}
