#version 130

uniform sampler2D tex;
in vec2 texCoord;

uniform vec3 eyePos;
uniform float height;
uniform float width;

vec2 kernel[4] = vec2[] (vec2(-1, 0), vec2(0, 1), vec2(1, 0), vec2(0, -1));
mat2 rot = mat2(0.707107, -0.707107, 0.707107, 0.707107);

uniform vec3 TL;
uniform vec3 TR;
uniform vec3 BL;
uniform vec3 BR;

out vec4 visibility;

vec3 getEyeDir(vec2 coord)
{
    vec3 eyeDir = BL+coord.x*(BR-BL)+coord.y*(TL-BL);
    eyeDir *= -1;
    return eyeDir;
}

float samplePixel(vec3 pos, vec3 normal, vec2 coord)
{
    vec3 eyeDir = getEyeDir(coord);
    vec4 dstNormal = texture(tex, coord).rgba;
    vec3 dstPosition = normalize(eyeDir.xyz)*dstNormal.w;
    if(dstNormal.w == 1) return 0.;
    vec3 positionVec = dstPosition - pos;
    float inten2 = 2*max(dot(normalize(positionVec.xyz), normal) - 0.4, 0.0);
    float attenuation = 1.0 / (2 + (15 * length(positionVec.xyz)));
    return inten2*attenuation;
}

void main(void)
{
    vec3 eyeDir = getEyeDir(texCoord);
    vec4 normal = texture(tex, texCoord).rgba;
    vec4 position = vec4(normalize(eyeDir.xyz)*normal.w, 1);
    if(normal.w == 1)
    {
        visibility = vec4(0, 0, 0, 1);
        return;
    }
    normal.xyz = normalize(normal.xyz);

    float occlusion = 0;
    float radius = 80/normal.w;
    for(int i = 0; i < 4; ++i)
    {
        vec2 k1 = radius*kernel[i]*vec2(width, height);
        vec2 k2 = rot*k1;
        occlusion  += samplePixel(position.xyz, normal.xyz, texCoord+k1);
        occlusion  += samplePixel(position.xyz, normal.xyz, texCoord+k2);
        occlusion  += samplePixel(position.xyz, normal.xyz, texCoord+k1*0.6);
        occlusion  += samplePixel(position.xyz, normal.xyz, texCoord+k2*0.6);
        occlusion  += samplePixel(position.xyz, normal.xyz, texCoord+k1*0.4);
        occlusion  += samplePixel(position.xyz, normal.xyz, texCoord+k2*0.4);
        occlusion  += samplePixel(position.xyz, normal.xyz, texCoord+k1*0.2);
        occlusion  += samplePixel(position.xyz, normal.xyz, texCoord+k2*0.2);
    }
    occlusion /= 3;
    //occlusion = clamp(normal.w, 0, 1);
    visibility = vec4(occlusion, 0, 0, 1);
}
