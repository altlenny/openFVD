#ifndef SECTION_H
#define SECTION_H

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
#include <QString>
#include "mnode.h"
#include "function.h"
#include <sstream>

#define EULER true
#define QUATERNION false

#define TIME false
#define DISTANCE true

class track;

enum secType
{
    anchor,
    straight,
    curved,
    forced,
    geometric,
    bezier,
    nolimitscsv
};

class section
{
public:
    section(track* getParent, enum secType _type, mnode* first);
    virtual ~section();
    float length;
    virtual int updateSection(int node = 0) = 0;
    virtual int exportSection(std::fstream *file, mnode* anchor, float mPerNode, float fHeart, glm::vec3& vHeartLat, glm::vec3& Norm, float fRollThresh);
    virtual void fillPointList(QList<glm::vec4> &List, QList<glm::vec3> &Normals, mnode* anchor, float mPerNode, float fHeart);
    virtual void iFillPointList(QList<int> &List, float mPerNode);
    void         Split(QList<int> &List, int l, int r, float total, float min);
    virtual void fFillPointList(QList<int> &List, float mPerNode);
    virtual void saveSection(std::fstream& file) = 0;
    virtual void loadSection(std::fstream& file) = 0;
    virtual void legacyLoadSection(std::fstream& file) = 0;
    virtual void saveSection(std::stringstream& file) = 0;
    virtual void loadSection(std::stringstream& file) = 0;
    virtual float getMaxArgument() = 0;
    virtual bool isLockable(func* _func) = 0;
    virtual bool isInFunction(int index, subfunc* func) = 0;
    float getSpeed();
    bool setLocked(eFunctype func, int _id, bool _active);
    void calcDirFromLast(int i);
	QVector<mnode> lNodes;
    track* parent;
    func* rollFunc;

    enum secType type;

    bool bSpeed;
    float fVel;

    bool bOrientation;
    bool bArgument;

    // Straight Section Parameters
    float fHLength;

    // Curved Section Parameters
    float fAngle;
    float fRadius;
    float fDirection;
    float fLeadIn;
    float fLeadOut;

    // Forced/Geometric Section Parameters
    int iTime;
    func* normForce;
    func* latForce;
    QString sName;

    // Bezier Section Parameters
    QList<bezier_t*> bezList;
    QList<glm::vec3> supList;
};

#endif // SECTION_H
