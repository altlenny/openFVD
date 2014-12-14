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

#include "track.h"
#include "exportfuncs.h"
#include "optionsmenu.h"
#include "mainwindow.h"
#include "smoothhandler.h"
#include "trackhandler.h"
#include "trackmesh.h"
#include "smoothui.h"
#include "trackwidget.h"

#define RELTHRESH 0.98f

using namespace std;

extern MainWindow* gloParent;
extern glViewWidget* glView;

track::track()
{

}

track::track(trackHandler* _parent, glm::vec3 startPos, float startYaw, float heartLine)
{
    this->anchorNode = new mnode(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0, 0, -1), 0., 10.f, 1., 0.);
    this->startPos = startPos;
    this->startYaw = startYaw;
    this->startPitch = 0.f;
    povPos = glm::vec2(0, 0);
    mParent = _parent;
    anchorNode->updateNorm();
    anchorNode->fEnergy = 0.5f*anchorNode->fVel*anchorNode->fVel + 9.80665f*anchorNode->fPosHearty(0.9*heartLine);
    this->fHeart = heartLine;
    fFriction = 0.03f;
    fResistance = 2e-5;
    hasChanged = true;
    drawTrack = true;
    drawHeartline = 0;
    mOptions = gloParent->mOptions;
    activeSection = NULL;

    smoothList.append(new smoothHandler(this, -1));

    smoothedUntil = 0;
    style = generic;
}

track::~track()
{
    while(lSections.size() != 0)
    {
        delete lSections.at(0);
        lSections.removeAt(0);
    }
    while(smoothList.size() != 0)
    {
        delete smoothList[0];
        smoothList.removeFirst();
    }
    delete anchorNode;
}

void track::removeSection(int index)
{
    if(lSections.size() <= index) return;

    delete smoothList[index+1];
    smoothList.removeAt(index+1);

    if(index == lSections.size()-1)
    {
        delete this->lSections.at(index);
        this->lSections.removeAt(index);
        if(lSections.size() != 0) activeSection = lSections.at(index-1);

        mParent->mMesh->buildMeshes(getNumPoints()-50 < 0 ? 0 : getNumPoints()-50);

        //updateTrack(index-1, lSections[index-1]->lNodes.size()-2);
    }
    else
    {
        lSections.at(index+1)->lNodes.prepend(lSections.at(index)->lNodes.at(0));
        delete this->lSections.at(index);
        this->lSections.removeAt(index);
        activeSection = lSections.at(index);

        updateTrack(index, 0);//lSections[index]->lNodes.size()-2);
    }
}

void track::removeSection(section* fromSection)
{
    int i = 0;
    if(lSections.size() == 0) return;
    for(; i > lSections.size(); ++i)
    {
        if(lSections.at(i) == fromSection) break;
    }
    removeSection(i);
}

void track::removeSmooth(int fromNode)
{
    if(smoothedUntil == fromNode) return;
    if(fromNode < 0) fromNode = 0;
    smoothedUntil = fromNode;
    mnode* prevNode, *curNode = NULL;
    float temp = 0.f;
    for(int i = 0; i < lSections.size(); ++i)
    {
        section* curSection = lSections[i];
        if(fromNode >= curSection->lNodes.size() && curSection->lNodes.size() > 1)
        {
            fromNode -= curSection->lNodes.size()-1;
            continue;
        }
        if(fromNode != 0) curNode = curSection->lNodes[fromNode-1];
        else if(i != 0) curNode = lSections[i-1]->lNodes.last();
        else curNode = this->anchorNode;
        for(int j = fromNode; j < curSection->lNodes.size(); ++j)
        {
            prevNode = curNode;
            curNode = curSection->lNodes[j];
            if(fabs(curNode->fSmoothSpeed) > 0.)
            {
                temp -= curNode->fSmoothSpeed;
                curNode->setRoll(temp/F_HZ);
                curNode->smoothNormal = 0.f;
                curNode->smoothLateral = 0.f;
                curNode->fSmoothSpeed = 0.f;
                curNode->fDistFromLast = glm::distance(curNode->vPosHeart(fHeart), prevNode->vPosHeart(fHeart));
                curNode->fTotalLength = prevNode->fTotalLength + curNode->fDistFromLast;
            }
        }
        fromNode = 1;
    }
}

void track::applySmooth(int fromNode)
{
    if(fromNode < 0) fromNode = 0;
    if(smoothedUntil != fromNode)
    {
        qWarning("Smoothing state unstable!");
        return;
    }
    mnode* prevNode, *curNode = NULL;
    smoothedUntil = getNumPoints();
    float temp = 0.f;
    for(int i = 0; i < lSections.size(); ++i)
    {
        section* curSection = lSections[i];
        if(fromNode >= curSection->lNodes.size() && curSection->lNodes.size() > 1)
        {
            fromNode -= curSection->lNodes.size()-1;
            continue;
        }
        if(fromNode != 0) curNode = curSection->lNodes[fromNode-1];
        else if(i != 0) curNode = lSections[i-1]->lNodes.last();
        else curNode = this->anchorNode;
        for(int j = fromNode; j < curSection->lNodes.size(); ++j)
        {
            prevNode = curNode;
            curNode = curSection->lNodes[j];
            if(fabs(curNode->fSmoothSpeed) > 0.)
            {
                temp += curNode->fSmoothSpeed;
                curNode->setRoll(temp/F_HZ);
                curNode->calcSmoothForces();
                curNode->fDistFromLast = glm::distance(curNode->vPosHeart(fHeart), prevNode->vPosHeart(fHeart));
                curNode->fTotalLength = prevNode->fTotalLength + curNode->fDistFromLast;
            }
        }
        fromNode = 1;
    }
}

