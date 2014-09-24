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

#include "secgeometric.h"
#include "exportfuncs.h"

secgeometric::~secgeometric()
{
    delete normForce;
    delete latForce;
}

secgeometric::secgeometric(track* getParent, mnode* first, float gettime): section(getParent, geometric, first)
{
    this->iTime = (int)(gettime+0.5);
    this->length = 0.0;
    rollFunc->changeLength(1.f, 0);
    float deltaPitch = lNodes[0]->getPitchChange();
    float deltaYaw = lNodes[0]->getYawChange();
    normForce = new function(0, 1, deltaPitch, deltaPitch, this, funcPitch);
    latForce = new function(0, 1, deltaYaw, deltaYaw, this, funcYaw);

    this->bOrientation = EULER;
    this->bArgument = TIME;
    bSpeed = 1;
    fVel = 10;
}

int secgeometric::updateSection(int node)
{
    if(rollFunc->lockedFunc() != -1) {
        if(fabs(rollFunc->funcList.last()->symArg) > 0.00001f && rollFunc->funcList.last()->minArgument*F_HZ < node) node = F_HZ*rollFunc->funcList.last()->minArgument-1.5f;
    }
    if(normForce->lockedFunc() != -1) {
        if(fabs(normForce->funcList.last()->symArg) > 0.00001f && normForce->funcList.last()->minArgument*F_HZ < node) node = F_HZ*normForce->funcList.last()->minArgument-1.5f;
    }
    if(latForce->lockedFunc() != -1) {
        if(fabs(latForce->funcList.last()->symArg) > 0.00001f && latForce->funcList.last()->minArgument*F_HZ < node) node = F_HZ*latForce->funcList.last()->minArgument-1.5f;
    }

    if(bArgument == DISTANCE) {
        return updateDistanceSection(node);
    }

    node = node > lNodes.size()-2 ? lNodes.size()-2 : node;
    node = node < 0 ? 0 : node;

    int numNodes = (int)(getMaxArgument()*F_HZ+0.5);
    iTime = numNodes;

    if(node >= lNodes.size()-1 && node > 0) {
        node = lNodes.size()-2;
    }

    if(lNodes.size() > 1 && this->parent->lSections.at(this->parent->lSections.size()-1) != this) {
        lNodes.removeLast(); // disjoint this section from the next one
    }

    if(node == 0) {
        lNodes.at(0)->updateNorm();

        float diff = lNodes[0]->getPitchChange(); // - normForce->funcList.at(0)->startValue;
        lenAssert(diff==diff);
        if(diff != diff) {
            lNodes.append(new mnode(*(this->lNodes.at(0))));
            return node;
        }
        normForce->funcList.at(0)->translateValues(diff);
        normForce->translateValues(normForce->funcList.at(0));

        diff = lNodes[0]->getYawChange(); // - latForce->funcList.at(0)->startValue;
        lenAssert(diff==diff);
        if(diff != diff) {
            lNodes.append(new mnode(*(this->lNodes.at(0))));
            return node;
        }
        latForce->funcList.at(0)->translateValues(diff);
        latForce->translateValues(latForce->funcList.at(0));

        diff = lNodes[0]->fRollSpeed; // - rollFunc->funcList.at(0)->startValue;
        if(bOrientation == 1) {
            diff += glm::dot(lNodes[0]->vDir, glm::vec3(0.f, 1.f, 0.f))*lNodes[0]->getYawChange();
        }
        rollFunc->funcList.at(0)->translateValues(diff);
        rollFunc->translateValues(rollFunc->funcList.at(0));
    }

    float artificialRoll = lNodes.at(0)->fRoll;
    for(int i = 0; i < node; ++i) {
        if(bOrientation == 0) {
            artificialRoll -= glm::dot(lNodes[i+1]->vDir, glm::vec3(0.f, -1.f, 0.f))*latForce->getValue((i+1)/F_HZ)/F_HZ;
        }
        artificialRoll += rollFunc->getValue((i+1)/F_HZ)/F_HZ;
        while(artificialRoll > 180.f) {
            artificialRoll -= 360.f;
        }
        while(artificialRoll < -180.f) {
            artificialRoll += 360.f;
        }
    }

    int i;
    for(i = node; i < numNodes; i++) {
        if(i >= lNodes.size()-1) {
            lNodes.append(new mnode(*(this->lNodes.at(i))));
        }

        mnode* prevNode = lNodes[i];
        mnode* curNode = lNodes[i+1];

        curNode->vPos = prevNode->vPos;
        curNode->vDir = prevNode->vDir;
        curNode->vLat = prevNode->vLat;
        curNode->vNorm = prevNode->vNorm;
        curNode->fVel = prevNode->fVel;
        curNode->fEnergy = prevNode->fEnergy;

        float pitchChange = normForce->getValue((i+1)/F_HZ)/F_HZ;
        float yawChange = latForce->getValue((i+1)/F_HZ)/F_HZ;
        int sign = 1;
        if(fabs(artificialRoll) >= 90.f) {
            sign = -1;
        }

        curNode->changePitch(pitchChange, sign == -1);
        curNode->changeYaw(yawChange);

        float pureYawChange = (1.f-fabs(glm::dot(curNode->vDir, glm::vec3(0.f, 1.f, 0.f))))*yawChange;
        float pureRollChange = glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f))*yawChange*F_HZ;
        float deltaAngle = sqrt(pitchChange*pitchChange + pureYawChange*pureYawChange);

        curNode->setRoll(-pureRollChange/F_HZ);
        artificialRoll -= pureRollChange/F_HZ;

        curNode->vPos += curNode->vDir*(curNode->fVel/(2.f*F_HZ))+prevNode->vDir*(prevNode->fVel/(2.f*F_HZ)) + (prevNode->vPosHeart(parent->fHeart) - curNode->vPosHeart(parent->fHeart));

        curNode->updateNorm();

        curNode->setRoll(rollFunc->getValue((i+1)/F_HZ)/F_HZ); //rollFunc->getValue((float)(i+1)/numNodes*fAngle)); //360./numNodes*(i+1));

        if(bOrientation == EULER  || rollFunc->getSubfunction((i+1)/F_HZ)->degree == tozero) {
            curNode->setRoll(+pureRollChange/F_HZ);
            artificialRoll += pureRollChange/F_HZ;
        }

        artificialRoll += rollFunc->getValue((i+1)/F_HZ)/F_HZ;
        while(artificialRoll > 180.f) {
            artificialRoll -= 360.f;
        }
        while(artificialRoll < -180.f) {
            artificialRoll += 360.f;
        }
        curNode->updateNorm();

        curNode->fDistFromLast = glm::distance(curNode->vPosHeart(parent->fHeart), prevNode->vPosHeart(parent->fHeart));
        curNode->fTotalLength = prevNode->fTotalLength + curNode->fDistFromLast;
        curNode->fHeartDistFromLast = glm::distance(curNode->vPos, prevNode->vPos);
        curNode->fTotalHeartLength = prevNode->fTotalHeartLength + curNode->fHeartDistFromLast;
        curNode->fRollSpeed = rollFunc->getValue((i+1)/F_HZ);

        if(bOrientation == EULER  || rollFunc->getSubfunction((i+1)/F_HZ)->degree == tozero) {
            curNode->fRollSpeed += pureRollChange;
        }


        if(bSpeed) {
            curNode->fEnergy -= (curNode->fVel*curNode->fVel*curNode->fVel/F_HZ * parent->fResistance);
            curNode->fVel = sqrt(2.f*(curNode->fEnergy-9.80665*(curNode->vPosHeart(parent->fHeart*0.9f).y+curNode->fTotalLength*parent->fFriction)));
        } else {
            curNode->fVel = this->fVel;
            curNode->fEnergy = 0.5*fVel*fVel + 9.80665f*(curNode->vPosHeart(parent->fHeart*0.9f).y + curNode->fTotalLength*parent->fFriction);
        }


        calcDirFromLast(i+1);
        float temp = cos(fabs(curNode->getPitch())*F_PI/180.f);
        float forceAngle = sqrt(temp*temp*curNode->fYawFromLast*curNode->fYawFromLast + curNode->fPitchFromLast*curNode->fPitchFromLast);//deltaAngle;
        curNode->fAngleFromLast = forceAngle;

        glm::vec3 forceVec;
        if(fabs(deltaAngle) < std::numeric_limits<float>::epsilon()) {
            forceVec = glm::vec3(0.f, 1.f, 0.f);
        } else {
            float normalDAngle = F_PI/180.f*(- curNode->fPitchFromLast * cos(curNode->fRoll*F_PI/180.) - temp*curNode->fYawFromLast*sin(curNode->fRoll*F_PI/180.));
            float lateralDAngle = F_PI/180.f*(curNode->fPitchFromLast * sin(curNode->fRoll*F_PI/180.) - temp*curNode->fYawFromLast*cos(curNode->fRoll*F_PI/180.));

            forceVec = glm::vec3(0.f, 1.f, 0.f) + lateralDAngle*curNode->fVel*F_HZ/9.80665f * curNode->vLat + normalDAngle*curNode->fHeartDistFromLast*F_HZ*F_HZ/9.80665f * curNode->vNorm;
        }
        curNode->forceNormal = - glm::dot(forceVec, glm::normalize(curNode->vNorm));
        curNode->forceLateral = - glm::dot(forceVec, glm::normalize(curNode->vLat));
    }
    while(lNodes.size() > 1+i) {
        delete lNodes.at(1+i);
        lNodes.removeAt(1+i);
    }
    if(lNodes.size()) length = lNodes.last()->fTotalLength - lNodes.first()->fTotalLength;
    else length = 0;
    return node;
}

