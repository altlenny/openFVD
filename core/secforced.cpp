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

#include "secforced.h"
#include "mnode.h"
#include "exportfuncs.h"

secforced::~secforced()
{
    delete normForce;
    delete latForce;
}

secforced::secforced(track* getParent, mnode* first, float gettime): section(getParent, forced, first)
{
    this->iTime = (int)(gettime+0.5);
    this->length = 0.0;
    rollFunc->changeLength(1.f, 0);
    normForce = new function(0, 1, this->lNodes[0]->forceNormal, this->lNodes[0]->forceNormal, this, funcNormal);
    latForce = new function(0, 1, this->lNodes[0]->forceLateral, this->lNodes[0]->forceLateral, this, funcLateral);

    this->bOrientation = QUATERNION;
    this->bArgument = TIME;
    bSpeed = 1;
    fVel = 10;
}

int secforced::updateSection(int node)
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

    if(node >= lNodes.size()-1 && node > 0) node = lNodes.size()-2;

    if(lNodes.size() > 1 && this->parent->lSections.at(this->parent->lSections.size()-1) != this) {
        lNodes.removeLast(); // disjoint this section from the next one
    }

    if(node == 0) {
        lNodes.at(0)->updateNorm();

        float diff = lNodes.at(0)->forceNormal; // - normForce->funcList.at(0)->startValue;
        lenAssert(diff==diff);
        if(diff != diff) {
            lNodes.append(new mnode(*(this->lNodes.at(0))));
            return node;
        }
        normForce->funcList.at(0)->translateValues(diff);
        normForce->translateValues(normForce->funcList.at(0));

        diff = lNodes.at(0)->forceLateral; // - latForce->funcList.at(0)->startValue;
        lenAssert(diff==diff);
        if(diff != diff) {
            lNodes.append(new mnode(*(this->lNodes.at(0))));
            return node;
        }
        latForce->funcList.at(0)->translateValues(diff);
        latForce->translateValues(latForce->funcList.at(0));

        diff = lNodes.at(0)->fRollSpeed; // - rollFunc->funcList.at(0)->startValue;
        if(bOrientation == 1) {
            diff += glm::dot(lNodes[0]->vDir, glm::vec3(0.f, 1.f, 0.f))*lNodes[0]->getYawChange();
        }
        rollFunc->funcList.at(0)->translateValues(diff);
        rollFunc->translateValues(rollFunc->funcList.at(0));
    }

    int i;
    for(i = node; i < numNodes; i++)
    {
        if(i >= lNodes.size()-1) {
            lNodes.append(new mnode(*(this->lNodes.at(i))));
        }

        mnode* prevNode = lNodes[i];
        mnode* curNode = lNodes[i+1];
        curNode->vPos = prevNode->vPos;
        curNode->fVel = prevNode->fVel;
        curNode->fEnergy = prevNode->fEnergy;

        glm::vec3 forceVec = - normForce->getValue((i+1)/F_HZ) * prevNode->vNorm - latForce->getValue((i+1)/F_HZ) * prevNode->vLat - glm::vec3(0.f, 1.f, 0.f);

        curNode->forceNormal = normForce->getValue((i+1)/F_HZ);
        curNode->forceLateral = latForce->getValue((i+1)/F_HZ);

        float nForce = - glm::dot(forceVec, glm::normalize(prevNode->vNorm))*9.80665f;
        float lForce = - glm::dot(forceVec, glm::normalize(prevNode->vLat))*9.80665f;

        float estVel = fabs(prevNode->fHeartDistFromLast) < std::numeric_limits<float>::epsilon() ? prevNode->fVel : prevNode->fHeartDistFromLast*F_HZ;

        curNode->vDir = glm::normalize(glm::angleAxis(nForce/F_HZ/estVel, prevNode->vLat) * glm::angleAxis(-lForce/prevNode->fVel/F_HZ, prevNode->vNorm) * prevNode->vDir);
        curNode->vLat = glm::normalize(glm::angleAxis(-lForce/prevNode->fVel/F_HZ, prevNode->vNorm) * prevNode->vLat);

        curNode->updateNorm();

        curNode->vPos += curNode->vDir*(curNode->fVel/(2.f*F_HZ)) + prevNode->vDir*(curNode->fVel/(2.f*F_HZ)) + (prevNode->vPosHeart(parent->fHeart) - curNode->vPosHeart(parent->fHeart));

        curNode->fRollSpeed = 0.f;
        curNode->setRoll(rollFunc->getValue((i+1)/F_HZ)/F_HZ); // - rollFunc->getValue(i/1000.f));
        calcDirFromLast(i+1);
        if(bOrientation == EULER || rollFunc->getSubfunction((i+1)/F_HZ)->degree == tozero) {
            curNode->setRoll(glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f))*curNode->fYawFromLast);
            curNode->fRollSpeed += glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f))*curNode->fYawFromLast*F_HZ;
        }


        curNode->updateNorm();

        curNode->fDistFromLast = glm::distance(curNode->vPosHeart(parent->fHeart), prevNode->vPosHeart(parent->fHeart));
        curNode->fTotalLength = prevNode->fTotalLength + curNode->fDistFromLast;
        curNode->fHeartDistFromLast = glm::distance(curNode->vPos, prevNode->vPos);
        curNode->fTotalHeartLength = prevNode->fTotalHeartLength + curNode->fHeartDistFromLast;
        curNode->fRollSpeed += rollFunc->getValue((i+1)/F_HZ);  // /1000.f/curNode->fDistFromLast;

        calcDirFromLast(i+1);
        float temp = cos(fabs(curNode->getPitch())*F_PI/180.f);
        float forceAngle = sqrt(temp*temp*curNode->fYawFromLast*curNode->fYawFromLast + curNode->fPitchFromLast*curNode->fPitchFromLast);//deltaAngle;


        curNode->fAngleFromLast = forceAngle;

        if(bSpeed) {
            curNode->fEnergy -= (curNode->fVel*curNode->fVel*curNode->fVel/F_HZ * parent->fResistance);
            curNode->fVel = sqrt(2.f*(curNode->fEnergy-9.80665f*(curNode->vPosHeart(parent->fHeart*0.9f).y+curNode->fTotalLength*parent->fFriction)));
        } else {
            curNode->fVel = this->fVel;
            curNode->fEnergy = 0.5*fVel*fVel + 9.80665f*(curNode->vPosHeart(parent->fHeart*0.9f).y + curNode->fTotalLength*parent->fFriction);
        }


        if(fabs(curNode->fAngleFromLast) < std::numeric_limits<float>::epsilon()) {
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
    if(lNodes.size()) {
        length = lNodes.last()->fTotalLength - lNodes.first()->fTotalLength;
    } else {
        length = 0;
    }
    return node;
}

