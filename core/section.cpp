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

#include "section.h"
#include "exportfuncs.h"
#include <cmath>

#include "track.h"


#define RELTHRESH 1.0f

using namespace std;

section::section(track* getParent, enum secType _type, mnode* first)
{
    lNodes.append(first);
    parent = getParent;
    normForce = NULL;
    latForce = NULL;
    if(_type != bezier) {
        rollFunc = new function(0, 10, 0, 0, this, funcRoll);
    } else {
        rollFunc = NULL;
    }
    type = _type;
}

section::~section()
{
    while(lNodes.size() > 2)
    {
        delete lNodes.at(1);
        lNodes.removeAt(1);
    }
    if(rollFunc) {
        delete rollFunc;
    }
}

int section::exportSection(fstream *file, mnode* anchor, float mPerNode, float fHeart, glm::vec3& vHeartLat, glm::vec3& Norm, float fRollThresh)
{
    Q_UNUSED(vHeartLat);
    Q_UNUSED(Norm);

    int count = 1;
    int lasti = 0;

    lNodes.at(0)->updateNorm();

    float fThreshold = 0.0;

    int numNodes = (int)(this->length / mPerNode);

    for(int i = 1; i < lNodes.size(); i++)
    {
        lNodes.at(i)->updateNorm();
        fThreshold += glm::distance(lNodes.at(i)->vPosHeart(fHeart), lNodes.at(i-1)->vPosHeart(fHeart));
        //qDebug("Node %d processed: Threshold is now %f", i, fThreshold);

        if(i == this->lNodes.size()-1 || fThreshold > this->length/numNodes)
        {
            //qDebug("Exporting Node %d", i);

            #define SCALING 3.f

            glm::vec3 V(anchor->vDir.x, 0, anchor->vDir.z);
            glm::vec4 KP1, P1, KP2;
            float temp = glm::length(V);
            glm::mat4 anchorBase;
            if(anchor->vDir.z != 0 || anchor->vDir.x != 0) {
                anchorBase = glm::mat4(glm::transpose(glm::mat4(-anchor->vDir.z/temp, 0.f, anchor->vDir.x/temp, 0.f,
                                 0.f, 1.f, 0.f, 0.f,
                                 -anchor->vDir.x/temp, 0.f, -anchor->vDir.z/temp, 0.f,
                                 0.f, 0.f, 0.f, 1.f))
                                 * glm::mat4(1.f, 0.f, 0.f, 0.f,
                                 0.f, 1.f, 0.f, 0.f,
                                 0.f, 0.f, 1.f, 0.f,
                                 -anchor->fPosHeartx(fHeart), -anchor->fPosHearty(fHeart), -anchor->fPosHeartz(fHeart), 1.f));
            } else {
                anchorBase = glm::mat4(glm::transpose(glm::mat4(-anchor->vNorm.z/temp, 0.f, anchor->vNorm.x/temp, 0.f,
                                 0.f, 1.f, 0.f, 0.f,
                                 -anchor->vNorm.x/temp, 0.f, -anchor->vNorm.z/temp, 0.f,
                                 0.f, 0.f, 0.f, 1.f))
                                 * glm::mat4(1.f, 0.f, 0.f, 0.f,
                                 0.f, 1.f, 0.f, 0.f,
                                 0.f, 0.f, 1.f, 0.f,
                                 -anchor->fPosHeartx(fHeart), -anchor->fPosHearty(fHeart), -anchor->fPosHeartz(fHeart), 1.f));
            }

            //float fRollSpeedPerMeter = lNodes.at(lasti)->fDistFromLast > 0.f ? lNodes.at(lasti)->fRollSpeed/F_HZ/lNodes.at(lasti)->fDistFromLast : 0.f;

            KP1 = anchorBase*(glm::vec4(glm::vec3(lNodes.at(lasti)->vPosHeart(fHeart) + fThreshold/SCALING * lNodes.at(lasti)->vDirHeart(fHeart)), 1.f));

            writeBytes(file, (const char*)&KP1.x, 4);
            writeBytes(file, (const char*)&KP1.y, 4);
            writeBytes(file, (const char*)&KP1.z, 4);


            //fRollSpeedPerMeter = lNodes.at(i)->fDistFromLast > 0.f ? lNodes.at(i)->fRollSpeed/F_HZ/lNodes.at(i)->fDistFromLast : 0.f;
            KP2 = anchorBase*(glm::vec4(glm::vec3(lNodes.at(i)->vPosHeart(fHeart) - fThreshold/SCALING * lNodes.at(i)->vDirHeart(fHeart)), 1.f));


            writeBytes(file, (const char*)&KP2.x, 4);
            writeBytes(file, (const char*)&KP2.y, 4);
            writeBytes(file, (const char*)&KP2.z, 4);


            P1 = anchorBase*(glm::vec4(glm::vec3(lNodes.at(i)->vPosHeart(fHeart)), 1.f));

            writeBytes(file, (const char*)&P1.x, 4);
            writeBytes(file, (const char*)&P1.y, 4);
            writeBytes(file, (const char*)&P1.z, 4);

            if(fabs(V.y) < fRollThresh) {
                temp = glm::atan(lNodes.at(i)->vLatHeart(fHeart).y, -lNodes.at(i)->vNorm.y);
            } else {
                glm::vec3 rotateAxis = glm::cross(lNodes.at(lasti)->vDirHeart(fHeart), lNodes.at(i)->vDirHeart(fHeart));
                glm::vec3 rotated = glm::vec3(glm::rotate(glm::angle(lNodes.at(lasti)->vDirHeart(fHeart), lNodes.at(i)->vDirHeart(fHeart)), rotateAxis)*glm::vec4(lNodes.at(lasti)->vLatHeart(fHeart), 0.f));
                temp = glm::angle(rotated, lNodes.at(i)->vLatHeart(fHeart))*F_PI/180.f;
                if(temp!=temp) {
                    temp = 0.f;
                }
            }

            writeBytes(file, (const char*)&temp, 4);

            char cTemp;

            cTemp = 0xFF;
            writeBytes(file, &cTemp, 1); // CONT ROLL
            cTemp = fabs(V.y) < fRollThresh ? 0x00 : 0xff;
            writeBytes(file, &cTemp, 1); // REL ROLL
            cTemp = 0x00;
            writeBytes(file, &cTemp, 1); // equalDistanceCP
            writeNulls(file, 7);

            count++;
            fThreshold -= this->length/numNodes;
            lasti = i;
        }
    }
    return count-1;
}