int secgeometric::updateDistanceSection(int node)
{
    node = node < 0 ? 0 : node;

    int i = 0;
    this->length = 0.f;
    float hDist = 0.f;
    float artificialRoll = lNodes.at(0)->fRoll;
    while(length < (float)node/F_HZ && i+1 < lNodes.size()) {
        hDist += lNodes[++i]->fHeartDistFromLast;
        length += lNodes[i]->fDistFromLast;

        if(bOrientation == 0) {
            artificialRoll -= glm::dot(lNodes[i]->vDir, glm::vec3(0.f, -1.f, 0.f))*latForce->getValue(length + lNodes[i]->fVel/F_HZ)*lNodes[i]->fVel/F_HZ;
        }

        artificialRoll += rollFunc->getValue(length + lNodes[i]->fVel/F_HZ)*(lNodes[i]->fVel/F_HZ);
        while(artificialRoll > 180.f) {
            artificialRoll -= 360.f;
        }
        while(artificialRoll < -180.f) {
            artificialRoll += 360.f;
        }
    }

    if(i >= lNodes.size()-1  && i > 0) {
        i = lNodes.size()-2;
    }

    if(lNodes.size() > 1 && this->parent->lSections.at(this->parent->lSections.size()-1) != this) {
        lNodes.removeLast(); // disjoint this section from the next one
    }

    if(i == 0) {
        lNodes.at(0)->updateNorm();

        float diff = lNodes[0]->getPitchChange()/lNodes[0]->fVel; // - normForce->funcList.at(0)->startValue;
        lenAssert(diff==diff);
        if(diff != diff) {
            lNodes.append(new mnode(*(this->lNodes.at(0))));
            return node;
        }
        normForce->funcList.at(0)->translateValues(diff);
        normForce->translateValues(normForce->funcList.at(0));

        diff = lNodes[0]->getYawChange()/lNodes[0]->fVel; // - latForce->funcList.at(0)->startValue;
        lenAssert(diff==diff);
        if(diff != diff) {
            lNodes.append(new mnode(*(this->lNodes.at(0))));
            return node;
        }
        latForce->funcList.at(0)->translateValues(diff);
        latForce->translateValues(latForce->funcList.at(0));

        diff = lNodes[0]->fRollSpeed/lNodes[0]->fVel; // - rollFunc->funcList.at(0)->startValue;
        if(bOrientation == 1) {
            diff += glm::dot(lNodes[0]->vDir, glm::vec3(0.f, 1.f, 0.f))*lNodes[0]->getYawChange()/lNodes[0]->fVel;
        }
        rollFunc->funcList.at(0)->translateValues(diff);
        rollFunc->translateValues(rollFunc->funcList.at(0));
    }

    int returnval = i;
    float end = this->getMaxArgument();

    while(length < end) {
        if(i >= lNodes.size()-1) {
            lNodes.append(new mnode(*(this->lNodes.at(i))));
        }

        mnode* prevNode = lNodes[i];
        mnode* curNode = lNodes[i+1];

        curNode->vPos = prevNode->vPos;
        curNode->vDir = prevNode->vDir;
        curNode->vLat = prevNode->vLat;
        curNode->vNorm = prevNode->vNorm;
        curNode->fVel = prevNode->fVel;
        curNode->fEnergy = prevNode->fEnergy;

        float pitchChange = normForce->getValue(length + curNode->fVel/F_HZ)*(curNode->fVel/F_HZ);
        float yawChange = latForce->getValue(length + curNode->fVel/F_HZ)*(curNode->fVel/F_HZ);
        int sign = 1;
        if(fabs(artificialRoll) >= 90.f) {
            sign = -1;
        }


        curNode->changePitch(pitchChange, sign == -1);
        curNode->changeYaw(yawChange);

        float pureYawChange = (1.f-fabs(glm::dot(curNode->vDir, glm::vec3(0.f, 1.f, 0.f))))*yawChange;
        float pureRollChange = glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f))*yawChange*F_HZ;
        float deltaAngle = sqrt(pitchChange*pitchChange + pureYawChange*pureYawChange);

        curNode->setRoll(-pureRollChange/F_HZ);
        artificialRoll -= pureRollChange/F_HZ;

        curNode->vPos += curNode->vDir*(curNode->fVel/(2.f*F_HZ))+prevNode->vDir*(prevNode->fVel/(2.f*F_HZ)) + (prevNode->vPosHeart(parent->fHeart) - curNode->vPosHeart(parent->fHeart));

        curNode->updateNorm();

        curNode->setRoll(rollFunc->getValue(length + curNode->fVel/F_HZ)*(curNode->fVel/F_HZ)); //rollFunc->getValue((float)(i+1)/numNodes*fAngle)); //360./numNodes*(i+1));

        if(bOrientation == EULER) {
            curNode->setRoll(pureRollChange/F_HZ);
            artificialRoll += pureRollChange/F_HZ;
        }

        artificialRoll += rollFunc->getValue(length + curNode->fVel/F_HZ)*(curNode->fVel/F_HZ);
        while(artificialRoll > 180.f) {
            artificialRoll -= 360.f;
        }
        while(artificialRoll < -180.f) {
            artificialRoll += 360.f;
        }

        curNode->fDistFromLast = glm::distance(curNode->vPosHeart(parent->fHeart), prevNode->vPosHeart(parent->fHeart));
        curNode->fTotalLength = prevNode->fTotalLength + curNode->fDistFromLast;
        curNode->fHeartDistFromLast = glm::distance(curNode->vPos, prevNode->vPos);
        curNode->fTotalHeartLength = prevNode->fTotalHeartLength + curNode->fHeartDistFromLast;
        curNode->fRollSpeed = rollFunc->getValue(length + curNode->fVel/F_HZ)*curNode->fVel;

        if(bOrientation == 1) {
            curNode->fRollSpeed += pureRollChange;
        }

        if(bSpeed) {
            curNode->fEnergy -= (curNode->fVel*curNode->fVel*curNode->fVel/F_HZ * parent->fResistance);
            curNode->fVel = sqrt(2.f*(curNode->fEnergy-9.80665*(curNode->vPosHeart(parent->fHeart*0.9f).y+curNode->fTotalLength*parent->fFriction)));
        } else {
            curNode->fVel = this->fVel;
            curNode->fEnergy = 0.5*fVel*fVel + 9.80665f*(curNode->vPosHeart(parent->fHeart*0.9f).y + curNode->fTotalLength*parent->fFriction);
        }


        calcDirFromLast(i+1);
        float temp = cos(fabs(curNode->getPitch())*F_PI/180.f);
        float forceAngle = sqrt(temp*temp*curNode->fYawFromLast*curNode->fYawFromLast + curNode->fPitchFromLast*curNode->fPitchFromLast);//deltaAngle;
        curNode->fAngleFromLast = forceAngle;

        glm::vec3 forceVec;
        if(fabs(deltaAngle) < std::numeric_limits<float>::epsilon()) {
            forceVec = glm::vec3(0.f, 1.f, 0.f);
        } else {
            float normalDAngle = F_PI/180.f*(-curNode->fPitchFromLast * cos(curNode->fRoll*F_PI/180.) - temp*curNode->fYawFromLast*sin(curNode->fRoll*F_PI/180.));
            float lateralDAngle = F_PI/180.f*(curNode->fPitchFromLast * sin(curNode->fRoll*F_PI/180.) - temp*curNode->fYawFromLast*cos(curNode->fRoll*F_PI/180.));

            forceVec = glm::vec3(0.f, 1.f, 0.f) + lateralDAngle*curNode->fVel*F_HZ/9.80665f * curNode->vLat + normalDAngle*curNode->fHeartDistFromLast*F_HZ*F_HZ/9.80665f * curNode->vNorm;
        }
        curNode->forceNormal = - glm::dot(forceVec, glm::normalize(curNode->vNorm));
        curNode->forceLateral = - glm::dot(forceVec, glm::normalize(curNode->vLat));

        this->length += curNode->fDistFromLast;
        if(curNode->fVel < 0.01) break;
        ++i;
    }
    while(lNodes.size() > 1+i) {
        delete lNodes.at(1+i);
        lNodes.removeAt(1+i);
    }
    if(lNodes.size()) {
        length = lNodes.last()->fTotalLength - lNodes.first()->fTotalLength;
    } else {
        length = 0;
    }
    return returnval;
}

