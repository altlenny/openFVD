#version 130

in vec4 bPosition;
in vec3 bNormal;
in vec4 screenPos;

out vec4 normal;

void main(void)
{
   normal = vec4(bNormal, length(bPosition));//screenPos.z*0.1);
   // oFragColor = vec4(visible, visible, visible, 1);
}