void section::fillPointList(QList<glm::vec4> &List, QList<glm::vec3> &Normals, mnode* anchor, float mPerNode, float fHeart)
{
    lNodes.at(0)->updateNorm();

    float fThreshold = 0.0;

    int numNodes = (int)(this->length / mPerNode);

    for(int i = 1; i < lNodes.size(); i++) {
        lNodes.at(i)->updateNorm();
        fThreshold += glm::distance(lNodes.at(i)->vPosHeart(fHeart), lNodes.at(i-1)->vPosHeart(fHeart));
        //qDebug("Node %d processed: Threshold is now %f", i, fThreshold);
        glm::vec3 V(anchor->vDir.x, 0, anchor->vDir.z), Norm;
        glm::vec4 P1;
        float temp = glm::length(V);
        glm::mat4 anchorBase;
        if(anchor->vDir.z != 0 || anchor->vDir.x != 0) {
            anchorBase = glm::mat4(glm::transpose(glm::mat4(-anchor->vDir.z/temp, 0.f, anchor->vDir.x/temp, 0.f,
                             0.f, 1.f, 0.f, 0.f,
                             -anchor->vDir.x/temp, 0.f, -anchor->vDir.z/temp, 0.f,
                             0.f, 0.f, 0.f, 1.f))
                             * glm::mat4(1.f, 0.f, 0.f, 0.f,
                             0.f, 1.f, 0.f, 0.f,
                             0.f, 0.f, 1.f, 0.f,
                             -anchor->fPosHeartx(fHeart), -anchor->fPosHearty(fHeart), -anchor->fPosHeartz(fHeart), 1.f));
        } else {
            anchorBase = glm::mat4(glm::transpose(glm::mat4(-anchor->vNorm.z/temp, 0.f, anchor->vNorm.x/temp, 0.f,
                             0.f, 1.f, 0.f, 0.f,
                             -anchor->vNorm.x/temp, 0.f, -anchor->vNorm.z/temp, 0.f,
                             0.f, 0.f, 0.f, 1.f))
                             * glm::mat4(1.f, 0.f, 0.f, 0.f,
                             0.f, 1.f, 0.f, 0.f,
                             0.f, 0.f, 1.f, 0.f,
                             -anchor->fPosHeartx(fHeart), -anchor->fPosHearty(fHeart), -anchor->fPosHeartz(fHeart), 1.f));
        }

        if(i == this->lNodes.size()-1 || fThreshold > this->length/numNodes) {
            P1 = anchorBase*(glm::vec4(glm::vec3(lNodes.at(i)->vPosHeart(fHeart)), 1.f));
            Norm = glm::vec3(anchorBase*(glm::vec4(lNodes[i]->vNorm, 0.f)));
            List.append(glm::vec4(glm::vec3(P1), lNodes.at(i)->fRoll*F_PI/180));
            Normals.append(Norm);
            fThreshold -= this->length/numNodes;
        }
    }
}