float secgeometric::getMaxArgument()
{
    float min = std::numeric_limits<float>::max();
    if(rollFunc->lockedFunc() == -1) {
        min = rollFunc->getMaxArgument();
    }
    if(normForce->lockedFunc() == -1) {
        float m = normForce->getMaxArgument();
        min = m < min ? m : min;
    }
    if(latForce->lockedFunc() == -1) {
        float m = latForce->getMaxArgument();
        min = m < min ? m : min;
    }
    return min;
}

void secgeometric::saveSection(std::fstream& file)
{
    file << "GEO";
    writeBytes(&file, (const char*)&bSpeed, sizeof(bool));

    int namelength = sName.length();
    std::string name = sName.toStdString();

    writeBytes(&file, (const char*)&namelength, sizeof(int));
    file << name;

    writeBytes(&file, (const char*)&fVel, sizeof(float));
    writeBytes(&file, (const char*)&iTime, sizeof(int));
    writeBytes(&file, (const char*)&bOrientation, sizeof(bool));
    writeBytes(&file, (const char*)&bArgument, sizeof(bool));
    rollFunc->saveFunction(file);
    normForce->saveFunction(file);
    latForce->saveFunction(file);
}

void secgeometric::loadSection(std::fstream& file)
{
    bSpeed = readBool(&file);

    int namelength = readInt(&file);
    sName = QString(readString(&file, namelength).c_str());


    fVel = readFloat(&file);
    iTime = readInt(&file);
    bOrientation = readBool(&file);
    bArgument = readBool(&file);
    rollFunc->loadFunction(file);
    normForce->loadFunction(file);
    latForce->loadFunction(file);
}