void track::updateTrack(int index, int iNode)
{
    //qDebug("called updateTrack(%d, %d)", index, iNode);
    if(index < 0) index = 0;
    if(lSections.size() <= index)
    {
        hasChanged = true;
        return;   // for savety
    }

    QElapsedTimer timer;
    float mSec;
    bool useSmoothing = false;
    timer.start();

    int nodeAt = (lSections[index]->type == straight || lSections[index]->type == curved) ? 0 : iNode;
    for(int i = 0; i < index; ++i)
    {
        nodeAt += lSections[i]->lNodes.size()-1;
    }

    for(int i = 0; i < smoothList.size(); ++i)
    {
        smoothHandler* cur = smoothList[i];
        if(cur->active == false) continue;

        cur->update();
        if(cur->getTo() > nodeAt)
        {
            useSmoothing = true;
            if(cur->getFrom() < nodeAt)
            {
                nodeAt = cur->getFrom();
                i = -1;
            }
        }
    }

    if(useSmoothing)
    {
        removeSmooth(nodeAt);
    }

    int updateFrom = lSections.at(index)->updateSection(iNode);
    for(int i = index+1; i < lSections.size(); i++)
    {
        lSections.at(i)->lNodes.prepend(lSections.at(i-1)->lNodes.at(lSections.at(i-1)->lNodes.size()-1));
        lSections.at(i)->updateSection(0);
    }

    if(useSmoothing && smoother && smoother->active()) smoother->applyRollSmooth(nodeAt);

    unsigned int count = getNumPoints() - nodeAt;
    unsigned int count2 = getNumPoints() - iNode - getNumPoints(lSections[index]);

    nodeAt = nodeAt > getNumPoints(lSections[index])+updateFrom ? getNumPoints(lSections[index])+updateFrom : nodeAt;

    mParent->mMesh->buildMeshes(nodeAt);

    mSec = timer.nsecsElapsed()/1000000.;
    gloParent->showMessage(QString::number(mSec).append(QString("ms used to update %1 (%2) points").arg(count2).arg(count)), 3000);

    hasChanged = true;
}

void track::updateTrack(section* fromSection, int iNode)
{
    int i = 0;
    if(lSections.size() == 0) return;
    for(; i < lSections.size(); ++i)
    {
        if(lSections.at(i) == fromSection) break;
    }
    updateTrack(i, iNode);
}

void track::newSection(enum secType type, int index)
{
    mnode* startNode;
    if(!lSections.isEmpty())
    {
        section* temp;
        if(index == -1)
        {
            temp = lSections.at(lSections.size()-1);
            startNode = temp->lNodes.at(temp->lNodes.size()-1);
        }
        else if(index == 0)
        {
            startNode = anchorNode;
            lSections.at(0)->lNodes.removeFirst();
        }
        else
        {
            temp = lSections.at(index-1);
            startNode = temp->lNodes[temp->lNodes.size()-1];
            if(lSections.size() > index)
            {
                lSections.at(index)->lNodes.removeFirst();
            }
        }
    }
    else
    {
        startNode = anchorNode;
    }

    section* newSection;
    switch(type)
    {
    case 1:
        newSection = new secstraight(this, startNode, 10);
        break;
    case 2:
        newSection = new seccurved(this, startNode, 90, 15);
        break;
    case 3:
        newSection = new secforced(this, startNode, 1000);
        break;
    case 4:
        newSection = new secgeometric(this, startNode, 1000);
        break;
    case 5:
        newSection = new secbezier(this, startNode);
        break;
    default:
        newSection = NULL;
        qWarning("Wrong Section type defined!");
    }
    activeSection = newSection;
    if(index == -1)
    {
        lSections.append(newSection);
        newSection->updateSection();

        smoothList.insert(lSections.size(), new smoothHandler(this, lSections.size()-1));
    }
    else if(index == 0)
    {
        lSections.prepend(newSection);
        newSection->updateSection();
        if(lSections.size() > 1)
        {
            lSections.at(1)->lNodes.prepend(newSection->lNodes[newSection->lNodes.size()-1]);
        }
        smoothList.insert(1, new smoothHandler(this, 0));
    }
    else
    {
        lSections.insert(index, newSection);
        newSection->updateSection();
        if(lSections.size() > index+1)
        {
            lSections.at(index+1)->lNodes.prepend(newSection->lNodes[newSection->lNodes.size()-1]);
        }
        smoothList.insert(index+1, new smoothHandler(this, index));
    }
    hasChanged = true;
}

int track::exportTrack(fstream *file, float mPerNode, int fromIndex, int toIndex, float fRollThresh)
{
    QList<int> exportPoints;
    mnode* anchor = lSections.at(fromIndex)->lNodes[0];
    for(int i = fromIndex; i <= toIndex; ++i)
    {
        lSections.at(i)->iFillPointList(exportPoints, mPerNode);
    }

    mnode *lastP = anchor, *curP = getPoint(exportPoints[0]);
    QList<bezier_t*> bezList;

    for(int i = 0; i < exportPoints.size(); ++i)
    {
        curP = getPoint(exportPoints[i]);

        curP->exportNode(bezList, lastP, NULL, anchor, fHeart, fRollThresh);

        lastP = curP;
    }


    size_t size = (exportPoints.size()-1);
    float *a = (float*)malloc(size*sizeof(float));
    float *b = (float*)malloc(size*sizeof(float));
    float *c = (float*)malloc(size*sizeof(float));
    glm::vec3 *d = (glm::vec3*)malloc(size*sizeof(glm::vec3));

    for(size_t i = 0; i < size; ++i)
    {
        if(i == 0)
        {
            b[i] = 2.f;
            a[i] = 0.f;
            c[i] = 1.f;
            d[i] = bezList[i]->P1 + 2.f * bezList[i+1]->P1;
        }
        else if(i == size-1)
        {
            b[i] = 7.f;
            a[i] = 2.f;
            c[i] = 0.f;
            d[i] = 8.f*bezList[i]->P1 + bezList[i+1]->P1;
        }
        else
        {
            a[i] = 1.f;
            b[i] = 4.f;
            c[i] = 1.f;
            d[i] = 4.f * bezList[i]->P1 + 2.f * bezList[i+1]->P1;
       }
    }

    // solve that shit

    c[0] = c[0]/b[0];
    d[0] = d[0]/b[0];

    for(size_t i = 1; i < size; ++i)
    {
        float m = 1.f/(b[i]-a[i]*c[i-1]);
        c[i] = c[i] * m;
        d[i] = m*(d[i] - a[i]*d[i-1]);
    }

    for(size_t i = size-1; i-- > 0;)
    {
        d[i] = d[i] - c[i] * d[i+1];
        bezList[i+1]->Kp1 = d[i];
        bezList[i+1]->Kp2 = (bezList[i]->P1 - bezList[i]->Kp1)+bezList[i]->P1;
    }

    bezList.last()->Kp1 = 0.5f*(bezList.last()->P1 + bezList[size-2]->Kp2);
    bezList.last()->Kp2 = (bezList.last()->P1 - bezList.last()->Kp1)+bezList.last()->P1;


    writeToExportFile(file, bezList);

    return exportPoints.size();
}