void section::iFillPointList(QList<int> &List, float mPerNode)
{
    lNodes.at(0)->updateNorm();

    float fThreshold = 0.0;

    int numNodes = (int)(this->length / mPerNode);
    int nodeCount = 0;
    if(this->type == straight) {
        List.append(parent->getNumPoints(this)+lNodes.size()-2);
        return;
    }

    for(int i = 1; i < lNodes.size(); i++) {
        lNodes.at(i)->updateNorm();
        fThreshold += lNodes[i]->fDistFromLast; //glm::distance(lNodes.at(i)->fPosHeart(fHeart), lNodes.at(i-1)->fPosHeart(fHeart));
        //qDebug("Node %d processed: Threshold is now %f", i, fThreshold);
        if(i == this->lNodes.size()-1 || fThreshold > this->length/numNodes) {
            List.append(parent->getNumPoints(this)+i-1);
            fThreshold -= this->length/numNodes;
            ++nodeCount;
        }
    }
    if(List.size() > 1 && (lNodes[List.last()-parent->getNumPoints(this)+1]->fTotalLength - lNodes[List[List.size()-2]-parent->getNumPoints(this)+1]->fTotalLength < mPerNode/2.f)) {
        List.removeAt(List.size()-2);
    }
}

void section::fFillPointList(QList<int> &List, float mPerNode)
{
    lNodes.at(0)->updateNorm();

    float fThreshold = 0.0;

    int numNodes = (int)(this->length / mPerNode);
    if(numNodes < 2) {
        numNodes = 2;
    }
    int nodeCount = 0;
    if(this->type == straight) {
        if(List.size()) {
            List.last() *= -1;
        }
        List.append((parent->getNumPoints(this)+lNodes.size()-2)*-1);
        return;
    }

    for(int i = 1; i < lNodes.size(); i++) {
        lNodes.at(i)->updateNorm();
        fThreshold += lNodes[i]->fDistFromLast;
        if(i == this->lNodes.size()-1 || fThreshold > this->length/numNodes) {
            List.append(parent->getNumPoints(this)+i-1);
            fThreshold -= this->length/numNodes;
            ++nodeCount;
        }
    }
    if(List.size() > 1 && (lNodes[List.last()-parent->getNumPoints(this)+1]->fTotalLength - lNodes[List[List.size()-2]-parent->getNumPoints(this)+1]->fTotalLength < mPerNode/2.f)) {
        List.removeAt(List.size()-2);
    }
}

