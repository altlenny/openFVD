#version 130

in vec3 aPosition;
out vec4 bPosition;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 anchorBase;
uniform vec3 eyePos;

uniform vec3 defaultColor;
uniform vec3 sectionColor;
uniform vec3 transitionColor;

in vec2 aUv;
out vec2 bUv;

out vec4 screenCoord;

in vec3 aNormal;
out vec3 bNormal;

in float aselected;
in float aVel;
in float aRoll;
in float aNForce;
in float aLForce;
in float aFlex;
out vec3 color;

uniform int colorMode;


vec3 getColor()
{
    switch(colorMode) {
        case 0: // nothing
            if(aselected < 0.5) return defaultColor;
            else if(aselected < 1.5) return sectionColor;
            else return transitionColor;
        case 1: // velocity
            if(aVel > 60.)
            return vec3(1., 0., 1.);
            else if(aVel >= 40.)
            return vec3(1., 0., (aVel-40.)/20);
            else if(aVel >= 30.)
            return vec3(1., (40.-aVel)/10., 0.);
            else if(aVel >= 20.)
            return vec3((aVel-20.)/10., 1., 0);
            else if(aVel >= 10.)
            return vec3(0., 1., (20-aVel)/10.);
            else if(aVel >= 1)
            return vec3(0., (aVel-1)/9, 1.);
            else
            return vec3(0., 0., 0.);
        case 2: // rollspeed
            if(aRoll > 240.)
            return vec3(0., 0., 0.);
            else if(aRoll >= 160)
            return vec3((240-aRoll)/80, 0., (240-aRoll)/80);
            else if(aRoll >= 80)
            return vec3(1., 0., (aRoll-80)/80);
            else if(aRoll >= 40)
            return vec3(1., (80-aRoll)/40, 0);
            else if(aRoll >= 20)
            return vec3((aRoll-20)/20, 1., 0.);
            else if(aRoll >= 10)
            return vec3(0., 1., (20.-aRoll)/10);
            else
            return vec3(0., aRoll/10, 1.);
        case 3: // normal force
            if(aNForce > 6.5)
            return vec3(0., 0., 0.);
            else if(aNForce > 5.)
            return vec3((6.5-aNForce)/1.5, 0., (6.5-aNForce)/1.5);
            else if(aNForce >= 3.5)
            return vec3(1., 0., (aNForce-3.5)/1.5);
            else if(aNForce >= 2)
            return vec3(1., (3.5-aNForce)/1.5, 0.);
            else if(aNForce >= 1.)
            return vec3(aNForce-1, 1., 0.);
            else if(aNForce >= 0.)
            return vec3(0., 1., 1-aNForce);
            else if(aNForce >= -1.)
            return vec3(0., aNForce+1., 1.);
            else if(aNForce >= -2.5)
            return vec3(0., 0., (aNForce+2.5)/(1.5));
            else
            return vec3(0., 0., 0.);
        case 4: // lateral force
            if(aLForce > 2.)
            return vec3(0., 0., 0.);
            else if(aLForce >= 1.5)
            return vec3((2-aLForce)/0.5, 0., (2-aLForce)/0.5);
            else if(aLForce >= 1.)
            return vec3(1., 0., (aLForce-1.0)/0.5);
            else if(aLForce >= 0.5)
            return vec3(1., (1.0-aLForce)/0.5, 0);
            else if(aLForce >= 0.25)
            return vec3((aLForce-0.25)/0.25, 1., 0.);
            else if(aLForce >= 0.1)
            return vec3(0., 1., (0.25-aLForce)/0.15);
            else
            return vec3(0., aLForce*10, 1.);
        case 5: // flexion
            if(aFlex > 30.)
            return vec3(0., 0., 0.);
            else if(aFlex >= 6)
            return vec3((30-aFlex)/24, 0., (30-aFlex)/24);
            else if(aFlex >= 4.5)
            return vec3(1., 0., (aFlex-4.5)/1.5);
            else if(aFlex >= 3.5)
            return vec3(1., (4.5-aFlex)/1, 0);
            else if(aFlex >= 2.5)
            return vec3((aFlex-2.5)/1, 1., 0.);
            else if(aFlex >= 1.0)
            return vec3(0., 1., (2.5-aFlex)/1.5);
            else
            return vec3(0., aFlex, 1.);
    }
}

void main(void)
{
    color = getColor();
    bPosition = anchorBase * vec4(aPosition, 1);
    gl_Position = projectionMatrix * modelMatrix * bPosition;
    bPosition -= vec4(eyePos, 0);
    bNormal = vec3(anchorBase * vec4(aNormal, 0));
    if(length(bNormal) < 0.5) bNormal = vec3(0, 1, 0);
    bUv = aUv;
    screenCoord = gl_Position;
}