int track::exportTrack2(fstream *file, float mPerNode, int fromIndex, int toIndex, float fRollThresh)
{
    QList<int> exportPoints;
    mnode* anchor = lSections.at(fromIndex)->lNodes[0];
    glm::vec3 anchorPos = anchor->vPosHeart(fHeart);
    for(int i = fromIndex; i <= toIndex; ++i)
    {
        lSections.at(i)->iFillPointList(exportPoints, mPerNode);
    }

    glm::vec3 KP1_this, P, KP2_this;

    KP2_this = anchor->vDirHeart(fHeart)*glm::distance(anchor->vPosHeart(fHeart), getPoint(exportPoints[0])->vPosHeart(fHeart))/3.f;
    qDebug("KP2_this %f %f %f",KP2_this.x,KP2_this.y,KP2_this.z);

    writeBytes(file, (const char*)&(KP2_this.x), 4);
    writeBytes(file, (const char*)&(KP2_this.y), 4);
    writeBytes(file, (const char*)&(KP2_this.z), 4);

    glm::vec3 startPoint = getPoint(exportPoints[0])->vPosHeart(fHeart)+(KP2_this+anchorPos - getPoint(exportPoints[0])->vPosHeart(fHeart))/2.f*3.f;
    qDebug("startPoint %f %f %f",startPoint.x,startPoint.y,startPoint.z);

    P = glm::vec3((1/6.f)*(startPoint+4.f*getPoint(exportPoints[0])->vPosHeart(fHeart)+getPoint(exportPoints[1])->vPosHeart(fHeart)))-anchorPos;
    KP1_this = glm::vec3((1/3.f)*(startPoint+2.f*getPoint(exportPoints[0])->vPosHeart(fHeart)))-anchorPos;
    KP2_this = glm::vec3((1/3.f)*(2.f*getPoint(exportPoints[0])->vPosHeart(fHeart)+getPoint(exportPoints[1])->vPosHeart(fHeart)))-anchorPos;

    writeBytes(file, (const char*)&(KP1_this.x), 4);
    writeBytes(file, (const char*)&(KP1_this.y), 4);
    writeBytes(file, (const char*)&(KP1_this.z), 4);

    writeBytes(file, (const char*)&(P.x), 4);
    writeBytes(file, (const char*)&(P.y), 4);
    writeBytes(file, (const char*)&(P.z), 4);

    glm::vec3 V = glm::normalize(P - KP1_this);

    glm::vec3 vHeartLat = glm::normalize(glm::cross(getPoint(exportPoints[0])->vNorm, V));
    float temp = glm::atan(vHeartLat.y, -getPoint(exportPoints[0])->vNorm.y);
    if(fabs(V.y) > fRollThresh)
    {
        glm::vec3 vLastLat = anchor->vLatHeart(fHeart);

        glm::vec3 rotateAxis = glm::cross(anchor->vDirHeart(fHeart), V);
        glm::vec3 rotated = glm::vec3(glm::rotate(glm::angle(anchor->vDirHeart(fHeart), V), rotateAxis)*glm::vec4(vLastLat, 0.f));
        temp = glm::angle(rotated, vHeartLat)*F_PI/180.f;
        if(glm::dot(glm::cross(rotated, vHeartLat), V) > 0)
        {
            temp *= -1.f;
        }
        if(temp!=temp)
        {
            temp = 0.f;
        }
    }
    writeBytes(file, (const char*)&(temp), 4);

    char cTemp;

    cTemp = 0xFF;
    writeBytes(file, &cTemp, 1); // CONT ROLL
    cTemp = 0x00;
    if(fabs(V.y) > fRollThresh)
    {
        cTemp = 0xFF;
    }
    writeBytes(file, &cTemp, 1); // equalDistanceCP
    cTemp = 0x00;
    writeBytes(file, &cTemp, 1); // REL ROLL
    writeNulls(file, 7); // were 5

    int i = 0;
    for(i = 1; i < exportPoints.size()-2; ++i)
    {
        writeBytes(file, (const char*)&(KP2_this.x), 4);
        writeBytes(file, (const char*)&(KP2_this.y), 4);
        writeBytes(file, (const char*)&(KP2_this.z), 4);

        P = glm::vec3((1/6.f)*(getPoint(exportPoints[i-1])->vPosHeart(fHeart)+4.f*getPoint(exportPoints[i])->vPosHeart(fHeart)+getPoint(exportPoints[i+1])->vPosHeart(fHeart)))-anchorPos;
        KP1_this = glm::vec3((1/3.f)*(getPoint(exportPoints[i-1])->vPosHeart(fHeart)+2.f*getPoint(exportPoints[i])->vPosHeart(fHeart)))-anchorPos;
        KP2_this = glm::vec3((1/3.f)*(2.f*getPoint(exportPoints[i])->vPosHeart(fHeart)+getPoint(exportPoints[i+1])->vPosHeart(fHeart)))-anchorPos;

        writeBytes(file, (const char*)&(KP1_this.x), 4);
        writeBytes(file, (const char*)&(KP1_this.y), 4);
        writeBytes(file, (const char*)&(KP1_this.z), 4);

        writeBytes(file, (const char*)&(P.x), 4);
        writeBytes(file, (const char*)&(P.y), 4);
        writeBytes(file, (const char*)&(P.z), 4);

        glm::vec3 V = glm::normalize(P - KP1_this);

        glm::vec3 vHeartLat = glm::normalize(glm::cross(getPoint(exportPoints[i])->vNorm, V));
        float temp = glm::atan(vHeartLat.y, -getPoint(exportPoints[i])->vNorm.y);
        if(fabs(V.y) > fRollThresh)
        {
            glm::vec3 vLastLat = getPoint(exportPoints[i-1])->vLatHeart(fHeart);

            glm::vec3 rotateAxis = glm::cross(getPoint(exportPoints[i-1])->vDirHeart(fHeart), V);
            glm::vec3 rotated = glm::vec3(glm::rotate(glm::angle(getPoint(exportPoints[i-1])->vDirHeart(fHeart), V), rotateAxis)*glm::vec4(vLastLat, 0.f));
            temp = glm::angle(rotated, vHeartLat)*F_PI/180.f;
            if(glm::dot(glm::cross(rotated, vHeartLat), V) > 0)
            {
                temp *= -1.f;
            }
            if(temp!=temp)
            {
                temp = 0.f;
            }
        }
        writeBytes(file, (const char*)&(temp), 4);

        char cTemp;

        cTemp = 0xFF;
        writeBytes(file, &cTemp, 1); // CONT ROLL
        cTemp = 0x00;
        if(fabs(V.y) > fRollThresh)
        {
            cTemp = 0xFF;
        }
        writeBytes(file, &cTemp, 1); // equalDistanceCP
        cTemp = 0x00;
        writeBytes(file, &cTemp, 1); // REL ROLL
        writeNulls(file, 7); // were 5
    }

    writeBytes(file, (const char*)&(KP2_this.x), 4);
    writeBytes(file, (const char*)&(KP2_this.y), 4);
    writeBytes(file, (const char*)&(KP2_this.z), 4);

    glm::vec3 endPoint = getPoint(exportPoints[exportPoints.size()-1])->vPosHeart(fHeart) - getPoint(exportPoints[exportPoints.size()-1])->vDirHeart(fHeart)*glm::distance(getPoint(exportPoints[exportPoints.size()-1])->vPosHeart(fHeart), getPoint(exportPoints[exportPoints.size()-2])->vPosHeart(fHeart));
    qDebug("endPoint %f %f %f",endPoint.x,endPoint.y,endPoint.z);

    P = glm::vec3((1/6.f)*(getPoint(exportPoints[i-1])->vPosHeart(fHeart)+4.f*endPoint+getPoint(exportPoints[i+1])->vPosHeart(fHeart)))-anchorPos;
    KP1_this = glm::vec3((1/3.f)*(getPoint(exportPoints[i-1])->vPosHeart(fHeart)+2.f*endPoint))-anchorPos;
    KP2_this = glm::vec3((1/3.f)*(2.f*endPoint+getPoint(exportPoints[i+1])->vPosHeart(fHeart)))-anchorPos;

    writeBytes(file, (const char*)&(KP1_this.x), 4);
    writeBytes(file, (const char*)&(KP1_this.y), 4);
    writeBytes(file, (const char*)&(KP1_this.z), 4);

    writeBytes(file, (const char*)&(P.x), 4);
    writeBytes(file, (const char*)&(P.y), 4);
    writeBytes(file, (const char*)&(P.z), 4);

    V = glm::normalize(P - KP1_this);

    vHeartLat = glm::normalize(glm::cross(getPoint(exportPoints[i])->vNorm, V));
    temp = glm::atan(vHeartLat.y, -getPoint(exportPoints[i])->vNorm.y);
    if(fabs(V.y) > fRollThresh)
    {
        glm::vec3 vLastLat = getPoint(exportPoints[i-1])->vLatHeart(fHeart);

        glm::vec3 rotateAxis = glm::cross(getPoint(exportPoints[i-1])->vDirHeart(fHeart), V);
        glm::vec3 rotated = glm::vec3(glm::rotate(glm::angle(getPoint(exportPoints[i-1])->vDirHeart(fHeart), V), rotateAxis)*glm::vec4(vLastLat, 0.f));
        temp = glm::angle(rotated, vHeartLat)*F_PI/180.f;
        if(glm::dot(glm::cross(rotated, vHeartLat), V) > 0)
        {
            temp *= -1.f;
        }
        if(temp!=temp)
        {
            temp = 0.f;
        }
    }
    writeBytes(file, (const char*)&(temp), 4);

    cTemp = 0xFF;
    writeBytes(file, &cTemp, 1); // CONT ROLL
    cTemp = 0x00;
    if(fabs(V.y) > fRollThresh)
    {
        cTemp = 0xFF;
    }
    writeBytes(file, &cTemp, 1); // equalDistanceCP
    cTemp = 0x00;
    writeBytes(file, &cTemp, 1); // REL ROLL
    writeNulls(file, 7); // were 5

    ++i;

    writeBytes(file, (const char*)&(KP2_this.x), 4);
    writeBytes(file, (const char*)&(KP2_this.y), 4);
    writeBytes(file, (const char*)&(KP2_this.z), 4);


    P = getPoint(exportPoints[exportPoints.size()-1])->vPosHeart(fHeart)-anchorPos;
    KP1_this = P-getPoint(exportPoints[exportPoints.size()-1])->vDirHeart(fHeart)*glm::distance(getPoint(exportPoints[exportPoints.size()-1])->vPosHeart(fHeart), getPoint(exportPoints[exportPoints.size()-2])->vPosHeart(fHeart))/3.f;

    writeBytes(file, (const char*)&(KP1_this.x), 4);
    writeBytes(file, (const char*)&(KP1_this.y), 4);
    writeBytes(file, (const char*)&(KP1_this.z), 4);

    writeBytes(file, (const char*)&(P.x), 4);
    writeBytes(file, (const char*)&(P.y), 4);
    writeBytes(file, (const char*)&(P.z), 4);

    V = glm::normalize(P - KP1_this);

    vHeartLat = glm::normalize(glm::cross(getPoint(exportPoints[i])->vNorm, V));
    temp = glm::atan(vHeartLat.y, -getPoint(exportPoints[i])->vNorm.y);
    if(fabs(V.y) > fRollThresh)
    {
        glm::vec3 vLastLat = getPoint(exportPoints[i-1])->vLatHeart(fHeart);

        glm::vec3 rotateAxis = glm::cross(getPoint(exportPoints[i-1])->vDirHeart(fHeart), V);
        glm::vec3 rotated = glm::vec3(glm::rotate(glm::angle(getPoint(exportPoints[i-1])->vDirHeart(fHeart), V), rotateAxis)*glm::vec4(vLastLat, 0.f));
        temp = glm::angle(rotated, vHeartLat)*F_PI/180.f;
        if(glm::dot(glm::cross(rotated, vHeartLat), V) > 0)
        {
            temp *= -1.f;
        }
        if(temp!=temp)
        {
            temp = 0.f;
        }
    }
    writeBytes(file, (const char*)&(temp), 4);

    cTemp = 0xFF;
    writeBytes(file, &cTemp, 1); // CONT ROLL
    cTemp = 0x00;
    if(fabs(V.y) > fRollThresh)
    {
        cTemp = 0xFF;
    }
    writeBytes(file, &cTemp, 1); // equalDistanceCP
    cTemp = 0x00;
    writeBytes(file, &cTemp, 1); // REL ROLL
    writeNulls(file, 7); // were 5


    return exportPoints.size();
}