void secgeometric::legacyLoadSection(std::fstream& file)
{
    bSpeed = readBool(&file);

    int namelength = readInt(&file);
    sName = QString(readString(&file, namelength).c_str());


    fVel = readFloat(&file);
    iTime = readInt(&file);
    bOrientation = readBool(&file);
    bArgument = readBool(&file);
    rollFunc->legacyLoadFunction(file);
    normForce->legacyLoadFunction(file);
    latForce->legacyLoadFunction(file);
}

void secgeometric::saveSection(std::stringstream& file)
{
    file << "GEO";
    writeBytes(&file, (const char*)&bSpeed, sizeof(bool));

    int namelength = sName.length();
    std::string name = sName.toStdString();

    writeBytes(&file, (const char*)&namelength, sizeof(int));
    file << name;

    writeBytes(&file, (const char*)&fVel, sizeof(float));
    writeBytes(&file, (const char*)&iTime, sizeof(int));
    writeBytes(&file, (const char*)&bOrientation, sizeof(bool));
    writeBytes(&file, (const char*)&bArgument, sizeof(bool));
    rollFunc->saveFunction(file);
    normForce->saveFunction(file);
    latForce->saveFunction(file);
}

void secgeometric::loadSection(std::stringstream& file)
{
    bSpeed = readBool(&file);

    int namelength = readInt(&file);
    sName = QString(readString(&file, namelength).c_str());


    fVel = readFloat(&file);
    iTime = readInt(&file);
    bOrientation = readBool(&file);
    bArgument = readBool(&file);
    rollFunc->loadFunction(file);
    normForce->loadFunction(file);
    latForce->loadFunction(file);
}

bool secgeometric::isInFunction(int index, subfunction* func)
{
    if(func == NULL) return false;
    if(bArgument == DISTANCE) {
        float dist = 0;
        if(index >= lNodes.size()) return false;
        for(int i = 1; i <= index; ++i) {
            dist += lNodes[i]->fHeartDistFromLast;
        }
        if(dist >= func->minArgument && dist <= func->maxArgument) {
            return true;
        }
        return false;
    } else if(index/F_HZ >= func->minArgument && index/F_HZ <= func->maxArgument) {
        return true;
    }
    return false;
}

bool secgeometric::isLockable(function* _func)
{
    if(_func == rollFunc) {
        if(normForce->lockedFunc() != -1 && latForce->lockedFunc() != -1) return false;
    } else if(_func == normForce) {
        if(rollFunc->lockedFunc() != -1 && latForce->lockedFunc() != -1) return false;
    } else if(_func == latForce) {
        if(rollFunc->lockedFunc() != -1 && normForce->lockedFunc() != -1) return false;
    } else {
        lenAssert(0 && "no such function");
        return false;
    }
    return true;
}