int secforced::updateDistanceSection(int node)
{
    node = node < 0 ? 0 : node;

    int i = 0;
    this->length = 0.f;
    float hDist = 0.f;
    while(length < (float)node/F_HZ && i+1 < lNodes.size()) {
        hDist += lNodes[++i]->fHeartDistFromLast;
        length += lNodes[i]->fDistFromLast;
    }

    if(i >= lNodes.size()-1 && i > 0) {
        i = lNodes.size()-2;
    }

    if(lNodes.size() > 1 && this->parent->lSections.at(this->parent->lSections.size()-1) != this) {
        lNodes.removeLast();
    }

    if(i == 0) {
        lNodes.at(0)->updateNorm();

        float diff = lNodes.at(0)->forceNormal; // - normForce->funcList.at(0)->startValue;
        lenAssert(diff==diff);
        if(diff != diff) {
            lNodes.append(new mnode(*(this->lNodes.at(0))));
            return node;
        }
        normForce->funcList.at(0)->translateValues(diff);
        normForce->translateValues(normForce->funcList.at(0));

        diff = lNodes.at(0)->forceLateral; // - latForce->funcList.at(0)->startValue;
        lenAssert(diff==diff);
        if(diff != diff) {
            lNodes.append(new mnode(*(this->lNodes.at(0))));
            return node;
        }
        latForce->funcList.at(0)->translateValues(diff);
        latForce->translateValues(latForce->funcList.at(0));

        diff = lNodes.at(0)->fRollSpeed/lNodes[0]->fVel; // - rollFunc->funcList.at(0)->startValue;
        if(bOrientation == 1) {
            diff += glm::dot(lNodes[0]->vDir, glm::vec3(0.f, 1.f, 0.f))*lNodes[0]->getYawChange()/lNodes[0]->fVel;
        }
        rollFunc->funcList.at(0)->translateValues(diff);
        rollFunc->translateValues(rollFunc->funcList.at(0));
    }

    //qDebug("deleting complete");

    int retval = i;
    float end = this->getMaxArgument();

    while(length < end) {
        if(i >= lNodes.size()-1) {
            lNodes.append(new mnode(*(this->lNodes.at(i))));
        }

        mnode* prevNode = lNodes[i];
        mnode* curNode = lNodes[i+1];
        curNode->vPos = prevNode->vPos;
        curNode->fVel = prevNode->fVel;
        curNode->fEnergy = prevNode->fEnergy;

        glm::vec3 forceVec = - normForce->getValue(length+prevNode->fVel/F_HZ) * prevNode->vNorm - latForce->getValue(length+prevNode->fVel/F_HZ) * prevNode->vLat - glm::vec3(0.f, 1.f, 0.f);

        curNode->forceNormal = normForce->getValue(length+prevNode->fVel/F_HZ);
        curNode->forceLateral = latForce->getValue(length+prevNode->fVel/F_HZ);

        float nForce = - glm::dot(forceVec, glm::normalize(prevNode->vNorm))*9.80665;
        float lForce = - glm::dot(forceVec, glm::normalize(prevNode->vLat))*9.80665*180/F_PI;

        float estVel = fabs(prevNode->fHeartDistFromLast) < std::numeric_limits<float>::epsilon() ? prevNode->fVel : prevNode->fHeartDistFromLast*F_HZ;

        curNode->vDir = glm::normalize(glm::angleAxis(nForce/F_HZ/estVel, prevNode->vLat) * glm::angleAxis(-lForce/prevNode->fVel/F_HZ, prevNode->vNorm) * prevNode->vDir);
        curNode->vLat = glm::normalize(glm::angleAxis(-lForce/prevNode->fVel/F_HZ, prevNode->vNorm) * prevNode->vLat);

        curNode->updateNorm();

        curNode->vPos += curNode->vDir*(curNode->fVel/(2.f*F_HZ)) + prevNode->vDir*(curNode->fVel/(2.f*F_HZ)) + (prevNode->vPosHeart(parent->fHeart) - curNode->vPosHeart(parent->fHeart));

        curNode->setRoll(rollFunc->getValue(length+curNode->fVel/F_HZ)*(curNode->fVel/F_HZ)); // - rollFunc->getValue(i/1000.f));

        curNode->fRollSpeed = 0.f;

        if(bOrientation == EULER) {
            calcDirFromLast(i+1);
            curNode->setRoll(glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f))*curNode->fYawFromLast);
            curNode->fRollSpeed += glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f))*curNode->fYawFromLast*F_HZ;
        }

        curNode->fDistFromLast = glm::distance(curNode->vPosHeart(parent->fHeart), prevNode->vPosHeart(parent->fHeart));
        curNode->fTotalLength = prevNode->fTotalLength + curNode->fDistFromLast;
        curNode->fHeartDistFromLast = glm::distance(curNode->vPos, prevNode->vPos);
        curNode->fTotalHeartLength = prevNode->fTotalHeartLength + curNode->fHeartDistFromLast;
        curNode->fRollSpeed += rollFunc->getValue(length+curNode->fVel/F_HZ) *curNode->fVel;  // /1000.f/curNode->fDistFromLast;

        calcDirFromLast(i+1);
        float temp = cos(fabs(curNode->getPitch())*F_PI/180.f);
        float forceAngle = sqrt(temp*temp*curNode->fYawFromLast*curNode->fYawFromLast + curNode->fPitchFromLast*curNode->fPitchFromLast);//deltaAngle;
        curNode->fAngleFromLast = forceAngle;

        if(bSpeed) {
            curNode->fEnergy -= (curNode->fVel*curNode->fVel*curNode->fVel/F_HZ * parent->fResistance);
            curNode->fVel = sqrt(2.f*(curNode->fEnergy-9.80665f*(curNode->vPosHeart(parent->fHeart*0.9f).y+curNode->fTotalLength*parent->fFriction)));
        } else {
            curNode->fVel = this->fVel;
            curNode->fEnergy = 0.5*fVel*fVel + 9.80665f*(curNode->vPosHeart(parent->fHeart*0.9f).y + curNode->fTotalLength*parent->fFriction);
        }

        if(fabs(curNode->fAngleFromLast) < std::numeric_limits<float>::epsilon()) {
            forceVec = glm::vec3(0.f, 1.f, 0.f);
        } else {
            float normalDAngle = F_PI/180.f*(- curNode->fPitchFromLast * cos(curNode->fRoll*F_PI/180.) - temp*curNode->fYawFromLast*sin(curNode->fRoll*F_PI/180.));
            float lateralDAngle = F_PI/180.f*(curNode->fPitchFromLast * sin(curNode->fRoll*F_PI/180.) - temp*curNode->fYawFromLast*cos(curNode->fRoll*F_PI/180.));

            forceVec = glm::vec3(0.f, 1.f, 0.f) + lateralDAngle*curNode->fVel*F_HZ/9.80665f * curNode->vLat + normalDAngle*curNode->fHeartDistFromLast*F_HZ*F_HZ/9.80665f * curNode->vNorm;
        }
        curNode->forceNormal = - glm::dot(forceVec, glm::normalize(curNode->vNorm));
        curNode->forceLateral = - glm::dot(forceVec, glm::normalize(curNode->vLat));

        this->length += curNode->fDistFromLast;
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
    return retval;
}