int track::exportTrack3(fstream *file, float mPerNode, int fromIndex, int toIndex, float fRollThresh)
{
    QList<int> exportPoints;
    mnode* anchor = lSections.at(fromIndex)->lNodes[0];
    for(int i = fromIndex; i <= toIndex; ++i)
    {
        lSections.at(i)->iFillPointList(exportPoints, mPerNode);
    }

    mnode *lastP = anchor, *curP = getPoint(exportPoints[0]);
    QList<bezier_t*> bezList;

    bezList.append(new bezier_t);
    bezList[0]->P1 = glm::vec3(0.f, 0.f, 0.f);

    for(int i = 0; i < exportPoints.size(); ++i)
    {
        curP = getPoint(exportPoints[i]);

        curP->exportNode(bezList, lastP, NULL, anchor, fHeart, fRollThresh);

        lastP = curP;
    }


    size_t size = bezList.size();
    QVector<float> a = QVector<float>(size);
    QVector<float> b = QVector<float>(size);
    QVector<float> c = QVector<float>(size);
    QVector<glm::vec3> d = QVector<glm::vec3>(size);

    for(size_t i = 0; i < size; ++i)
    {
        d[i] = bezList[i]->P1;
        if(i == 0)
        {
            a[i] = 0.f;
            b[i] = 1.f;
            c[i] = 0.f;
        }
        else if(i == size-1)
        {
            a[i] = 0.f;
            b[i] = 1.f;
            c[i] = 0.f;
        }
        else
        {
            a[i] = 1.f/6.f;
            b[i] = 4.f/6.f;
            c[i] = 1.f/6.f;
       }
    }

    // solve that shit

    c[0] = c[0]/b[0];
    d[0] = d[0]/b[0];

    for(size_t i = 1; i < size; ++i)
    {
        float m = 1.f/(b[i]-a[i]*c[i-1]);
        c[i] = c[i] * m;
        d[i] = m*(d[i] - a[i]*d[i-1]);
    }

    for(size_t i = size-1; i-- > 0;)
    {
        d[i] = d[i] - c[i] * d[i+1];
    }

    int i;
    for(i = 1; i < bezList.size(); ++i)
    {
        bezList[i]->Kp1 = 1.f/3.f*(d[i]+2.f*d[i-1]);
        bezList[i]->Kp2 = 1.f/3.f*(2.f*d[i]+d[i-1]);
        bezList[i-1]->P1 = 0.5f*(bezList[i-1]->Kp2 + bezList[i]->Kp1);
    }
    delete bezList[0];

    bezList.removeFirst();

    writeToExportFile(file, bezList);

    for(int i = 0; i < bezList.size(); ++i)
    {
        delete bezList[i];
    }

    return exportPoints.size();
}

