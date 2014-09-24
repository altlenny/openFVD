#version 130

out vec4 oFragColor;
in vec4 bPosition;
in vec3 bNormal;
in vec3 color;
in vec2 bUv;
in vec4 screenCoord;

uniform vec3 lightDir;

uniform sampler2D metalTex;
uniform sampler2D shadowTex;
uniform samplerCube skyTex;
uniform sampler2D occlusionTex;

void main(void)
{
    vec3 bump = texture(metalTex, bUv).xyz-vec3(0.5, 0.5, 0.5);
    vec3 m_color = color+0.75*bump.xxx;
    vec3 normal = normalize(bNormal+6*cross(bNormal, vec3(1, 0, 0))*bump.y + 6*cross(bNormal, vec3(0, 0, 1))*bump.z);
    vec3 h = normalize(normalize(-bPosition.xyz) - lightDir);
    float specular = max(dot(h, normal), 0);
    vec3 reflection = -normalize(reflect(vec3(bPosition), normal));
    specular = (0.5*pow(specular, 120) + 0.5*pow(specular, 12));
    reflection = texture(skyTex, reflection).xyz;
    float diffusal = max(dot(normal, -lightDir), 0);
    float visible = texture(shadowTex, 0.5*(screenCoord.xy/screenCoord.w+vec2(1, 1))).x;
    float ambient = 0.4-0.3*texture(occlusionTex, 0.5*(screenCoord.xy/screenCoord.w+vec2(1, 1))).x;
    m_color = (1-0.1*diffusal)*m_color + 0.1*diffusal*reflection;
    oFragColor = vec4(ambient*m_color, 1);
    oFragColor.xyz += visible*(0.5*diffusal*m_color + 0.5*specular*reflection);
    oFragColor.xyz = pow(clamp(oFragColor.xyz, 0, 1), vec3(1.f/2.2f, 1.f/2.2f, 1.f/2.2f));
   // oFragColor = vec4(visible, visible, visible, 1);
}