float secforced::getMaxArgument()
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

void secforced::saveSection(std::fstream& file)
{
    file << "FRC";
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

void secforced::loadSection(std::fstream& file)
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

void secforced::legacyLoadSection(std::fstream& file)
{
    bSpeed = readBool(&file);

    int namelength = readInt(&file);
    sName = QString(readString(&file, namelength).c_str());


    bSpeed = true;
    iTime = readInt(&file);
    bOrientation = readBool(&file);
    bArgument = readBool(&file);
    rollFunc->legacyLoadFunction(file);
    normForce->legacyLoadFunction(file);
    latForce->legacyLoadFunction(file);
}

void secforced::saveSection(std::stringstream& file)
{
    file << "FRC";
    writeNulls(&file, 1);

    int namelength = sName.length();
    std::string name = sName.toStdString();

    writeBytes(&file, (const char*)&namelength, sizeof(int));
    file << name;

    writeBytes(&file, (const char*)&iTime, sizeof(int));
    writeBytes(&file, (const char*)&bOrientation, sizeof(bool));
    writeBytes(&file, (const char*)&bArgument, sizeof(bool));
    rollFunc->saveFunction(file);
    normForce->saveFunction(file);
    latForce->saveFunction(file);
}

void secforced::loadSection(std::stringstream& file)
{
    readNulls(&file, 1);

    int namelength = readInt(&file);
    sName = QString(readString(&file, namelength).c_str());

    bSpeed = true;
    iTime = readInt(&file);
    bOrientation = readBool(&file);
    bArgument = readBool(&file);
    rollFunc->loadFunction(file);
    normForce->loadFunction(file);
    latForce->loadFunction(file);
}

bool secforced::isInFunction(int index, subfunction* func)
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

bool secforced::isLockable(function* _func)
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