int track::exportTrack4(fstream *file, float mPerNode, int fromIndex, int toIndex, float fRollThresh)
{
    QList<int> exportPoints;
    mnode* anchor = lSections.at(fromIndex)->lNodes[0];
    for(int i = fromIndex; i <= toIndex; ++i)
    {
        lSections.at(i)->iFillPointList(exportPoints, mPerNode);
    }

    mnode *last = anchor, *current = getPoint(exportPoints[0]), *mid = getPoint(exportPoints[0]/2);
    QList<bezier_t*> bezList;

    for(int i = 0; i < exportPoints.size(); ++i)
    {
        current = getPoint(exportPoints[i]);

        mid = NULL; //i == 0 ? getPoint(exportPoints[0]/2) : getPoint((exportPoints[i]+exportPoints[i-1])/2);

        current->exportNode(bezList, last, mid, anchor, fHeart, fRollThresh);

        last = current;
    }

    writeToExportFile(file, bezList);

    return exportPoints.size();
}

void track::exportNL2Track(FILE *file, float mPerNode, int fromIndex, int toIndex)
{
    QList<int> exportPoints, rollPoints;
    mnode* anchor = lSections.at(fromIndex)->lNodes[0];
    exportPoints.append(getNumPoints(lSections.at(fromIndex)));
    rollPoints.append(getNumPoints(lSections.at(fromIndex)));
    for(int i = fromIndex; i <= toIndex; ++i)
    {
        lSections.at(i)->fFillPointList(exportPoints, mPerNode);
    }

    for(int i = 0; i < exportPoints.size(); ++i) {
        qDebug("%d\n", exportPoints[i]);
    }

    size_t size = exportPoints.size();
    QVector<float> a = QVector<float>(size);
    QVector<float> b = QVector<float>(size);
    QVector<float> c = QVector<float>(size);
    QVector<glm::vec3> d = QVector<glm::vec3>(size);

    for(size_t i = 0; i < size; ++i)
    {
        int point = exportPoints[i];
        mnode* curNode = getPoint(point < 0 ? -point : point);
        d[i] = curNode->vPos - anchor->vPos;
        if(i == 0 || i == size-1 || point < 0) {
            a[i] = 0.f;
            b[i] = 1.f;
            c[i] = 0.f;
        } else {
            a[i] = 1.f/6.f;
            b[i] = 4.f/6.f;
            c[i] = 1.f/6.f;
       }
    }

    // tridiagonal solver

    c[0] = c[0]/b[0];
    d[0] = d[0]/b[0];

    for(size_t i = 1; i < size; ++i) {
        float m = 1.f/(b[i]-a[i]*c[i-1]);
        c[i] = c[i] * m;
        d[i] = m*(d[i] - a[i]*d[i-1]);
    }

    for(size_t i = size-1; i-- > 0;) {
        d[i] = d[i] - c[i] * d[i+1];
    }

    // resolve strictness
    QList<glm::vec4> e;
    e.append(glm::vec4(d[0], 1.f));
    for(size_t i = 1; i < size-1; ++i) {
        int point = exportPoints[i];
        int ppoint = exportPoints[i-1];
        int npoint = exportPoints[i+1];
        int strict = 0;
        if(ppoint <= 0) {
            strict |= 1;
            ppoint *= -1;
        }
        if(point < 0) {
            strict |= 2;
            point *= -1;
        }
        if(npoint <= 0) {
            strict |= 4;
            npoint *= -1;
        }

        // if strict == 0 -> export normally
        // if strict == 1 -> straight into open section -> resolve point
        // if strict == 2 -> can't happen
        // if strict == 3 -> export normally
        // if strict == 4 -> open section into straight -> resolve point
        // if strict == 5 -> straight into straight -> resolve point into two points
        // if strict == 6 -> export normally
        // if strict == 7 -> can't happen

        if(strict == 1) {
            glm::vec3 dir = getPoint(ppoint)->vDir;
            glm::vec3 nP = getPoint(npoint)->vPos;
            float a = glm::length(nP-d[i-1]);
            float cosa = glm::dot(glm::normalize(nP-d[i-1]), dir);
            e.append(glm::vec4(d[i-1]+dir*a/(2.f*cosa), 0.f));
        } else if (strict == 2) {
            qDebug("bad export status");
        } else if (strict == 3) {
            e.append(glm::vec4(d[i], 1.f));
        } else if (strict == 4) {
            glm::vec3 dir = getPoint(npoint)->vDir;
            glm::vec3 pP = getPoint(ppoint)->vPos;
            float a = glm::length(d[i+1]-pP);
            float cosa = glm::dot(glm::normalize(d[i+1]-pP), dir);
            e.append(glm::vec4(d[i+1]-dir*a/(2.f*cosa), 0.f));
        } else if (strict == 5) {
            glm::vec3 dp = getPoint(npoint)->vPos - getPoint(ppoint)->vPos;
            glm::vec3 dv = getPoint(npoint)->vDir + getPoint(ppoint)->vDir;

            float a = glm::length2(dv)-1.f;
            float b = glm::dot(dv, dp)*2.f;
            float c = glm::length2(dp);

            b /= a;
            c /= a;

            float p = b/2.f;
            float x0 = -p + sqrt(p*p - c);
            //float x1 = -p - sqrt(p*p - c); // second solution (unsused)

            e.append(glm::vec4(getPoint(ppoint)->vPos-x0*getPoint(ppoint)->vDir, 0.f));
            e.append(glm::vec4(getPoint(npoint)->vPos+x0*getPoint(npoint)->vDir, 0.f));

            //qDebug("%f, %f", x0, x1);

        } else if (strict == 6) {
            e.append(glm::vec4(d[i], 1.f));
        } else if (strict == 7) {
            e.append(glm::vec4(d[i], 1.f));
        } else {
            e.append(glm::vec4(d[i], 0.f));
        }
    }
    e.append(glm::vec4(d[size-1], 1.f));

    float temp = glm::length(glm::vec3(anchor->vDir.x, 0.f, anchor->vDir.z));
    glm::mat3 anchorBase = glm::transpose(glm::mat3(-anchor->vDir.z/temp, 0.f, anchor->vDir.x/temp,
                     0.f, 1.f, 0.f,
                     -anchor->vDir.x/temp, 0.f, -anchor->vDir.z/temp));

    for(int i = 0; i < e.size(); ++i) {
        glm::vec3 ex = anchorBase*glm::vec3(e[i]);
        fprintf(file, "\t\t\t<vertex>\n");
        fprintf(file, "\t\t\t\t<x>%e</x>\n", ex.x);
        fprintf(file, "\t\t\t\t<y>%e</y>\n", ex.y);
        fprintf(file, "\t\t\t\t<z>%e</z>\n", ex.z);
        if(e[i].w > 0.5f) {
            fprintf(file, "\t\t\t\t<strict>true</strict>\n");
        }
        fprintf(file, "\t\t\t</vertex>\n");
    }

    float startLen = getPoint(exportPoints[0])->fTotalHeartLength;
    int lp = abs(exportPoints.last());
    float endLen = getPoint(lp)->fTotalHeartLength;

    for(size_t i = 0; i < size; ++i) {
        int point = exportPoints[i];
        mnode* curNode = getPoint(point < 0 ? -point : point);

        glm::vec3 up = anchorBase*(-curNode->vNorm);
        glm::vec3 right = anchorBase*(curNode->vLat);
        float coord = (curNode->fTotalHeartLength-startLen)/(endLen-startLen);

        fprintf(file, "\t\t\t<roll>\n");
        fprintf(file, "\t\t\t\t<ux>%e</ux>\n", up.x);
        fprintf(file, "\t\t\t\t<uy>%e</uy>\n", up.y);
        fprintf(file, "\t\t\t\t<uz>%e</uz>\n", up.z);
        fprintf(file, "\t\t\t\t<rx>%e</rx>\n", right.x);
        fprintf(file, "\t\t\t\t<ry>%e</ry>\n", right.y);
        fprintf(file, "\t\t\t\t<rz>%e</rz>\n", right.z);
        fprintf(file, "\t\t\t\t<coord>%e</coord>\n", coord);
        fprintf(file, "\t\t\t\t<strict>false</strict>\n");
        fprintf(file, "\t\t\t</roll>\n");
    }

    return;
}

