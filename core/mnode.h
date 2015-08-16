#ifndef MNODE_H
#define MNODE_H

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

#include <QList>
#include <sstream>
#include <fstream>
#include "lenassert.h"

#define F_HZ (1000.f)

typedef struct bezier_s
{
    glm::vec3 Kp1;
    glm::vec3 Kp2;
    glm::vec3 P1;
    float roll;
    bool contRoll;
    bool equalDist;
    bool relRoll;

    float ptf;
    float fvdRoll;
    float length;
    int numNodes;
    float fVel;
} bezier_t;

class mnode
{
public:
    mnode();
    mnode(glm::vec3 getPos, glm::vec3 getDir, float getRoll, float getVel, float getNForce, float getLateral);
    void setRoll(float dRoll);
    void updateRoll();
    void updateNorm() { vNorm = glm::cross(vDir, vLat); }
    void changePitch(float dAngle, bool inverted);
    void changeYaw(float dAngle);
    float getPitchChange() { return fPitchFromLast*F_HZ; }
    float getYawChange() { return fYawFromLast*F_HZ; }
    float fPosHeartx(float fHeart) { return vPos.x+vNorm.x*fHeart; }
    float fPosHearty(float fHeart) { return vPos.y+vNorm.y*fHeart; }
    float fPosHeartz(float fHeart) { return vPos.z+vNorm.z*fHeart; }
    glm::vec3 vLatHeart(float fHeart);
    glm::vec3 vDirHeart(float fHeart);
    glm::vec3 vPosHeart(float fHeart) { return vPos + fHeart*vNorm; }

    glm::vec3 vRelPos(float y, float x, float z = 0.f) { return vPos - y*vNorm + x*vLatHeart(-y) + z*vDirHeart(-y); }

    void exportNode(QList<bezier_t*> &bezList, mnode* last, mnode* mid, mnode* anchor, float fHeart, float fRollThresh);

    float getPitch() { return glm::atan(vDir.y, glm::sqrt(vDir.x*vDir.x+vDir.z*vDir.z))*180/F_PI; }
    float getDirection() { return glm::atan(-vDir.x, -vDir.z)*180/F_PI; }


    void saveNode(std::fstream& file);
    void legacyLoadNode(std::fstream& file);

    void calcSmoothForces();

    glm::vec3 vPos;
    glm::vec3 vDir;
    glm::vec3 vLat;
    glm::vec3 vNorm;
    float fRoll;
    float fVel;
    float fEnergy;
    float forceNormal;
    float forceLateral;
    float smoothNormal;
    float smoothLateral;
    float fDistFromLast;
    float fHeartDistFromLast;
    float fAngleFromLast;
    float fTrackAngleFromLast;
    float fDirFromLast;
    float fPitchFromLast;
    float fYawFromLast;
    float fRollSpeed;
    float fSmoothSpeed;
    float fFlexion() { return fDistFromLast <= 0.0 ? 0.0f : fTrackAngleFromLast / fDistFromLast; }
    float fTotalLength;
    float fTotalHeartLength;
};

#endif // MNODE_H
