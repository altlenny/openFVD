#version 130

out vec4 oFragColor;
uniform sampler2D tex;
in vec2 texCoord;
uniform vec4 hmdWarp;
uniform float lensCenter;
uniform float scale;

vec2 warpedCoord(vec2 coord)
{
    int bla = -1;
    if(coord.x < 0.5)
    {
        coord.x *= 2;
    }
    else
    {
        coord.x = (coord.x-0.5) *2;
        bla =1;
    }
    vec2 theta = 2*(coord + vec2(bla*lensCenter, 0))-vec2(1, 1);
    float rSq = theta.x*theta.x + theta.y*theta.y;
    theta *= (hmdWarp.x + rSq*(hmdWarp.y + rSq*(hmdWarp.z + rSq*hmdWarp.w)));
    theta =  vec2(0.5-lensCenter*bla,0.5)+0.5*theta*scale;
    if(theta.x > 1 || theta.x < 0 || theta.y > 1 || theta.y < 0)
    {
        return vec2(-1, -1);
    }
    if(bla==1)
    {
        theta.x = theta.x*0.5 + 0.5;
    }
    else
    {
        theta.x *= 0.5;
    }
    return theta;
}

void main(void)
{
    vec2 warped = warpedCoord(texCoord);
    if(warped.y < 0)
    {
        oFragColor = vec4(0, 0, 0, 1);
        return;
    }
    oFragColor = vec4(texture(tex, warped).rgb, 1);
}