QString track::saveTrack(fstream& file, trackWidget* _widget)
{   
    file << "TRC";

    int namelength = name.length();
    std::string stdName = name.toStdString();

    writeBytes(&file, (const char*)&namelength, sizeof(int));
    file << stdName;

    writeBytes(&file, (const char*)&(_widget->inTrack->trackColors), 3*sizeof(QColor));

    // ANCHOR
    writeBytes(&file, (const char*)&startPos, sizeof(glm::vec3));
    writeBytes(&file, (const char*)&anchorNode->fRoll, sizeof(float));
    writeBytes(&file, (const char*)&startPitch, sizeof(float));
    writeBytes(&file, (const char*)&startYaw, sizeof(float));

    writeBytes(&file, (const char*)&anchorNode->fVel, sizeof(float));

    writeBytes(&file, (const char*)&anchorNode->forceNormal, sizeof(float));
    writeBytes(&file, (const char*)&anchorNode->forceLateral, sizeof(float));

    writeBytes(&file, (const char*)&fHeart, sizeof(float));
    writeBytes(&file, (const char*)&fFriction, sizeof(float));
    writeBytes(&file, (const char*)&fResistance, sizeof(float));

    writeBytes(&file, (const char*)&drawTrack, sizeof(bool));
    writeBytes(&file, (const char*)&drawHeartline, sizeof(int));
    writeBytes(&file, (const char*)&style, sizeof(int));
    writeBytes(&file, (const char*)&mParent->mMesh->isWireframe, sizeof(bool));

    writeBytes(&file, (const char*)&povPos.x, sizeof(float));
    writeBytes(&file, (const char*)&povPos.y, sizeof(float));

    //float fFriction;
    int size = lSections.size();
    writeBytes(&file, (const char*)&size, sizeof(int));


    for(int i = 0; i < lSections.size(); ++i)
    {
        lSections.at(i)->saveSection(file);
    }

    size = smoothList.size();
    writeBytes(&file, (const char*)&size, sizeof(int));
    for(int i = 0; i < size; ++i)
    {
        smoothList[i]->saveSmooth(file);
    }

    file << "EOT";

    return QString("Save Successful");
}

