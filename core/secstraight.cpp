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

#include "secstraight.h"
#include "exportfuncs.h"
#include "mnode.h"
#include "optionsmenu.h"

#include <cmath>

using namespace std;

secstraight::secstraight(track* getParent, mnode* first, float getlength) : section(getParent, straight, first)
{
    this->fHLength = getlength;
    this->bArgument = TIME;
    this->bOrientation = QUATERNION;
    bSpeed = 0;
    fVel = 10;
}

void secstraight::changelength(float newlength)
{
    this->fHLength = newlength;
    this->updateSection();
}

int secstraight::updateSection(int)
{
    //this->rollFunc->setMaxArgument(fHLength);

    int numNodes = 1;
    this->length = 0;
    fHLength = getMaxArgument();

    while(lNodes.size() > 1) {
        if(lNodes.size() > 2 || this->parent->lSections.at(this->parent->lSections.size()-1) == this) {
            delete lNodes.at(1);
        }
        lNodes.removeAt(1);
    }
    lNodes.at(0)->updateNorm();

    float diff = lNodes.at(0)->fRollSpeed; // - rollFunc->funcList.at(0)->startValue;
    rollFunc->funcList.at(0)->translateValues(diff);
    rollFunc->translateValues(rollFunc->funcList.at(0));

    bool lastNode = false;

    float fCurLength = 0.0f;

    while(fCurLength < this->fHLength - std::numeric_limits<float>::epsilon() && !lastNode) {
        lNodes.append(new mnode(*(this->lNodes.at(numNodes-1))));

        float dTime;
        mnode* prevNode = lNodes[numNodes-1];
        mnode* curNode = lNodes[numNodes];

        if(curNode->fVel < 0.1f) {
            qWarning("train goes very slowly");
            break;
        }
        if(curNode->fVel/F_HZ < this->fHLength - fCurLength) {
            dTime = F_HZ;
        } else {
            lastNode = true;
            dTime = (curNode->fVel + std::numeric_limits<float>::epsilon())/(this->fHLength - fCurLength);
        }

        curNode->vPos += curNode->vDir*(curNode->fVel/dTime);

        fCurLength += curNode->fVel/dTime;

        curNode->setRoll(rollFunc->getValue(fCurLength)/dTime); //rollFunc->getValue((i+1)/10.0) - rollFunc->getValue(i/10.0));

        curNode->forceNormal = -curNode->vNorm.y;
        curNode->forceLateral = -curNode->vLat.y;

        curNode->fDistFromLast = glm::distance(curNode->vPosHeart(parent->fHeart), prevNode->vPosHeart(parent->fHeart));
        curNode->fTotalLength += curNode->fDistFromLast;
        curNode->fHeartDistFromLast = glm::distance(curNode->vPos, prevNode->vPos);
        curNode->fTotalHeartLength += curNode->fHeartDistFromLast;

        curNode->fRollSpeed = rollFunc->getValue(fCurLength);

        calcDirFromLast(numNodes);
        curNode->fAngleFromLast = 0.0;
        curNode->fDirFromLast = 0.0;
        curNode->fYawFromLast = 0.0;
        curNode->fPitchFromLast = 0.0;
        if(fabs(lNodes[numNodes]->fRollSpeed) < 0.001) {
            curNode->fTrackAngleFromLast = 0.0;
        }

        if(bSpeed) {
            curNode->fEnergy -= (curNode->fVel*curNode->fVel*curNode->fVel/F_HZ * parent->fResistance);
            curNode->fVel = sqrt(2.f*(curNode->fEnergy-9.80665f*(curNode->vPosHeart(parent->fHeart*0.9f).y+curNode->fTotalLength*parent->fFriction)));
        } else {
            curNode->fVel = this->fVel;
            curNode->fEnergy = 0.5*fVel*fVel + 9.80665f*(curNode->vPosHeart(parent->fHeart*0.9f).y + curNode->fTotalLength*parent->fFriction);
        }

        this->length += curNode->fDistFromLast;
        ++numNodes;
    }

    if(lNodes.size()) length = lNodes.last()->fTotalLength - lNodes.first()->fTotalLength;
    else length = 0;

    //qDebug("Straight section Length:%f", this->length);
    return 0;
}

float secstraight::getMaxArgument()
{
    return rollFunc->getMaxArgument();
}

void secstraight::saveSection(std::fstream& file)
{
    file << "STR";
    writeBytes(&file, (const char*)&bSpeed, sizeof(bool));

    int namelength = sName.length();
    std::string name = sName.toStdString();

    writeBytes(&file, (const char*)&namelength, sizeof(int));
    file << name;

    writeBytes(&file, (const char*)&fVel, sizeof(float));
    writeBytes(&file, (const char*)&fHLength, sizeof(float));
    rollFunc->saveFunction(file);
}

void secstraight::loadSection(std::fstream& file)
{
    bSpeed = readBool(&file);

    int namelength = readInt(&file);
    sName = QString(readString(&file, namelength).c_str());


    fVel = readFloat(&file);
    fHLength = readFloat(&file);
    rollFunc->loadFunction(file);
}

void secstraight::legacyLoadSection(std::fstream& file)
{
    bSpeed = readBool(&file);

    int namelength = readInt(&file);
    sName = QString(readString(&file, namelength).c_str());


    fVel = readFloat(&file);
    fHLength = readFloat(&file);
    rollFunc->legacyLoadFunction(file);
}

void secstraight::saveSection(std::stringstream& file)
{
    file << "STR";
    writeBytes(&file, (const char*)&bSpeed, sizeof(bool));

    int namelength = sName.length();
    std::string name = sName.toStdString();

    writeBytes(&file, (const char*)&namelength, sizeof(int));
    file << name;

    writeBytes(&file, (const char*)&fVel, sizeof(float));
    writeBytes(&file, (const char*)&fHLength, sizeof(float));
    rollFunc->saveFunction(file);
}

void secstraight::loadSection(std::stringstream& file)
{
    bSpeed = readBool(&file);

    int namelength = readInt(&file);
    sName = QString(readString(&file, namelength).c_str());


    fVel = readFloat(&file);
    fHLength = readFloat(&file);
    rollFunc->loadFunction(file);
}

bool secstraight::isInFunction(int index, subfunction* func)
{
    if(func == NULL) return false;
    if(index >= lNodes.size()) return false;
    float dist = lNodes[index]->fTotalHeartLength - lNodes[0]->fTotalHeartLength;
    if(dist >= func->minArgument && dist <= func->maxArgument) {
        return true;
    }
    return false;
}

bool secstraight::isLockable(function* _func)
{
    Q_UNUSED(_func)
    return false;
}
