/*
#    FVD++, an advanced coaster design tool for NoLimits
#    Copyright (C) 2012-2015, Stephan "Lenny" Alt <alt.stephan@web.de>
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

#include "mnode.h"
#include "exportfuncs.h"
#include <cmath>
#include "lenassert.h"

using namespace std;

mnode::mnode()
{

}

mnode::mnode(glm::vec3 getPos, glm::vec3 getDir, float getRoll, float getVel, float getNForce, float getLateral)
{
    this->vPos = getPos;
    this->vDir = getDir;
    this->vDir = glm::normalize(vDir);
    this->fRoll = getRoll;
    this->fVel = getVel;
    this->fEnergy = 0;
    this->forceNormal = getNForce;
    this->forceLateral = getLateral;
    this->fDistFromLast = 0.0f;
    this->fHeartDistFromLast = 0.0f;
    this->fAngleFromLast = 0.0f;
    this->fTrackAngleFromLast = 0.0f;
    this->fDirFromLast = 0.0f;
    this->fPitchFromLast = 0.0f;
    this->fYawFromLast = 0.0f;
    this->fTotalLength = 0.0f;
    this->fTotalHeartLength = 0.0f;
    this->fSmoothSpeed = 0.0f;
    this->smoothNormal = 0.0f;
    this->smoothLateral = 0.0f;

    if(this->vDir.y == 1)
    {
        this->vLat = glm::vec3(glm::angleAxis(TO_RAD(getRoll), glm::vec3(0.f, -1.f, 0.f))*glm::vec4(1.f, 0.f, 0.f, 0.f));
    }
    else
    {
        this->vLat = glm::vec3(-this->vDir.z, 0.f, this->vDir.x);
    }

    this->vLat.y = tan(fRoll*F_PI/180)*sqrt(this->vLat.x*this->vLat.x+this->vLat.z*this->vLat.z);
    this->vLat = glm::normalize(vLat);
    this->fRollSpeed = 0.0;
}

void mnode::setRoll(float dRoll)
{
    vLat = glm::normalize(glm::angleAxis(TO_RAD(-dRoll), vDir)*vLat);
    this->updateRoll();
    return;
}

void mnode::updateRoll()
{
    this->updateNorm();
    fRoll = glm::atan(vLat.y, -vNorm.y)*180.f/F_PI;
    return;
}

void mnode::saveNode(fstream& file)
{
    /*writeBytes(&file, (const char*)&vPos, sizeof(glm::vec3));
    writeBytes(&file, (const char*)&vDir, sizeof(glm::vec3));*/
    writeBytes(&file, (const char*)&vLat, sizeof(glm::vec3));
    writeBytes(&file, (const char*)&fVel, sizeof(float));
}

void mnode::legacyLoadNode(fstream& file)
{
    vPos = readVec3(&file);
    vDir = readVec3(&file);
    vLat = readVec3(&file);
    fVel = readFloat(&file);
}

void mnode::changePitch(float dAngle, bool inverted)
{
    glm::vec3 rotateAround;
    lenAssert(fabs(vLat.y) < 1.9f);
    rotateAround = glm::normalize(glm::cross(glm::vec3(0, vNorm.y, 0), vDir));
    if(inverted) {
        rotateAround *= -1.f;
    }
    vDir = glm::normalize(glm::angleAxis(TO_RAD(dAngle), rotateAround) * vDir);
    vLat = glm::normalize(glm::angleAxis(TO_RAD(dAngle), rotateAround) * vLat);
    updateNorm();
}

void mnode::changeYaw(float dAngle)
{
    vDir = glm::normalize(glm::angleAxis(TO_RAD(dAngle), glm::vec3(0.f, 1.f, 0.f))*vDir);
    vLat = glm::normalize(glm::angleAxis(TO_RAD(dAngle), glm::vec3(0.f, 1.f, 0.f))*vLat);
    this->updateNorm();
}

glm::vec3 mnode::vLatHeart(float fHeart)
{
    float estimated;
    float estDistFromLast = 0.7f*fHeartDistFromLast + 0.3f*fDistFromLast;
    if(fAngleFromLast < 0.001f) {
        estimated = fHeartDistFromLast;
    } else {
        estimated = fVel/F_HZ;
    }
    float fRollSpeedPerMeter = estDistFromLast > 0.f ? (fRollSpeed + fSmoothSpeed)/F_HZ/estimated : 0.f;
    return glm::normalize(glm::normalize(vLat) - glm::normalize(vDir)*(float)(fRollSpeedPerMeter*F_PI*fHeart/180.f));
}