QString track::loadTrack(fstream& file, trackWidget* _widget)
{
    int namelength = readInt(&file);
    name = QString(readString(&file, namelength).c_str());

    readBytes(&file, &_widget->inTrack->trackColors, 3*sizeof(QColor));

    startPos = readVec3(&file);
    anchorNode->fRoll = readFloat(&file);
    startPitch = readFloat(&file);
    startYaw = readFloat(&file);
    anchorNode->fVel = readFloat(&file);
    anchorNode->forceNormal = readFloat(&file);
    anchorNode->forceLateral = readFloat(&file);

    fHeart = readFloat(&file);
    fFriction = readFloat(&file);
    fResistance = readFloat(&file);

    drawTrack = readBool(&file);
    drawHeartline = readInt(&file);
    style = (enum trackStyle)readInt(&file);
    mParent->mMesh->isWireframe = readBool(&file);

    povPos.x = readFloat(&file);
    povPos.y = readFloat(&file);

    anchorNode->fEnergy = 0.5f*anchorNode->fVel*anchorNode->fVel + 9.80665f*anchorNode->fPosHearty(0.9*fHeart);

    anchorNode->changePitch(startPitch, false);
    anchorNode->setRoll(anchorNode->fRoll);

    _widget->updateAnchorGeometrics();

    anchorNode->updateNorm();

    string temp;
    //gloParent->treeInit(this);
    int size = readInt(&file);
    for(int i = 0; i < size; ++i)
    {
        temp = readString(&file, 3);
        if(temp == "STR")
        {
            _widget->addStraightSec();
            //this->newSection(straight);
            activeSection->loadSection(file);
            activeSection->updateSection();
            //gloParent->addStraightSec(activeSection);
        }
        else if(temp == "CUR")
        {
            _widget->addCurvedSec();
            //this->newSection(curved);
            activeSection->loadSection(file);
            activeSection->updateSection();
            //gloParent->addCurvedSec(activeSection);
        }
        else if(temp == "GEO")
        {
            _widget->addGeometricSec();
            //this->newSection(geometric);
            activeSection->loadSection(file);
            activeSection->updateSection();
            //gloParent->addGeometricSec(activeSection);
        }
        else if(temp == "FRC")
        {
            _widget->addForceSec();
            //this->newSection(forced);
            activeSection->loadSection(file);
            activeSection->updateSection();
            //gloParent->addForceSec(activeSection);
        }
        else if(temp == "BEZ")
        {
            _widget->addSection(bezier);
            //this->newSection(forced);
            activeSection->loadSection(file);
            activeSection->updateSection();
            //gloParent->addForceSec(activeSection);
        }
        else
        {
            return QString("Error while Loading: No Such Segment!");
        }
    }

    size = readInt(&file);
    for(int i = 0; i < size; ++i)
    {
        if(i >= smoothList.size()) smoothList.append(new smoothHandler(this, -2));

        smoothList[i]->loadSmooth(file);
    }

    temp = readString(&file, 3);
    if(temp == "EOT")
    {
        updateTrack(0, 0);
        _widget->clearSelection();
        _widget->setNames();
        return QString("Load Successful");
    }
    else
    {
        return QString("Load not Successful");
    }
}