void section::calcDirFromLast(int i)
{
    lenAssert(i>=0 || i < lNodes.size());
    if(i == 0 || i >= lNodes.size()) {
        return;
    }
    glm::vec3 diff = lNodes[i]->vDir -lNodes[i-1]->vDir;
    if(diff.length() <= std::numeric_limits<float>::epsilon()) {
        lNodes[i]->fDirFromLast = 0.f;
        lNodes[i]->fPitchFromLast = 0.f;
        lNodes[i]->fYawFromLast = 0.f;
    } else {
        float y = -glm::dot(diff, lNodes[i-1]->vNorm);
        float x = -glm::dot(diff, lNodes[i-1]->vLat);
        float angle = glm::atan(x, y)*180.f/F_PI;
        lNodes[i]->fDirFromLast = angle;
        lNodes[i]->fPitchFromLast = lNodes[i]->getPitch()-lNodes[i-1]->getPitch();
        lNodes[i]->fYawFromLast = lNodes[i]->getDirection()-lNodes[i-1]->getDirection();
        lNodes[i]->fDirFromLast = glm::atan(lNodes[i]->fYawFromLast, lNodes[i]->fPitchFromLast)*180.f/F_PI - lNodes[i]->fRoll;
    }

    glm::vec3 curDirHeart = lNodes[i]->vDirHeart(parent->fHeart);
    glm::vec3 prevDirHeart = lNodes[i-1]->vDirHeart(parent->fHeart);
    float fTrackPitchFromLast = 180.f/F_PI*(asin(curDirHeart.y) - asin(prevDirHeart.y));
    float fTrackYawFromLast = 180.f/F_PI*(glm::atan(-curDirHeart.x, -curDirHeart.z) - glm::atan(-prevDirHeart.x, -prevDirHeart.z));
    float temp = cos(fabs(asin(curDirHeart.y)));
    lNodes[i]->fTrackAngleFromLast = sqrt(temp*temp*fTrackYawFromLast*fTrackYawFromLast + fTrackPitchFromLast * fTrackPitchFromLast);
    if(lNodes[i]->fYawFromLast > 270.f) {
        lNodes[i]->fYawFromLast -= 360.f;
    } else if(lNodes[i]->fYawFromLast < -270.f) {
        lNodes[i]->fYawFromLast += 360.f;
    }
    return;
}

bool section::setLocked(eFunctype func, int _id, bool _locked)
{
    switch(func) {
    case funcRoll:
        if(!_locked) {
            return rollFunc->unlock(_id);
        } else if(normForce == NULL && latForce == NULL) {
            return false;
        } else if(normForce->lockedFunc() > -1 && latForce->lockedFunc() > -1) {
            return false;
        } else {
            return rollFunc->lock(_id);
        }
        break;
    case funcNormal:
        if(!_locked) {
            return normForce->unlock(_id);
        } else if(rollFunc->lockedFunc() > -1 && latForce->lockedFunc() > -1) {
            return false;
        } else {
            return normForce->lock(_id);
        }
        break;
    case funcLateral:
        if(!_locked) {
            return latForce->unlock(_id);
        } else if(rollFunc->lockedFunc() > -1 && normForce->lockedFunc() > -1) {
            return false;
        } else {
            return latForce->lock(_id);
        }
        break;
    case funcPitch:
        if(!_locked) {
            return normForce->unlock(_id);
        } else if(rollFunc->lockedFunc() > -1 && latForce->lockedFunc() > -1) {
            return false;
        } else {
            return normForce->lock(_id);
        }
        break;
    case funcYaw:
        if(!_locked) {
            return latForce->unlock(_id);
        } else if(rollFunc->lockedFunc() > -1 && normForce->lockedFunc() > -1) {
            return false;
        } else {
            return latForce->lock(_id);
        }
        break;
    default:
        lenAssert(0 && "setActive()");
        break;
    }
    return false;
}

float section::getSpeed()
{
    if(bSpeed) {
        return lNodes.last()->fVel;
    } else {
        return fVel;
    }
}
