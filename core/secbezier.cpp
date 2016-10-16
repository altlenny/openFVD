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

#include "secbezier.h"
#include "exportfuncs.h"

secbezier::secbezier(track* getParent, mnode* first) : section(getParent, bezier, first)
{
    this->bOrientation = QUATERNION;
    this->bArgument = TIME;
    bSpeed = 0;
    fVel = 10;
}

secbezier::~secbezier()
{
    for(int i = 0; i < bezList.size(); ++i)
    {
        delete bezList[i];
    }
}

int secbezier::updateSection(int node)
{
    Q_UNUSED(node);
    QList<float> tList;
    while(lNodes.size() > 1)
    {
		/*if(lNodes.size() > 2 || this->parent->lSections.at(this->parent->lSections.size()-1) == this)
        {
            delete lNodes.at(1);
		}*/
        lNodes.removeAt(1);
    }
	lNodes[0].updateNorm();


	int cur = 0, lastcur = 0;
    float t = 0.f;

	mnode* curNode = &lNodes[0], *prevNode = NULL;
    for(int b = 0; b < bezList.size()-1; ++b)
    {
        while(t < 1.f)
        {
            tList.append(t);

            int bnext = (b+1)%bezList.size();
            float t1 = 1.f-t;
            if(cur >= lNodes.size())
            {
				lNodes.append(lNodes.back());
            }
			prevNode = &lNodes[glm::max(cur-1, 0)];
			curNode = &lNodes[cur];
            curNode->fEnergy = prevNode->fEnergy;
			curNode->vPos = t1*t1*t1*bezList[b]->P1 + 3.f*t1*t1*t*bezList[b]->Kp2 + 3.f*t1*t*t*bezList[bnext]->Kp1 + t*t*t*bezList[bnext]->P1;


			curNode->fRoll = t1*bezList[b]->fvdRoll + t*bezList[bnext]->fvdRoll;
            curNode->fRoll *= 180.f/F_PI;

			glm::vec3 diff1 = bezList[b]->Kp2-bezList[b]->P1;
			glm::vec3 diff2 = bezList[bnext]->Kp1-bezList[b]->Kp2;
			glm::vec3 diff3 = bezList[bnext]->P1-bezList[bnext]->Kp1;

            curNode->vDir = t1*t1*diff1 + 2*t1*t*diff2 + t*t*diff3;

            float lengthDir = glm::length(curNode->vDir);

            curNode->vDir = glm::normalize(curNode->vDir);

            curNode->vLat.x = -curNode->vDir.z;
            curNode->vLat.y = 0;
            curNode->vLat.z = curNode->vDir.x;

            if(glm::length(curNode->vLat) < std::numeric_limits<float>::epsilon())
            {
                curNode->vLat = glm::normalize(glm::cross(curNode->vNorm, curNode->vDir));
            }

            curNode->setRoll(0.f);//curNode->fRoll);

            if(cur)
            {
				curNode->fHeartDistFromLast = glm::distance(curNode->vPos, lNodes[cur-1].vPos);
                curNode->fTotalHeartLength += curNode->fHeartDistFromLast;
                //curNode->fVel = curNode->fHeartDistFromLast*F_HZ;
                curNode->fDistFromLast = glm::distance(curNode->vPosHeart(parent->fHeart), prevNode->vPosHeart(parent->fHeart));
                curNode->fTotalLength = prevNode->fTotalLength + curNode->fDistFromLast;
            }

			float vel = bezList[b]->fVel;
            if(vel == 0.f)
            {
                //float heightDiff = curNode->vPosHeart(parent->fHeart*0.9f).y - prevNode->vPosHeart(parent->fHeart*0.9f).y;
                //vel = glm::sqrt(prevNode->fVel*prevNode->fVel - 2*9.08665*heightDiff);
                //vel = glm::sqrt(vel * vel - 2*curNode->fDistFromLast*parent->fFriction); // glm::length(forceVec + glm::vec3(0, 1.f, 0))
                curNode->fEnergy -= (curNode->fVel*curNode->fVel*curNode->fVel/F_HZ * parent->fResistance);
                vel = sqrt(2.f*(curNode->fEnergy-9.80665*(curNode->vPosHeart(parent->fHeart*0.9f).y+curNode->fTotalLength*parent->fFriction)));
            }
            else
            {
                curNode->fEnergy = 0.5*vel*vel + F_G*(curNode->vPosHeart(parent->fHeart*0.9f).y + curNode->fTotalLength*parent->fFriction);
            }
            if(vel < 1.f)
            {
                vel = 1.f;
                curNode->fEnergy = 0.5*vel*vel + F_G*(curNode->vPosHeart(parent->fHeart*0.9f).y + curNode->fTotalLength*parent->fFriction);
            }
            if(vel != vel)
            {
                vel = 10.f;
                curNode->fEnergy = 0.5*vel*vel + F_G*(curNode->vPosHeart(parent->fHeart*0.9f).y + curNode->fTotalLength*parent->fFriction);
            }
            curNode->fVel = vel;
            t += 1.f/(3000.f*lengthDir/vel);
            ++cur;
        }
        t -= 1.f;
		bezList[b]->length = lNodes[cur-1].fTotalHeartLength - lNodes[lastcur].fTotalHeartLength;
		bezList[b]->numNodes = cur-1-lastcur;
        lastcur = cur-1;
    }

    if(!tList.size()) return 0;

    int b = 0;
    float correction = 0;

	bezList[0]->fvdRoll = bezList[0]->roll;

    for(int i = 0; i < lNodes.size(); ++i)
    {
        calcDirFromLast(i); // now get roll change you need to get from a to b
        if(i && tList[i] < tList[i-1])
        {
            ++b;
			if(bezList[b]->relRoll)
            {
				bezList[b]->fvdRoll = bezList[b-1]->fvdRoll + correction*F_PI/180.f + bezList[b]->roll;
				bezList[b-1]->ptf = bezList[b]->roll;
            }
            else
            {
				bezList[b]->fvdRoll = bezList[b]->roll;
				bezList[b-1]->ptf = bezList[b]->fvdRoll - bezList[b-1]->fvdRoll - correction*F_PI/180.f;
				if(fabs(bezList[b-1]->ptf) > F_PI)
                {
					bezList[b-1]->ptf += bezList[b-1]->ptf > 0.f ? -2.f*F_PI : 2.f*F_PI;
                }
            }
            correction = 0.f;
        }
		correction -= glm::dot(lNodes[i].vDir, glm::vec3(0.f, -1.f, 0.f))*lNodes[i].fYawFromLast;

		float fRoll = bezList[b]->fvdRoll*180.f/F_PI;

		lNodes[i].setRoll(fRoll + correction);
    }

    b = 0;
    int bNext = 1;
    float startVal, endVal, area, a1, b1, c1;
    float value = 0.f;
    for(int i = 0; i < lNodes.size(); ++i)
    {
        float tNext;
        if(i == lNodes.size()-1 || tList[i+1] < tList[i])
        {
            tNext = 1.f;
        }
        else
        {
            tNext = tList[i+1];
        }

        if(i && tList[i] < tList[i-1])
        {
            ++b;
            bNext = (b+1)%bezList.size();
            value = 0.f;

            startVal = endVal;

			if(bezList[bNext]->contRoll)
            {
				endVal = (bezList[b]->length*bezList[b]->ptf + bezList[bNext]->length*bezList[bNext]->ptf)/(bezList[b]->length + bezList[bNext]->length);//*(tNext - tList[i]);
            }
            else
            {
                endVal = 0.f;
            }
			area = bezList[b]->ptf;

            a1 = 3.f*startVal + 3.f*endVal - 6.f*area;
            b1 = 6.f*area - 4.f*startVal - 2.f*endVal;
            c1 = startVal;
        }
        else if(!i)
        {
            startVal = 0.f;
            value = 0.f;
			if(bezList.size() > 1 && bezList[1]->contRoll)
            {
				endVal = (bezList[b]->length*bezList[b]->ptf + bezList[bNext]->length*bezList[bNext]->ptf)/(bezList[b]->length + bezList[bNext]->length);//*(tNext - tList[i]);
            }
            else
            {
                endVal = 0.f;
            }
			area = bezList[0]->ptf;

            a1 = 3.f*startVal + 3.f*endVal - 6.f*area;
            b1 = 6.f*area - 4.f*startVal - 2.f*endVal;
            c1 = startVal;
        }
        value += (c1 + tList[i]*(b1+ a1*tList[i]))*180.f/F_PI * (tNext - tList[i]);

		lNodes[i].setRoll(value);

		//lNodes[i].vPos = lNodes[i].vPosHeart(-parent->fHeart);

        if(i)
        {
			lNodes[i].fDistFromLast = glm::distance(lNodes[i].vPosHeart(parent->fHeart), lNodes[i-1].vPosHeart(parent->fHeart));
			lNodes[i].fTotalLength = lNodes[i-1].fTotalLength + lNodes[i].fDistFromLast;
			lNodes[i].fRollSpeed = (c1 + tList[i]*(b1+ a1*tList[i]))*F_HZ*180.f/F_PI * (tNext - tList[i]);//(lNodes[i].fRoll - lNodes[i-1].fRoll - glm::dot(lNodes[i].vDir, glm::vec3(0.f, -1.f, 0.f))*lNodes[i].fYawFromLast)*1000.f;
			lNodes[i].fHeartDistFromLast = glm::distance(lNodes[i].vPos, lNodes[i-1].vPos);
			lNodes[i].fTotalHeartLength += lNodes[i].fHeartDistFromLast;
        }

		/*lNodes[i].fRollSpeed *= -1;
		lNodes[i].vDir = lNodes[i].vDirHeart(parent->fHeart);
		lNodes[i].vLat = lNodes[i].vLatHeart(parent->fHeart);
		lNodes[i].fRollSpeed *= -1;*/


        calcDirFromLast(i);
		float temp = cos(fabs(lNodes[i].getPitch())*F_PI/180.f);
		float forceAngle = sqrt(temp*temp*lNodes[i].fYawFromLast*lNodes[i].fYawFromLast + lNodes[i].fPitchFromLast*lNodes[i].fPitchFromLast);//deltaAngle;
		lNodes[i].fAngleFromLast = forceAngle;

        glm::vec3 forceVec;
        if(fabs(forceAngle) < std::numeric_limits<float>::epsilon())
        {
            forceVec = glm::vec3(0.f, 1.f, 0.f);
        }
        else
        {
			//forceVec = glm::vec3(0.f, 1.f, 0.f) + (float)((lNodes[i].fHeartDistFromLast*1000.f*lNodes[i].fAngleFromLast*F_PI/180.f) / (9.80665*0.001f)) * glm::normalize(glm::vec3(glm::rotate(lNodes[i].fDirFromLast, -lNodes[i].vDir)*glm::vec4(-lNodes[i].vNorm, 0.f)));
			float normalDAngle = F_PI/180.f*(- lNodes[i].fPitchFromLast * cos(lNodes[i].fRoll*F_PI/180.) - temp*lNodes[i].fYawFromLast*sin(lNodes[i].fRoll*F_PI/180.));
			float lateralDAngle = F_PI/180.f*(lNodes[i].fPitchFromLast * sin(lNodes[i].fRoll*F_PI/180.) - temp*lNodes[i].fYawFromLast*cos(lNodes[i].fRoll*F_PI/180.));

			forceVec = glm::vec3(0.f, 1.f, 0.f) + lateralDAngle*lNodes[i].fVel*F_HZ/F_G * lNodes[i].vLat + normalDAngle*lNodes[i].fHeartDistFromLast*F_HZ*F_HZ/F_G * lNodes[i].vNorm;
        }
		lNodes[i].forceNormal = - glm::dot(forceVec, glm::normalize(lNodes[i].vNorm));
		lNodes[i].forceLateral = - glm::dot(forceVec, glm::normalize(lNodes[i].vLat));

    }
	if(lNodes.size()) length = lNodes.last().fTotalLength - lNodes.first().fTotalLength;
    else length = 0;
    return node;
}