glm::vec3 mnode::vDirHeart(float fHeart)
{
    float estimated;
    if(fAngleFromLast < 0.001f) {
        estimated = fHeartDistFromLast;
    } else {
        estimated = fVel/F_HZ;
    }
    float fRollSpeedPerMeter = fHeartDistFromLast > 0.f ? (fRollSpeed + fSmoothSpeed)/F_HZ/estimated : 0.f;
    if(fRollSpeedPerMeter != fRollSpeedPerMeter)
        fRollSpeedPerMeter = 0.f;
    return glm::normalize(vDir + vLat*(float)(fRollSpeedPerMeter*F_PI*fHeart/180.f));
}

void mnode::exportNode(QList<bezier_t*> &bezList, mnode *last, mnode*, mnode* anchor, float fHeart, float fRollThresh)
{
    #define SCALING 3.f

    bezList.append(new bezier_t);

    float realDist = this->fTotalLength - last->fTotalLength;

    float fThreshold = this->fTotalLength - last->fTotalLength; // glm::distance(last->fPosHeart(fHeart), this->fPosHeart(fHeart));
    float temp = glm::length(glm::vec3(anchor->vDir.x, 0.f, anchor->vDir.z));
    glm::mat3 anchorBase = glm::mat3(-anchor->vDir.z/temp, 0.f, -anchor->vDir.x/temp,
                     0.f, 1.f, 0.f,
                     anchor->vDir.x/temp, 0.f, -anchor->vDir.z/temp);

    float radius = this->fVel/(this->fTrackAngleFromLast*F_PI/180.f)/F_HZ;
    float angle = (F_HZ*this->fTrackAngleFromLast*F_PI/180.f)/this->fVel*realDist;


    fThreshold = 0.998f * 2.f/3.f * radius * tan(angle/2);

    if(fThreshold != fThreshold) {
        fThreshold = 0.998f*2.f/3.f*realDist/2.f;
    }

    bezList.last()->P1 = anchorBase*(this->vPosHeart(fHeart) - anchor->vPosHeart(fHeart));

    if(bezList.size() > 1) {
        bezList.last()->Kp1 = bezList[bezList.size()-2]->P1 + anchorBase*(fThreshold * last->vDirHeart(fHeart));
    } else {
        bezList.last()->Kp1 = anchorBase*(last->vPosHeart(fHeart) - anchor->vPosHeart(fHeart) + fThreshold * last->vDirHeart(fHeart));
    }
    bezList.last()->Kp2 = bezList.last()->P1 - anchorBase*(fThreshold * this->vDirHeart(fHeart));


    temp = 0.f;

    if(fabs(this->vDirHeart(fHeart).y) < fRollThresh) {
        temp = glm::atan(this->vLatHeart(fHeart).y, -this->vNorm.y);
    } else {
        glm::vec3 rotateAxis = glm::cross(last->vDirHeart(fHeart), this->vDirHeart(fHeart));
        glm::vec3 rotated = glm::vec3(glm::rotate(glm::angle(last->vDirHeart(fHeart), this->vDirHeart(fHeart)), rotateAxis)*glm::vec4(last->vLatHeart(fHeart), 0.f));
        temp = glm::angle(rotated, this->vLatHeart(fHeart))*F_PI/180.f;
        if(temp!=temp)
        {
            temp = 0.f;
        }
    }

    bezList.last()->roll = temp;

    if(fabs(this->vDirHeart(fHeart).y) < fRollThresh) {
        bezList.last()->relRoll = false;
    } else {
        bezList.last()->relRoll = true;
    }
}

void mnode::calcSmoothForces()
{
    glm::vec3 forceVec;
    float temp = cos(fabs(getPitch())*F_PI/180.f);
    if(fabs(fAngleFromLast) < std::numeric_limits<float>::epsilon()) {
        forceVec = glm::vec3(0.f, 1.f, 0.f);
    } else {
        float normalDAngle = F_PI/180.f*(- fPitchFromLast * cos(fRoll*F_PI/180.) - temp*fYawFromLast*sin(fRoll*F_PI/180.));
        float lateralDAngle = F_PI/180.f*(fPitchFromLast * sin(fRoll*F_PI/180.) - temp*fYawFromLast*cos(fRoll*F_PI/180.));
        forceVec = glm::vec3(0.f, 1.f, 0.f) + lateralDAngle*fVel*F_HZ/F_G * vLat + normalDAngle*fHeartDistFromLast*F_HZ*F_HZ/F_G * vNorm;
    }
    smoothNormal = - glm::dot(forceVec, glm::normalize(vNorm)) - forceNormal;
    smoothLateral = - glm::dot(forceVec, glm::normalize(vLat)) - forceLateral;
}
