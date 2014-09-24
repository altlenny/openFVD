/*
#    FVD++, an advanced coaster design tool for NoLimits
#    Copyright (C) 2012-2014, Stephan "Lenny" Alt <alt.stephan@web.de>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "subfunction.h"
#include "function.h"
#include "section.h"
#include "track.h"
#include "mnode.h"

#include "exportfuncs.h"

using namespace std;

float interpolate(float t, float x1, float x2)
{
    return t*x2+x1-t*x1;
}

float interpolate(float t, float x1, float x2, float x3)
{
    float t1 = 1-t;
    return t1*t1*x1 + 2*t*t1*x2 + t*t*x3;
}

float interpolate(float t, float x1, float x2, float x3, float x4)
{
    float t1 = 1-t;
    return t1*t1*t1*x1 + 3*t*t1*t1*x2 + 3*t*t*t1*x3 + t*t*t*x4;
}


subfunction::subfunction()
{
}

subfunction::subfunction(float min, float max, float start, float diff, function* getparent)
{
    minArgument = min;
    maxArgument = max;

    centerArg = 0.f;
    tensionArg = 0.f;
    symArg = diff;

    startValue = start;
    parent = getparent;
    lenAssert(getparent != NULL);

    if(parent->type == funcNormal) {
        changeDegree(cubic);
    } else {
        changeDegree(quartic);
    }
    locked = false;
}

void subfunction::update(float min, float max, float diff)
{
    minArgument = min;
    maxArgument = max;

    symArg = diff;

    this->parent->translateValues(this);
}

void subfunction::updateBez()
{
    int i = 0;
    valueList.clear();
    float t=0;
    float nextT = 0, gotT;
    while(i<100)
    {
        nextT+=0.01f;
        valueList.append(interpolate(t, 0, pointList[0].y, pointList[1].y, 1));
        gotT = interpolate(t, 0, pointList[0].x, pointList[1].x, 1);
        t += (nextT-gotT)/(3*interpolate(t, pointList[0].x, pointList[1].x-pointList[0].x, 1.f-pointList[1].x));
        ++i;
    }
    valueList.append(1);
}

void subfunction::changeDegree(enum eDegree newDegree)
{
    degree = newDegree;

    switch (newDegree)
    {
    case linear:
        break;
    case quadratic:
        break;
    case cubic:
        break;
    case quartic:
        arg1 = -10.f;
        break;
    case quintic:
        arg1 = 0.f;
        break;
    case sinusoidal:
        break;
    case plateau:
        arg1 = 1.f;
        break;
    case freeform:
        pointList.clear();
        bez_t b;
        b.x = 0.3;
        b.y = 0.0;
        pointList.append(b);
        b.x = 0.7;
        b.y = 1.0;
        pointList.append(b);
        updateBez();
        break;
    case tozero:
        centerArg = 0;
        tensionArg = 0;
        symArg = -startValue;
        break;
    default:
        lenAssert(0 && "unknown degree");
    }
    return;
}

float subfunction::getValue(float x)
{
    if(locked)
    {
        parent->changeLength(parent->secParent->getMaxArgument()-minArgument, parent->getSubfunctionNumber(this));
        //maxArgument = parent->secParent->getMaxArgument();
    }
    else if(x > maxArgument)
    {
        qWarning("Function got parameter out of bounds: x = %f", x);
        x = maxArgument;
    }
    else if(x < minArgument)
    {
        qWarning("Function got parameter out of bounds: x = %f", x);
        x = minArgument;
    }

    x = (x-minArgument)/(maxArgument-minArgument);

    x = applyCenter(x);
    x = applyTension(x);

    float root;
    float max;
    float a,b,c,d,e;
    mnode *curNode, *prevNode;

    track* inTrack;

    switch (degree)
    {
    case linear:
        return symArg*x+startValue;
    case quadratic:
        if(isSymmetric())
        {
            x = 2.f*x-1.f;
            return symArg*(1.f - x*x) + startValue;
        }
        else if(arg1 < 0.f)
        {
            return symArg*(1.f-(1.f-x)*(1.f-x))+startValue;
        }
        else
        {
            return symArg*x*x+startValue;
        }
    case cubic:
        return symArg*x*x*(3+x*(-2))+startValue;
    case quartic:
        if(!isSymmetric())
        {
            return x*x*(-(6*symArg*arg1)/(1-2*arg1)+x*(symArg*(4*arg1+4)/(1-2*arg1)+x*((-3*symArg/(1-2*arg1)))))+startValue;
        }
        else
        {
            return symArg*x*x*(16+x*(-32+x*16))+startValue;
        }
        break;
    case quintic:
        if(fabs(arg1) < 0.005)
        {
            return symArg*x*x*x*(10+x*(-15+x*6))+startValue;
        }
        else if(arg1 < 0)
        {
            root = -sqrt(9+fabs(arg1/10.f)*(-16+16*fabs(arg1/10.f)));
            max = 0.01728+0.00576*root + fabs(arg1/10.f)*(-0.0288-0.00448*root + fabs(arg1/10.f)*(0.0032-0.00576*root + fabs(arg1/10.f)*(-0.0704+0.02048*root + fabs(arg1/10.f)*(0.1024-0.01024*root + arg1/10.f*0.04096))));
            return symArg/max*x*x*(x-1)*(x-1)*(x+arg1/10.f)+startValue;
        }
        else
        {
            root = sqrt(9+arg1/10.f*(-16+16*arg1/10.f));
            max = 0.01728+0.00576*root + arg1/10.f*(-0.0288-0.00448*root + arg1/10.f*(0.0032-0.00576*root + arg1/10.f*(-0.0704+0.02048*root + arg1/10.f*(0.1024-0.01024*root - arg1/10.f*0.04096))));
            return symArg/max*x*x*(x-1)*(x-1)*(x-arg1/10.f)+startValue;
        }
        break;
    case sinusoidal:
        return 0.5f*symArg*(1-cos(F_PI*x))+startValue;
        break;
    case plateau:
        //return symArg*(1.f-exp(-20.f*pow(sin(F_PI*x), 4))) +startValue;
        return symArg*(1.f-(exp(-arg1*15.f*(pow(1.f-fabs(2.f*x-1.f), 3)))))+startValue;
        break;
    case freeform:
        root = (x*(valueList.size()-2));
        max = floor(root)+0.01;
        root = root-floor(root);
        if((int)max == valueList.size()-1)
        {
            return root*symArg*valueList[(int)max]+startValue;
        }
        else
        {
           return (1-root)*symArg*valueList[(int)max]+root*symArg*valueList[(int)(max+1)] +startValue;
        }
        break;
    case tozero:
        inTrack = parent->secParent->parent;

        curNode = this->parent->secParent->parent->getPoint(inTrack->getNumPoints(parent->secParent)+minArgument*1000.f-0.5f);
        prevNode = this->parent->secParent->parent->getPoint(inTrack->getNumPoints(parent->secParent)+minArgument*1000.f-1.5f);
        if(this->parent->secParent->bOrientation == EULER)
        {
        d = (curNode->fRollSpeed + glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f))*curNode->fYawFromLast
             - prevNode->fRollSpeed - glm::dot(prevNode->vDir, glm::vec3(0.f, -1.f, 0.f))*prevNode->fYawFromLast)*F_HZ;
        e = startValue;
        }
        else
        {
            d = (curNode->fRollSpeed + glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f))*curNode->fYawFromLast
                 - prevNode->fRollSpeed - glm::dot(prevNode->vDir, glm::vec3(0.f, -1.f, 0.f))*prevNode->fYawFromLast)*F_HZ;
            e = -glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f))*curNode->fYawFromLast*F_HZ;
            e += startValue;
        }
        arg1 = -curNode->fRoll/(maxArgument-minArgument);
        a = -2.5f*(d+6.f*(e-2.f*arg1));
        b = 6.f*d + 32.f*e -60.f*arg1;
        c = -d*4.5f - 18.f * e + 30.f*arg1;
        return x*(d+x*(c+x*(b+x*a)))+e;
    default:
        qWarning("unknown degree");
    }
    return -1;
}

float subfunction::getMinValue() // relic, doesn't get used at all at this time
{
    return startValue < endValue() ? startValue : endValue();
}

float subfunction::getMaxValue()
{
    return startValue > endValue() ? startValue : endValue();

}

void subfunction::translateValues(float newStart)
{
    startValue = newStart;
    if(degree == tozero) {
        symArg = -startValue;
    }
}

bool subfunction::isSymmetric()
{
    if(degree == quadratic && fabs(arg1) < 0.5f)
        return true;
    if(degree == quartic && arg1 < 0)
        return true;
    if(degree == quintic && fabs(arg1) > 0.005f)
        return true;
    if(degree == plateau)
        return true;
    return false;
}

void subfunction::saveSubFunc(fstream& file)
{
    writeBytes(&file, (const char*)&degree, sizeof(enum eDegree));
    writeBytes(&file, (const char*)&minArgument, sizeof(float));
    writeBytes(&file, (const char*)&maxArgument, sizeof(float));
    writeBytes(&file, (const char*)&startValue, sizeof(float));
    writeBytes(&file, (const char*)&arg1, sizeof(float));
    writeBytes(&file, (const char*)&symArg, sizeof(float));
    writeBytes(&file, (const char*)&centerArg, sizeof(float));
    writeBytes(&file, (const char*)&tensionArg, sizeof(float));
    writeBytes(&file, (const char*)&locked, sizeof(bool));
}

void subfunction::saveSubFunc(stringstream& file)
{
    writeBytes(&file, (const char*)&degree, sizeof(enum eDegree));
    writeBytes(&file, (const char*)&minArgument, sizeof(float));
    writeBytes(&file, (const char*)&maxArgument, sizeof(float));
    writeBytes(&file, (const char*)&startValue, sizeof(float));
    writeBytes(&file, (const char*)&arg1, sizeof(float));
    writeBytes(&file, (const char*)&symArg, sizeof(float));
    writeBytes(&file, (const char*)&centerArg, sizeof(float));
    writeBytes(&file, (const char*)&tensionArg, sizeof(float));
    writeBytes(&file, (const char*)&locked, sizeof(bool));
}

void subfunction::loadSubFunc(fstream& file)
{
    degree = (enum eDegree)readInt(&file);
    minArgument = readFloat(&file);
    maxArgument = readFloat(&file);
    startValue = readFloat(&file);
    arg1 = readFloat(&file);
    symArg = readFloat(&file);
    centerArg = readFloat(&file);
    tensionArg = readFloat(&file);
    locked = readBool(&file);
}

void subfunction::legacyLoadSubFunc(fstream& file)
{
    degree = (enum eDegree)readInt(&file);
    minArgument = readFloat(&file);
    maxArgument = readFloat(&file);
    startValue = readFloat(&file);
    arg1 = readFloat(&file);
    symArg = readFloat(&file);
    centerArg = readFloat(&file);
    tensionArg = readFloat(&file);
    locked = readBool(&file);
}

void subfunction::loadSubFunc(stringstream& file)
{
    degree = (enum eDegree)readInt(&file);
    minArgument = readFloat(&file);
    maxArgument = readFloat(&file);
    parent->changeLength(maxArgument-minArgument, parent->getSubfunctionNumber(this));
    startValue = readFloat(&file);
    arg1 = readFloat(&file);
    symArg = readFloat(&file);
    centerArg = readFloat(&file);
    tensionArg = readFloat(&file);
    locked = readBool(&file);
}

float subfunction::applyTension(float x)
{
    if(fabs(tensionArg) < 0.0005)
    {
        return x;
    }
    else if(tensionArg > 0.f)
    {
        x = 2.f*tensionArg*(x-0.5f);
        x = sinh(x)/sinh(tensionArg);
        x = 0.5f*(x+1.f);
    }
    else
    {
        x = 2.f*sinh(tensionArg)*(x-0.5f);
        x = asinh(x)/tensionArg;
        x = 0.5f*(x+1.f);
    }
    return x;
}

float subfunction::applyCenter(float x)
{
    if(centerArg > 0.f)
    {
        //x = sinh(x*centerArg)/sinh(centerArg);
        x = pow(x, pow(2, centerArg/2.f));
    }
    else if(centerArg < 0.f)
    {
        //x = sinh((x-1.f)*centerArg)/sinh(centerArg)+1;
        x = 1.f - pow(1.f-x, pow(2, -centerArg/2.f));
    }
    return x;
}

float subfunction::endValue()
{
    if(isSymmetric()) {
        return startValue;
    } else {
        return startValue+symArg;
    }
}