void secbezier::saveSection(std::fstream& file)
{
    file << "BEZ";
    int namelength = sName.length();
    std::string name = sName.toStdString();

    writeBytes(&file, (const char*)&namelength, sizeof(int));
    file << name;

    int bezcount = bezList.size();
    writeBytes(&file, (const char*)&bezcount, sizeof(int));
    for(int i = 0; i < bezcount; ++i)
    {
		writeBytes(&file, (const char*)&bezList[i]->P1, sizeof(glm::vec3));
		writeBytes(&file, (const char*)&bezList[i]->Kp1, sizeof(glm::vec3));
		writeBytes(&file, (const char*)&bezList[i]->Kp2, sizeof(glm::vec3));
		writeBytes(&file, (const char*)&bezList[i]->contRoll, sizeof(bool));
		writeBytes(&file, (const char*)&bezList[i]->relRoll, sizeof(bool));
		writeBytes(&file, (const char*)&bezList[i]->roll, sizeof(float));
    }

    int supcount = supList.size();
    writeBytes(&file, (const char*)&supcount, sizeof(int));
    for(int i = 0; i < supcount; ++i)
    {
        writeBytes(&file, (const char*)&supList[i], sizeof(glm::vec3));
    }
}

void secbezier::loadSection(std::fstream& file)
{
    int namelength = readInt(&file);
    sName = QString(readString(&file, namelength).c_str());

    int bezcount = readInt(&file);
    for(int i = 0; i < bezcount; ++i)
    {
        bezList.append(new bezier_t);
		bezList[i]->P1 = readVec3(&file);
		bezList[i]->Kp1 = readVec3(&file);
		bezList[i]->Kp2 = readVec3(&file);
		bezList[i]->contRoll = readBool(&file);
		bezList[i]->relRoll = readBool(&file);
		bezList[i]->roll = readFloat(&file);
		bezList[i]->fVel = 0;
    }

    int supcount = readInt(&file);
    for(int i = 0; i < supcount; ++i)
    {
        supList.append(readVec3(&file));
    }
}

