#version 130

out vec4 oFragColor;
uniform sampler2D rasterTex;
uniform sampler2D floorTex;
uniform sampler2D shadowTex;
in vec2 rasterCoord;
in vec2 floorCoord;
in vec4 screenCoord;
uniform float opacity;
uniform int border;
uniform int grid;

void main(void)
{
    oFragColor = texture(floorTex, floorCoord);
    oFragColor.w = opacity;
    if((floorCoord.x > 1 || floorCoord.y > 1 || floorCoord.x < 0 || floorCoord.y < 0) && border == 1)
    {
        oFragColor.xyz = vec3(0.5, 0.5, 0.5);
    }
    if(grid == 1) oFragColor.xyz *= (texture(rasterTex, rasterCoord).x);
    float visible = texture(shadowTex, 0.5*(screenCoord.xy/screenCoord.w+vec2(1, 1))).x;
    if(visible > 0) visible = 1;
    else visible = 0;
    oFragColor.xyz -= 0.5*(1-visible);
    oFragColor.xyz /= opacity;
    oFragColor.xyz = pow(clamp(oFragColor.xyz, 0, 1), vec3(1.f/2.2f, 1.f/2.2f, 1.f/2.2f));
}