QString track::legacyLoadTrack(fstream& file, trackWidget* _widget)
{
    int namelength = readInt(&file);
    name = QString(readString(&file, namelength).c_str());

    readBytes(&file, &_widget->inTrack->trackColors, 3*sizeof(QColor));

    startPos = readVec3(&file);
    anchorNode->fRoll = readFloat(&file);
    startPitch = readFloat(&file);
    startYaw = readFloat(&file);
    anchorNode->fVel = readFloat(&file);
    anchorNode->forceNormal = readFloat(&file);
    anchorNode->forceLateral = readFloat(&file);

    fHeart = readFloat(&file);
    fFriction = readFloat(&file);
    fResistance = readFloat(&file);

    drawTrack = readBool(&file);
    drawHeartline = readInt(&file);
    style = (enum trackStyle)readInt(&file);
    mParent->mMesh->isWireframe = readBool(&file);

    povPos.x = readFloat(&file);
    povPos.y = readFloat(&file);

    anchorNode->fEnergy = 0.5f*anchorNode->fVel*anchorNode->fVel + 9.80665f*anchorNode->fPosHearty(0.9*fHeart);

    anchorNode->changePitch(startPitch, false);
    anchorNode->setRoll(anchorNode->fRoll);

    _widget->updateAnchorGeometrics();

    anchorNode->updateNorm();

    string temp;
    //gloParent->treeInit(this);
    int size = readInt(&file);
    for(int i = 0; i < size; ++i)
    {
        temp = readString(&file, 3);
        if(temp == "STR")
        {
            _widget->addStraightSec();
            //this->newSection(straight);
            activeSection->legacyLoadSection(file);
            activeSection->updateSection();
            //gloParent->addStraightSec(activeSection);
        }
        else if(temp == "CUR")
        {
            _widget->addCurvedSec();
            //this->newSection(curved);
            activeSection->legacyLoadSection(file);
            activeSection->updateSection();
            //gloParent->addCurvedSec(activeSection);
        }
        else if(temp == "GEO")
        {
            _widget->addGeometricSec();
            //this->newSection(geometric);
            activeSection->legacyLoadSection(file);
            activeSection->updateSection();
            //gloParent->addGeometricSec(activeSection);
        }
        else if(temp == "FRC")
        {
            _widget->addForceSec();
            //this->newSection(forced);
            activeSection->legacyLoadSection(file);
            activeSection->updateSection();
            //gloParent->addForceSec(activeSection);
        }
        else if(temp == "BEZ")
        {
            _widget->addSection(bezier);
            //this->newSection(forced);
            activeSection->legacyLoadSection(file);
            activeSection->updateSection();
            //gloParent->addForceSec(activeSection);
        }
        else
        {
            return QString("Error while Loading: No Such Segment!");
        }
    }

    size = readInt(&file);
    for(int i = 0; i < size; ++i)
    {
        if(i >= smoothList.size()) smoothList.append(new smoothHandler(this, -2));

        smoothList[i]->legacyLoadSmooth(file);
    }

    temp = readString(&file, 3);
    if(temp == "EOT")
    {
        updateTrack(0, 0);
        _widget->clearSelection();
        _widget->setNames();
        return QString("Load Successful");
    }
    else
    {
        return QString("Load not Successful");
    }
}

mnode* track::getPoint(int index)
{
    int i = 0;
    if(index < 0) index = 0;
    while(lSections.size() > i && index > lSections.at(i)->lNodes.size()-1)
    {
        index -= lSections.at(i++)->lNodes.size()-1;
    }
    if(lSections.size() == i)
    {
        if(lSections.size())    return lSections.last()->lNodes.last();
        else return anchorNode;
    }
    return lSections.at(i)->lNodes[index];
}

int  track::getIndexFromDist(float dist)
{
    int lower = 0;
    int upper = getNumPoints();
    mnode* point = getPoint(upper);
    float cur = point->fTotalLength;
    if(dist > cur)
    {
        return upper;
    }
    else if(dist < 0.f)
    {
        return 0;
    }
    else
    {
        while(lower+2 <= upper-2)
        {
            point = getPoint((upper+lower)/2);
            cur = point->fTotalLength;
            if(cur < dist)
            {
                lower = (upper+lower)/2;
            }
            else
            {
                upper = (upper+lower)/2;
            }
        }
        return (upper+lower)/2;
    }
}

int track::getNumPoints(section* until)
{

    int sum = 0;
    for(int i = 0; i < lSections.size(); ++i)
    {
        if(lSections.at(i) == until) return sum;
        sum += lSections.at(i)->lNodes.size()-1;
    }
    return sum;
}

int track::getSectionNumber(section *_section)
{
    int number = 0;
    while(number < lSections.size() && lSections.at(number) != _section) ++number;
    if(number < lSections.size())
    {
        return number;
    }
    else
    {
        return -1;
    }
}

void track::getSecNode(int index, int *node, int *section)
{
    int i = 0;
    while(lSections.size() > i && index > lSections.at(i)->lNodes.size()-1)
    {
        index -= lSections.at(i++)->lNodes.size()-1;
    }
    if(lSections.size() == i)
    {
        if(lSections.size())
        {
            *node = lSections.last()->lNodes.size()-1;
            *section = lSections.size()-1;
        }
        else
        {
            *node = 0;
            *section = -1;
        }
        return;
    }
    *node = index;
    *section = i;
    return;
}