void secbezier::legacyLoadSection(std::fstream& file)
{
    int namelength = readInt(&file);
    sName = QString(readString(&file, namelength).c_str());

    int bezcount = readInt(&file);
    for(int i = 0; i < bezcount; ++i)
    {
        bezList.append(new bezier_t);
		bezList[i]->P1 = readVec3(&file);
		bezList[i]->Kp1 = readVec3(&file);
		bezList[i]->Kp2 = readVec3(&file);
		bezList[i]->contRoll = readBool(&file);
		bezList[i]->relRoll = readBool(&file);
		bezList[i]->roll = readFloat(&file);
		bezList[i]->fVel = 0;
    }

    int supcount = readInt(&file);
    for(int i = 0; i < supcount; ++i)
    {
        supList.append(readVec3(&file));
    }
}

void secbezier::saveSection(std::stringstream& file)
{
    file << "BEZ";
    int namelength = sName.length();
    std::string name = sName.toStdString();

    writeBytes(&file, (const char*)&namelength, sizeof(int));
    file << name;

    int bezcount = bezList.size();
    writeBytes(&file, (const char*)&bezcount, sizeof(int));
    for(int i = 0; i < bezcount; ++i)
    {
		writeBytes(&file, (const char*)&bezList[i]->P1, sizeof(glm::vec3));
		writeBytes(&file, (const char*)&bezList[i]->Kp1, sizeof(glm::vec3));
		writeBytes(&file, (const char*)&bezList[i]->Kp2, sizeof(glm::vec3));
		writeBytes(&file, (const char*)&bezList[i]->contRoll, sizeof(bool));
		writeBytes(&file, (const char*)&bezList[i]->relRoll, sizeof(bool));
		writeBytes(&file, (const char*)&bezList[i]->roll, sizeof(float));
    }

    int supcount = supList.size();
    writeBytes(&file, (const char*)&supcount, sizeof(int));
    for(int i = 0; i < supcount; ++i)
    {
        writeBytes(&file, (const char*)&supList[i], sizeof(glm::vec3));
    }
}

void secbezier::loadSection(std::stringstream& file)
{
    int namelength = readInt(&file);
    sName = QString(readString(&file, namelength).c_str());

    int bezcount = readInt(&file);
    for(int i = 0; i < bezcount; ++i)
    {
        bezList.append(new bezier_t);
		bezList[i]->P1 = readVec3(&file);
		bezList[i]->Kp1 = readVec3(&file);
		bezList[i]->Kp2 = readVec3(&file);
		bezList[i]->contRoll = readBool(&file);
		bezList[i]->relRoll = readBool(&file);
		bezList[i]->roll = readFloat(&file);
		bezList[i]->fVel = 0;
    }

    int supcount = readInt(&file);
    for(int i = 0; i < supcount; ++i)
    {
        supList.append(readVec3(&file));
    }
}

float secbezier::getMaxArgument()
{
    return 0.f;
}

bool secbezier::isLockable(func* _func)
{
    Q_UNUSED(_func);
    return false;
}

bool secbezier::isInFunction(int index, subfunc* func)
{
    Q_UNUSED(index);
    Q_UNUSED(func);
    return false;
}
