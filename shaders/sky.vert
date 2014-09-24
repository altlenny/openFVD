#version 130

in vec3 aPosition;
out vec3 texCoord;
uniform vec3 TL;
uniform vec3 TR;
uniform vec3 BL;
uniform vec3 BR;

void main(void)
{
    gl_Position = vec4(aPosition, 1);
    if(aPosition.x < -0.5)
    {//left
        if(aPosition.y < -0.5)
        {//bottom
            texCoord = BL;
        }
        else
        {
            texCoord = TL;
        }
    }
    else
    {//right
        if(aPosition.y < -0.5)
        {//bottom
            texCoord = BR;
        }
        else
        {
            texCoord = TR;
        }
    }
}
