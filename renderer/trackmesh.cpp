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

#include "trackmesh.h"
#include "mainwindow.h"
#include "optionsmenu.h"
#include "mnode.h"

extern MainWindow* gloParent;
extern glViewWidget* glView;

trackMesh::trackMesh(track* parent)
{
    if(!glView->legacyMode)
    {
        glGenVertexArrays(5, TrackObject);
        glGenBuffers(7, TrackBuffer);
        glGenBuffers(5, TrackIndices);
        glGenVertexArrays(5, HeartObject);
        glGenBuffers(5, HeartBuffer);
        glGenBuffers(5, HeartIndices);
        glGenVertexArrays(1, ShadowObject);
        glGenBuffers(1, ShadowBuffer);
    }

    trackVertexSize = 0;
    numRails = 0;
    supportsSize = 0;
    heartlineSize = 0;
    railShadowSize = 0;
    trackData = parent;
    isWireframe = true;
}

trackMesh::~trackMesh()
{
    if(!glView->legacyMode)
    {
        glDeleteVertexArrays(5, TrackObject);
        glDeleteVertexArrays(5, HeartObject);
        glDeleteBuffers(7, TrackBuffer);
        glDeleteBuffers(5, HeartBuffer);
        glDeleteBuffers(5, TrackIndices);
        glDeleteBuffers(5, HeartIndices);
    }
}

/*enum trackStyle {
    generic = 0,    // 0,5m
    genericflat,    // 0,7m
    vekoma,         // 0,6m
    bm,             // 0,6m
    triangle,       // 0,5m
    box,            // 0,5m
    small           // 0,5m
};*/

#define APPEND_TRACK_NODE(name) {name.append(nextPos.x); name.append(nextPos.y); name.append(nextPos.z); name.append(nextNorm.x); name.append(nextNorm.y); name.append(nextNorm.z); name.append(curNode->fVel); name.append(fabs(curNode->fRollSpeed+curNode->fSmoothSpeed)); name.append(curNode->forceNormal+curNode->smoothNormal); name.append(fabs(curNode->forceLateral + curNode->smoothLateral)); name.append(fabs(curNode->fFlexion())); if(trackData == gloParent->curTrack() && curSection == trackData->activeSection) { if(curSection->isInFunction(j, gloParent->selectedFunc)) { name.append(2.0f); } else { name.append(1.0f); } } else { name.append(0.0f); } }
#define APPEND_NODE(name) {name.append(nextPos.x); name.append(nextPos.y); name.append(nextPos.z);}

void trackMesh::appendTrackNode(QVector<tracknode_t> &list, float _u, float _v)
{
    tracknode_t temp;
    temp.node = nextNode;
    temp.pos = nextPos;
    temp.normal = nextNorm;
    temp.uv = glm::vec2(_u, _v);
    temp.vel = curNode->fVel;
    temp.rollSpeed = fabs(curNode->fRollSpeed+curNode->fSmoothSpeed);
    temp.yForce = curNode->forceNormal+curNode->smoothNormal;
    temp.xForce = fabs(curNode->forceLateral + curNode->smoothLateral);
    temp.flexion = fabs(curNode->fFlexion());

    temp.selected = (trackData == gloParent->curTrack() && curSection == trackData->activeSection) ? (curSection->isInFunction(j, gloParent->selectedFunc) ? 2.f : 1.f) : 0.f;

    list.append(temp);
}

void trackMesh::append3dsNode(QVector<float> *_vertices)
{
    _vertices->append(nextPos.x);
    _vertices->append(nextPos.y);
    _vertices->append(nextPos.z);
}

void trackMesh::appendSupportNode(QVector<tracknode_t> &list, float _u, float _v)
{
    tracknode_t temp;
    temp.node = nextNode;
    temp.pos = nextPos;
    temp.normal = nextNorm;
    temp.uv = glm::vec2(_u, _v);
    temp.vel =0;
    temp.rollSpeed = 0;
    temp.yForce = 0;
    temp.xForce = 0;
    temp.flexion = 0;
    temp.selected = 0;

    list.append(temp);
}

void trackMesh::appendMeshNode(QVector<meshnode_t> &list)
{
    meshnode_t temp;
    temp.pos = nextPos;
    temp.node = nextNode;

    list.append(temp);
}

int trackMesh::createPipes(QVector<tracknode_t> &list, QList<pipeoption_t> &options)
{
    int count = 0;
    float angle;
    int numPipes = options.size();

    if(!secList.isEmpty() && rails.isEmpty())
    {
        for(int p = 0; p < numPipes; ++p)
        {
            j = 0;
            curSection = trackData->lSections[0];
            curNode = curSection->lNodes[0];
            nextNorm = -curNode->vDirHeart(-options[p].offset.y);
            nextPos = curNode->vRelPos(options[p].offset.y, options[p].offset.x);
            nextNode = 0;
            appendTrackNode(list, 0, curNode->fTotalLength);
        }
    }

    for(int pos = 0; pos < posList.size(); ++pos)
    {
        for(int p = 0; p < numPipes; ++p)
        {
            float r = 0.5f*(options[p].radius.y+options[p].radius.x);
            for(int i = 0; i < options[p].edges; ++i)
            {
                if(options[p].smooth) angle = i*360.f/options[p].edges - 180.f/options[p].edges;
                else angle = (i/2)*720.f/options[p].edges - 360.f/options[p].edges;

                j = posList[pos];
                curSection = trackData->lSections[secList[pos]];
                curNode = curSection->lNodes[j];

                nextNorm = -(float)(options[p].radius.y*cos(angle*F_PI/180))*curNode->vNorm+(float)(options[p].radius.x*sin(angle*F_PI/180))*curNode->vLatHeart(-options[p].offset.y);
                nextPos = curNode->vRelPos(options[p].offset.y, options[p].offset.x)+nextNorm;
                nextNode = trackData->getNumPoints(curSection) + j;
                if(options[p].smooth)
                {
                    nextNorm = glm::normalize(nextNorm);
                    appendTrackNode(list, r/0.3f*fabs(angle-180+180.f/options[p].edges)/180.f, curNode->fTotalLength);
                }
                else
                {
                    nextNorm = -(float)(options[p].radius.y*cos(angle*F_PI/180+((i%2)*2-1)*F_PI_4))*curNode->vNorm+(float)(options[p].radius.x*sin(angle*F_PI/180+((i%2)*2-1)*F_PI_4))*curNode->vLatHeart(-options[p].offset.y);
                    nextNorm = glm::normalize(nextNorm);
                    appendTrackNode(list, r/0.3f*fabs(angle-180+180.f/options[p].edges)/180.f, curNode->fTotalLength);
                }
            }
        }
    }

    //last node here
    if(!posList.isEmpty())
    {
        for(int p = 0; p < numPipes; ++p)
        {
            j = posList.last();
            curSection = trackData->lSections[secList.last()];
            curNode = curSection->lNodes[j];

            nextNorm = curNode->vDirHeart(-options[p].offset.y);
            nextPos = curNode->vRelPos(options[p].offset.y, options[p].offset.x);
            nextNode = trackData->getNumPoints(curSection) + j;
            appendTrackNode(list, 0, curNode->fTotalLength);
        }
    }

    // get shadow vertices

    glm::vec3 P1, P2, P3, P4;
    for(int i =  0; i < posList.size(); ++i)
    {
        for(int p = 0; p < numPipes; ++p)
        {
            j = posList[i];
            curSection = trackData->lSections[secList[i]];
            curNode = curSection->lNodes[j];
            nextNode = trackData->getNumPoints(curSection) + j;

            float banking = glm::atan(curNode->vLatHeart(-options[p].offset.y).y, -curNode->vNorm.y)+F_PI_2+0.001;
            if(!options[p].smooth)
            {
                banking /= F_PI_2;
                banking = floor(banking);
                banking *= F_PI_2;
                banking += F_PI_4;
            }
            nextNorm = -(float)(options[p].radius.y*cos(banking))*curNode->vNorm+(float)(options[p].radius.x*sin(banking))*curNode->vLatHeart(-options[p].offset.y);
            P1 = curNode->vRelPos(options[p].offset.y, options[p].offset.x)+nextNorm;
            P2 = glm::vec3(P1.x, -trackData->startPos.y-1.f, P1.z);
            banking = glm::atan(curNode->vLatHeart(-options[p].offset.y).y, -curNode->vNorm.y)-F_PI_2-0.001;
            if(!options[p].smooth)
            {
                banking /= F_PI_2;
                banking = floor(banking);
                banking *= F_PI_2;
                banking += F_PI_4;
            }
            nextNorm = -(float)(options[p].radius.y*cos(banking))*curNode->vNorm+(float)(options[p].radius.x*sin(banking))*curNode->vLatHeart(-options[p].offset.y);
            P4 = curNode->vRelPos(options[p].offset.y, options[p].offset.x)+nextNorm;
            P3 = glm::vec3(P4.x, -trackData->startPos.y-1.f, P4.z);
            /*if(j == 0 && secList[i] == 0)
            {
                createQuad(railshadows, P1, P2, P3, P4);
                railShadowSize+=2;
            }
            else if(i == posList.size()-1)
            {
                createBox(railshadows, P1, P2, P4, P3, P5, P6, P8, P7);
                railShadowSize+=8;
                createQuad(railshadows, P2, P1, P4, P3);
                railShadowSize+=2;
            }
            else
            {
                createBox(railshadows, P1, P2, P4, P3, P5, P6, P8, P7);
                railShadowSize+=8;
            }*/
            nextPos = P1;
            appendMeshNode(railshadows);
            nextPos = P2;
            appendMeshNode(railshadows);
            nextPos = P3;
            appendMeshNode(railshadows);
            nextPos = P4;
            appendMeshNode(railshadows);
        }
    }

    return count;
}

int trackMesh::create3dsPipes(QVector<float> *_vertices, QList<pipeoption_t> &options)
{
    int count = 0;
    float angle;
    int numPipes = options.size();

    for(int p = 0; p < numPipes; ++p)
    {
        if(!secList.isEmpty())
        {
            j = posList[0];
            curSection = trackData->lSections[secList[0]];
            curNode = curSection->lNodes[j];
            nextNorm = -curNode->vDirHeart(-options[p].offset.y);
            nextPos = curNode->vRelPos(options[p].offset.y, options[p].offset.x);
            nextNode = trackData->getNumPoints(curSection) + j;
            append3dsNode(_vertices);
        }

        for(int pos = 0; pos < posList.size(); ++pos)
        {
            //float r = 0.5f*(options[p].radius.y+options[p].radius.x);
            for(int i = 0; i < options[p].edges; ++i)
            {
                if(options[p].smooth) angle = i*360.f/options[p].edges - 180.f/options[p].edges;
                else angle = (i/2)*720.f/options[p].edges - 360.f/options[p].edges;

                j = posList[pos];
                curSection = trackData->lSections[secList[pos]];
                curNode = curSection->lNodes[j];

                nextNorm = -(float)(options[p].radius.y*cos(angle*F_PI/180))*curNode->vNorm+(float)(options[p].radius.x*sin(angle*F_PI/180))*curNode->vLatHeart(-options[p].offset.y);
                nextPos = curNode->vRelPos(options[p].offset.y, options[p].offset.x)+nextNorm;
                nextNode = trackData->getNumPoints(curSection) + j;
                if(options[p].smooth)
                {
                    nextNorm = glm::normalize(nextNorm);
                    append3dsNode(_vertices);
                }
                else
                {
                    nextNorm = -(float)(options[p].radius.y*cos(angle*F_PI/180+((i%2)*2-1)*F_PI_4))*curNode->vNorm+(float)(options[p].radius.x*sin(angle*F_PI/180+((i%2)*2-1)*F_PI_4))*curNode->vLatHeart(-options[p].offset.y);
                    nextNorm = glm::normalize(nextNorm);
                    append3dsNode(_vertices);
                }
            }
        }

        //last node here
        if(!posList.isEmpty())
        {
            j = posList.last();
            curSection = trackData->lSections[secList.last()];
            curNode = curSection->lNodes[j];

            nextNorm = curNode->vDirHeart(-options[p].offset.y);
            nextPos = curNode->vRelPos(options[p].offset.y, options[p].offset.x);
            nextNode = trackData->getNumPoints(curSection) + j;
            append3dsNode(_vertices);
        }
    }

    return count;
}

int trackMesh::createPipe(QVector<tracknode_t> &list, int edges, float radiusy, float radiusx, float y, float x, bool smooth)
{
    int count = 0;
    const float r = 0.5f*(radiusy+radiusx);

    float angleLeft, angleRight;
    for(int iteration = 0; iteration < edges; iteration++)
    {
        angleLeft = iteration*360.f/edges - 180.f/edges;
        angleRight = iteration*360.f/edges + 180.f/edges;

        for(int i = (iteration%2==0) ? 0 : posList.size()-1; (iteration%2==0) ? i < posList.size() : i >= 0; (iteration%2==0) ? ++i : --i)
        {
            j = posList[i];
            curSection = trackData->lSections[secList[i]];

            curNode = curSection->lNodes[j];

            if((i == 0 && iteration%2==0) || (i == posList.size()-1 && iteration%2!=0))
            {
                nextPos = curNode->vRelPos(y, x);
                nextNorm = ((iteration%2==0) ? -1.f : 1.f)*curNode->vDirHeart(-y);
                appendTrackNode(list, 0, curNode->fTotalLength);
                ++count;
            }

            if(iteration%2 == 0)
            {
                nextNorm = -(float)(radiusy*cos(angleLeft*F_PI/180))*curNode->vNorm+(float)(radiusx*sin(angleLeft*F_PI/180))*curNode->vLatHeart(-y);
                nextPos = curNode->vRelPos(y, x)+nextNorm;
                if(smooth)
                {
                    nextNorm = glm::normalize(nextNorm);
                }
                else
                {
                    nextNorm = glm::normalize(-(float)(radiusy*cos((angleLeft+angleRight)*F_PI/360))*curNode->vNorm+(float)(radiusx*sin((angleLeft+angleRight)*F_PI/360))*curNode->vLatHeart(-y));
                }
                appendTrackNode(list, r/0.3f*angleLeft/180.f, curNode->fTotalLength);
                ++count;

                nextNorm = -(float)(radiusy*cos(angleRight*F_PI/180))*curNode->vNorm+(float)(radiusx*sin(angleRight*F_PI/180))*curNode->vLatHeart(-y);
                nextPos = curNode->vRelPos(y, x)+nextNorm;
                if(smooth)
                {
                    nextNorm = glm::normalize(nextNorm);
                }
                else
                {
                    nextNorm = glm::normalize(-(float)(radiusy*cos((angleLeft+angleRight)*F_PI/360))*curNode->vNorm+(float)(radiusx*sin((angleLeft+angleRight)*F_PI/360))*curNode->vLatHeart(-y));
                }
                appendTrackNode(list, r/0.3f*angleRight/180.f, curNode->fTotalLength);
                ++count;
            }
            else
            {
                nextNorm = -(float)(radiusy*cos(angleRight*F_PI/180))*curNode->vNorm+(float)(radiusx*sin(angleRight*F_PI/180))*curNode->vLatHeart(-y);
                nextPos = curNode->vRelPos(y, x)+nextNorm;
                if(smooth)
                {
                    nextNorm = glm::normalize(nextNorm);
                }
                else
                {
                    nextNorm = glm::normalize(-(float)(radiusy*cos((angleLeft+angleRight)*F_PI/360))*curNode->vNorm+(float)(radiusx*sin((angleLeft+angleRight)*F_PI/360))*curNode->vLatHeart(-y));
                }
                appendTrackNode(list, r/0.3f*angleRight/180.f, curNode->fTotalLength);
                ++count;


                nextNorm = -(float)(radiusy*cos(angleLeft*F_PI/180))*curNode->vNorm+(float)(radiusx*sin(angleLeft*F_PI/180))*curNode->vLatHeart(-y);
                nextPos = curNode->vRelPos(y, x)+nextNorm;
                if(smooth)
                {
                    nextNorm = glm::normalize(nextNorm);
                }
                else
                {
                    nextNorm = glm::normalize(-(float)(radiusy*cos((angleLeft+angleRight)*F_PI/360))*curNode->vNorm+(float)(radiusx*sin((angleLeft+angleRight)*F_PI/360))*curNode->vLatHeart(-y));
                }
                appendTrackNode(list, r/0.3f*angleLeft/180.f, curNode->fTotalLength);
                ++count;
            }

            if((i == 0 && iteration == edges-1))
            {
                nextPos = curNode->vRelPos(y, x);
                nextNorm = ((iteration%2==0) ? 1.f : -1.f)*curNode->vDirHeart(-y);
                appendTrackNode(list, r/0.3f*angleLeft/180.f, curNode->fTotalLength);
                ++count;
            }

        }
    }


    glm::vec3 P1, P2, P3, P4, P5, P6, P7, P8;
    for(int i =  0; i < posList.size(); ++i)
    {
        j = posList[i];
        curSection = trackData->lSections[secList[i]];

        curNode = curSection->lNodes[j];

        P5 = P1;
        P6 = P2;
        P7 = P3;
        P8 = P4;
        float banking = glm::atan(curNode->vLatHeart(-y).y, -curNode->vNorm.y)+F_PI_2+0.001;
        if(!smooth)
        {
            banking /= F_PI_2;
            banking = floor(banking);
            banking *= F_PI_2;
            banking += F_PI_4;
        }
        nextNorm = -(float)(radiusy*cos(banking))*curNode->vNorm+(float)(radiusx*sin(banking))*curNode->vLatHeart(-y);
        P1 = curNode->vRelPos(y, x)+nextNorm;
        P2 = glm::vec3(P1.x, -trackData->startPos.y-1.f, P1.z);
        banking = glm::atan(curNode->vLatHeart(-y).y, -curNode->vNorm.y)-F_PI_2-0.001;
        if(!smooth)
        {
            banking /= F_PI_2;
            banking = floor(banking);
            banking *= F_PI_2;
            banking += F_PI_4;
        }
        nextNorm = -(float)(radiusy*cos(banking))*curNode->vNorm+(float)(radiusx*sin(banking))*curNode->vLatHeart(-y);
        P4 = curNode->vRelPos(y, x)+nextNorm;
        P3 = glm::vec3(P4.x, -trackData->startPos.y-1.f, P4.z);
        if(i == 0)
        {
            createQuad(railshadows, P1, P2, P3, P4);
            railShadowSize+=2;
        }
        else if(i == posList.size()-1)
        {
            createBox(railshadows, P1, P2, P4, P3, P5, P6, P8, P7);
            railShadowSize+=8;
            createQuad(railshadows, P2, P1, P4, P3);
            railShadowSize+=2;
        }
        else
        {
            createBox(railshadows, P1, P2, P4, P3, P5, P6, P8, P7);
            railShadowSize+=8;
        }
    }

    return count;
}

void trackMesh::createSupport(QVector<tracknode_t> &list, int edges, float radiusy, float radiusx, glm::vec3 P1, glm::vec3 P2, bool smooth)
{
    const float r = 0.5f*(radiusy+radiusx);
    const float length = glm::length(P1-P2);

    glm::vec3 dir = glm::normalize(P2-P1);
    glm::vec3 up, side;
    if(fabs(dir.y) > 0.999) side = glm::normalize(glm::cross(dir, glm::vec3(0.f, 0.f, 1.f)));
    else side = glm::normalize(glm::cross(dir, glm::vec3(0.f, 1.f, 0.f)));

    up = glm::normalize(glm::cross(side, dir));

    float angleLeft;
    float angleRight;

    for(int i = 0; i < edges; ++i)
    {
        angleLeft = i*360.f/edges - 180.f/edges;
        angleRight = i*360.f/edges + 180.f/edges;

        nextPos = i%2 ? P1 : P2;
        nextNorm = (i%2 ? -1.f : 1.f)*dir;
        appendSupportNode(list, 0, (i%2) ? length : 0);


        nextNorm = -(float)(radiusy*cos(angleLeft*F_PI/180))*up+(float)(radiusx*sin(angleLeft*F_PI/180))*side;
        nextPos = i%2 ? P1+nextNorm : P2+nextNorm;
        if(smooth)
        {
            nextNorm = glm::normalize(nextNorm);
        }
        else
        {
            nextNorm = glm::normalize(-(float)(radiusy*cos((angleLeft+angleRight)*F_PI/360))*up+(float)(radiusx*sin((angleLeft+angleRight)*F_PI/360))*side);
        }
        appendSupportNode(list, r/0.3f*angleLeft/180.f, (i%2) ? length : 0);

        nextNorm = -(float)(radiusy*cos(angleRight*F_PI/180))*up+(float)(radiusx*sin(angleRight*F_PI/180))*side;
        nextPos = i%2 ? P1+nextNorm : P2+nextNorm;
        if(smooth)
        {
            nextNorm = glm::normalize(nextNorm);
        }
        else
        {
            nextNorm = glm::normalize(-(float)(radiusy*cos((angleLeft+angleRight)*F_PI/360))*up+(float)(radiusx*sin((angleLeft+angleRight)*F_PI/360))*side);
        }
        appendSupportNode(list, r/0.3f*angleRight/180.f, (i%2) ? length : 0);

        nextNorm = -(float)(radiusy*cos(angleLeft*F_PI/180))*up+(float)(radiusx*sin(angleLeft*F_PI/180))*side;
        nextPos = i%2 ? P2+nextNorm : P1+nextNorm;
        if(smooth)
        {
            nextNorm = glm::normalize(nextNorm);
        }
        else
        {
            nextNorm = glm::normalize(-(float)(radiusy*cos((angleLeft+angleRight)*F_PI/360))*up+(float)(radiusx*sin((angleLeft+angleRight)*F_PI/360))*side);
        }
        appendSupportNode(list, r/0.3f*angleLeft/180.f, (i%2) ? 0 : length);

        nextNorm = -(float)(radiusy*cos(angleRight*F_PI/180))*up+(float)(radiusx*sin(angleRight*F_PI/180))*side;
        nextPos = i%2 ? P2+nextNorm : P1+nextNorm;
        if(smooth)
        {
            nextNorm = glm::normalize(nextNorm);
        }
        else
        {
            nextNorm = glm::normalize(-(float)(radiusy*cos((angleLeft+angleRight)*F_PI/360))*up+(float)(radiusx*sin((angleLeft+angleRight)*F_PI/360))*side);
        }
        appendSupportNode(list, r/0.3f*angleRight/180.f, (i%2) ? 0 : length);

    }


    nextPos = edges%2 ? P2 : P1;
    nextNorm = (edges%2 ? 1.f : -1.f)*dir;
    appendSupportNode(list, 0, (edges%2) ? 0 : length);

    /* SHADOW STUFF */

    if(fabs(up.y) < 0.0001) return;
    glm::vec3 P1s, P2s, P3, P4, P5, P6, P7, P8;

    nextNorm = radiusx*side;
    P1s = P1+nextNorm;
    P2s = glm::vec3(P1s.x, -trackData->startPos.y-1, P1s.z);
    P4 = P1-nextNorm;
    P3 = glm::vec3(P4.x, -trackData->startPos.y-1, P4.z);
    P5 = P2+nextNorm;
    P6 = glm::vec3(P5.x, -trackData->startPos.y-1, P5.z);
    P8 = P2-nextNorm;
    P7 = glm::vec3(P8.x, -trackData->startPos.y-1, P8.z);

    //createQuad(railshadows, P4, P3, P2s, P1s);
    createQuad(supportshadows, P1s, P2s, P3, P4);
    railShadowSize+=2;
    createBox(supportshadows, P5, P6, P8, P7, P1s, P2s, P4, P3);
    //createBox(railshadows, P3, P4, P2s, P1s, P7, P8, P6, P5);
    railShadowSize+=8;
    //createQuad(railshadows, P5, P6, P7, P8);
    createQuad(supportshadows, P8, P7, P6, P5);
    railShadowSize+=2;
}


void trackMesh::createQuad(QVector<tracknode_t> &list, glm::vec3 P1, glm::vec3 P2, glm::vec3 P3, glm::vec3 P4)
{
    //float start = glm::length(P1);
    glm::vec3 base1 = glm::normalize(P1-P4);
    glm::vec3 base2 = glm::normalize(P1-P2);
    base2 = glm::cross(base1, base2);
    base2 = glm::cross(base1, base2);

    nextNorm = glm::normalize(glm::cross(P1-P4, P1-P2));
    //nextNorm = glm::rotateY(nextNorm, trackData->startYaw-90);
    nextPos = P1;
    appendTrackNode(list, glm::dot(P1, base1), glm::dot(P1, base2));
    nextPos = P2;
    appendTrackNode(list, glm::dot(P2, base1), glm::dot(P2, base2));
    nextPos = P4;
    appendTrackNode(list, glm::dot(P4, base1), glm::dot(P4, base2));

    nextNorm = glm::normalize(glm::cross(P2-P4, P2-P3));
    //nextNorm = glm::rotateY(nextNorm, trackData->startYaw-90);
    nextPos = P2;
    appendTrackNode(list, glm::dot(P2, base1), glm::dot(P2, base2));
    nextPos = P3;
    appendTrackNode(list, glm::dot(P3, base1), glm::dot(P3, base2));
    nextPos = P4;
    appendTrackNode(list, glm::dot(P4, base1), glm::dot(P4, base2));
}

void trackMesh::create3dsQuad(QVector<float> *_vertices, QVector<unsigned int> *_indices, glm::vec3 P1, glm::vec3 P2, glm::vec3 P3, glm::vec3 P4)
{
    int offset = _vertices->size()/3;
    _vertices->append(P1.x);
    _vertices->append(P1.y);
    _vertices->append(P1.z);

    _vertices->append(P2.x);
    _vertices->append(P2.y);
    _vertices->append(P2.z);

    _vertices->append(P3.x);
    _vertices->append(P3.y);
    _vertices->append(P3.z);

    _vertices->append(P4.x);
    _vertices->append(P4.y);
    _vertices->append(P4.z);

    _indices->append(offset+0);
    _indices->append(offset+2);
    _indices->append(offset+1);

    _indices->append(offset+0);
    _indices->append(offset+3);
    _indices->append(offset+2);
}

void trackMesh::createQuad(QVector<meshnode_t> &list, glm::vec3 P1, glm::vec3 P2, glm::vec3 P3, glm::vec3 P4)
{
    nextNorm = glm::normalize(glm::cross(P1-P4, P1-P2));
    //nextNorm = glm::rotateY(nextNorm, trackData->startYaw-90);
    nextPos = P1;
    appendMeshNode(list);
    nextPos = P2;
    appendMeshNode(list);
    nextPos = P4;
    appendMeshNode(list);

    nextNorm = glm::normalize(glm::cross(P2-P4, P2-P3));
    //nextNorm = glm::rotateY(nextNorm, trackData->startYaw-90);
    nextPos = P2;
    appendMeshNode(list);
    nextPos = P3;
    appendMeshNode(list);
    nextPos = P4;
    appendMeshNode(list);
}

int trackMesh::createShadowTriangle(QVector<meshnode_t> &list, glm::vec3 P1, glm::vec3 P2, glm::vec3 P3)
{
    nextNorm = glm::normalize(glm::cross(P1-P2, P1-P3));
    glm::vec3 lightvec = glm::vec3(0.f, 1.f, 0.f);
    if(glm::dot(nextNorm, lightvec) < 0) return 0;

    nextPos = P1;
    appendMeshNode(list);
    nextPos = P3;
    appendMeshNode(list);
    nextPos = P2;
    appendMeshNode(list);

    nextNorm = glm::vec3(0, -1, 0);
    nextPos = glm::vec3(P1.x, -1-trackData->startPos.y, P1.z);
    appendMeshNode(list);
    nextPos = glm::vec3(P2.x, -1-trackData->startPos.y, P2.z);
    appendMeshNode(list);
    nextPos = glm::vec3(P3.x, -1-trackData->startPos.y, P3.z);
    appendMeshNode(list);
    return 2;
}

void trackMesh::createBox(QVector<tracknode_t> &list, glm::vec3 P1l, glm::vec3 P2l, glm::vec3 P3l, glm::vec3 P4l, glm::vec3 P1r, glm::vec3 P2r, glm::vec3 P3r, glm::vec3 P4r)
{
    createQuad(list, P1l, P2l, P2r, P1r);
    createQuad(list, P2l, P4l, P4r, P2r);
    createQuad(list, P4l, P3l, P3r, P4r);
    createQuad(list, P3l, P1l, P1r, P3r);
}

void trackMesh::create3dsBox(QVector<float> *_vertices, QVector<unsigned int> *_indices, glm::vec3 P1l, glm::vec3 P2l, glm::vec3 P3l, glm::vec3 P4l, glm::vec3 P1r, glm::vec3 P2r, glm::vec3 P3r, glm::vec3 P4r)
{
    int offset = _vertices->size()/3;
    _vertices->append(P1l.x);
    _vertices->append(P1l.y);
    _vertices->append(P1l.z);

    _vertices->append(P2l.x);
    _vertices->append(P2l.y);
    _vertices->append(P2l.z);

    _vertices->append(P4l.x);
    _vertices->append(P4l.y);
    _vertices->append(P4l.z);

    _vertices->append(P3l.x);
    _vertices->append(P3l.y);
    _vertices->append(P3l.z);

    _vertices->append(P1r.x);
    _vertices->append(P1r.y);
    _vertices->append(P1r.z);

    _vertices->append(P2r.x);
    _vertices->append(P2r.y);
    _vertices->append(P2r.z);

    _vertices->append(P4r.x);
    _vertices->append(P4r.y);
    _vertices->append(P4r.z);

    _vertices->append(P3r.x);
    _vertices->append(P3r.y);
    _vertices->append(P3r.z);

    const int l = 0;
    const int r = 4;
    QList<int> points;
    for(int i = 0; i < 5; ++i)
    {
        points.append(offset+i%4+l);
        points.append(offset+i%4+r);
    }
    for(int i = 0; i < points.size()-2; ++i)
    {
        if(i%2)
        {
            _indices->append(points[i]);
            _indices->append(points[i+2]);
            _indices->append(points[i+1]);
        }
        else
        {
            _indices->append(points[i]);
            _indices->append(points[i+1]);
            _indices->append(points[i+2]);
        }
    }
}

void trackMesh::createBox(QVector<meshnode_t> &list, glm::vec3 P1l, glm::vec3 P2l, glm::vec3 P3l, glm::vec3 P4l, glm::vec3 P1r, glm::vec3 P2r, glm::vec3 P3r, glm::vec3 P4r)
{
    createQuad(list, P1l, P2l, P2r, P1r);
    createQuad(list, P2l, P4l, P4r, P2r);
    createQuad(list, P4l, P3l, P3r, P4r);
    createQuad(list, P3l, P1l, P1r, P3r);
}

int trackMesh::createShadowBox(QVector<meshnode_t> &list, glm::vec3 P1l, glm::vec3 P2l, glm::vec3 P3l, glm::vec3 P4l, glm::vec3 P1r, glm::vec3 P2r, glm::vec3 P3r, glm::vec3 P4r)
{
    int count = 0;
    glm::vec3 PS1, PS2;
    glm::vec3 lightvec = glm::vec3(0.f, 1.f, 0.f);
    glm::vec3 Nor1 = glm::normalize(glm::cross(P1l-P1r, P1l-P2l));
    glm::vec3 Nor2 = glm::normalize(glm::cross(P2l-P1r, P2l-P2r));
    float dot1, dot2;
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);
    PS1 = glm::vec3(P2l.x, -trackData->startPos.y-1.f, P2l.z);
    PS2 = glm::vec3(P1r.x, -trackData->startPos.y-1.f, P1r.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P2l, P1r, PS2, PS1);
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P1r, P2l, PS1, PS2);
        count+=2;
    }


    Nor1 = Nor2;
    Nor2 = glm::normalize(glm::cross(P2l-P2r, P2l-P4l));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P2l.x, -trackData->startPos.y-1.f, P2l.z);
    PS2 = glm::vec3(P2r.x, -trackData->startPos.y-1.f, P2r.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P2l, P2r, PS2, PS1);
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P2r, P2l, PS1, PS2);
        count+=2;
    }

    Nor1 = Nor2;
    Nor2 = glm::normalize(glm::cross(P4l-P2r, P4l-P4r));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P4l.x, -trackData->startPos.y-1.f, P4l.z);
    PS2 = glm::vec3(P2r.x, -trackData->startPos.y-1.f, P2r.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P4l, P2r, PS2, PS1);
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P2r, P4l, PS1, PS2);
        count+=2;
    }


    Nor1 = Nor2;
    Nor2 = glm::normalize(glm::cross(P4l-P4r, P4l-P3l));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P4l.x, -trackData->startPos.y-1.f, P4l.z);
    PS2 = glm::vec3(P4r.x, -trackData->startPos.y-1.f, P4r.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P4l, P4r, PS2, PS1); // x
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P4r, P4l, PS1, PS2);
        count+=2;
    }

    Nor1 = Nor2;
    Nor2 = glm::normalize(glm::cross(P3l-P4r, P4r-P3r));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P3l.x, -trackData->startPos.y-1.f, P3l.z);
    PS2 = glm::vec3(P4r.x, -trackData->startPos.y-1.f, P4r.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P3l, P4r, PS2, PS1);
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P4r, P3l, PS1, PS2); // x
        count+=2;
    }

    Nor1 = Nor2;
    Nor2 = glm::normalize(glm::cross(P3l-P3r, P3l-P1l));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P3l.x, -trackData->startPos.y-1.f, P3l.z);
    PS2 = glm::vec3(P3r.x, -trackData->startPos.y-1.f, P3r.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P3l, P3r, PS2, PS1); // x
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P3r, P3l, PS1, PS2);
        count+=2;
    }

    Nor1 = Nor2;
    Nor2 = glm::normalize(glm::cross(P1l-P3r, P1l-P1r));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P1l.x, -trackData->startPos.y-1.f, P1l.z);
    PS2 = glm::vec3(P3r.x, -trackData->startPos.y-1.f, P3r.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P1l, P3r, PS2, PS1);
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P3r, P1l, PS1, PS2);
        count+=2;
    }

    Nor1 = Nor2;
    Nor2 = glm::normalize(glm::cross(P1l-P1r, P1l-P2l));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P1l.x, -trackData->startPos.y-1.f, P1l.z);
    PS2 = glm::vec3(P1r.x, -trackData->startPos.y-1.f, P1r.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P1l, P1r, PS2, PS1);
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P1r, P1l, PS1, PS2); // x
        count+=2;
    }

    /* torus until here */

    Nor1 = Nor2;
    Nor2 = glm::normalize(glm::cross(P1l-P2l, P1l-P4l));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P1l.x, -trackData->startPos.y-1.f, P1l.z);
    PS2 = glm::vec3(P2l.x, -trackData->startPos.y-1.f, P2l.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P1l, P2l, PS2, PS1);
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P2l, P1l, PS1, PS2);
        count+=2;
    }

    Nor1 = Nor2;
    Nor2 = glm::normalize(glm::cross(P1l-P4l, P1l-P3l));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P1l.x, -trackData->startPos.y-1.f, P1l.z);
    PS2 = glm::vec3(P4l.x, -trackData->startPos.y-1.f, P4l.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P1l, P4l, PS2, PS1);
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P4l, P1l, PS1, PS2);
        count+=2;
    }

    Nor1 = Nor2;
    Nor2 = glm::normalize(glm::cross(P3l-P4l, P3l-P4r));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P3l.x, -trackData->startPos.y-1.f, P3l.z);
    PS2 = glm::vec3(P4l.x, -trackData->startPos.y-1.f, P4l.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P3l, P4l, PS2, PS1);
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P4l, P3l, PS1, PS2);
        count+=2;
    }

    Nor1 = glm::normalize(glm::cross(P3l-P3r, P3l-P1l));
    Nor2 = glm::normalize(glm::cross(P3l-P1l, P3l-P4l));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P3l.x, -trackData->startPos.y-1.f, P3l.z);
    PS2 = glm::vec3(P1l.x, -trackData->startPos.y-1.f, P1l.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P3l, P1l, PS2, PS1);
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P1l, P3l, PS1, PS2); // x
        count+=2;
    }

    Nor1 = glm::normalize(glm::cross(P4l-P1l, P4l-P2l));
    Nor2 = glm::normalize(glm::cross(P4l-P2l, P4l-P2r));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P4l.x, -trackData->startPos.y-1.f, P4l.z);
    PS2 = glm::vec3(P2l.x, -trackData->startPos.y-1.f, P2l.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P4l, P2l, PS2, PS1);
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P2l, P4l, PS1, PS2);
        count+=2;
    }

    /* left side until here */

    Nor1 = glm::normalize(glm::cross(P1r-P2r, P1r-P2l));
    Nor2 = glm::normalize(glm::cross(P2r-P1r, P2r-P4r));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P2r.x, -trackData->startPos.y-1.f, P2r.z);
    PS2 = glm::vec3(P1r.x, -trackData->startPos.y-1.f, P1r.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P2r, P1r, PS2, PS1); // x
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P1r, P2r, PS1, PS2);
        count+=2;
    }

    Nor1 = Nor2;
    Nor2 = glm::normalize(glm::cross(P4r-P1r, P4r-P3r));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P4r.x, -trackData->startPos.y-1.f, P4r.z);
    PS2 = glm::vec3(P1r.x, -trackData->startPos.y-1.f, P1r.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P4r, P1r, PS2, PS1);
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P1r, P4r, PS1, PS2);
        count+=2;
    }

    Nor1 = Nor2;
    Nor2 = glm::normalize(glm::cross(P4r-P3r, P4r-P3l));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P4r.x, -trackData->startPos.y-1.f, P4r.z);
    PS2 = glm::vec3(P3r.x, -trackData->startPos.y-1.f, P3r.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P4r, P3r, PS2, PS1);
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P3r, P4r, PS1, PS2); // x
        count+=2;
    }

    Nor1 = glm::normalize(glm::cross(P1r-P3r, P1r-P4r));
    Nor2 = glm::normalize(glm::cross(P3r-P1r, P3r-P1l));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P3r.x, -trackData->startPos.y-1.f, P3r.z);
    PS2 = glm::vec3(P1r.x, -trackData->startPos.y-1.f, P1r.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P3r, P1r, PS2, PS1);
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P1r, P3r, PS1, PS2);
        count+=2;
    }

    Nor1 = glm::normalize(glm::cross(P2r-P4r, P2r-P4l));
    Nor2 = glm::normalize(glm::cross(P4r-P2r, P4r-P1r));
    dot1=glm::dot(Nor1, lightvec);
    dot2 = glm::dot(Nor2, lightvec);

    PS1 = glm::vec3(P4r.x, -trackData->startPos.y-1.f, P4r.z);
    PS2 = glm::vec3(P2r.x, -trackData->startPos.y-1.f, P2r.z);
    if(dot1 <= 0 && dot2 > 0)
    {
        createQuad(list, P4r, P2r, PS2, PS1); // x
        count+=2;
    }
    else if(dot2 <= 0 && dot1 > 0)
    {
        createQuad(list, P2r, P4r, PS1, PS2);
        count+=2;
    }

    count+=createShadowTriangle(list, P2l, P1r, P1l);
    count+=createShadowTriangle(list, P2l, P2r, P1r);
    count+=createShadowTriangle(list, P4l, P2r, P2l);
    count+=createShadowTriangle(list, P4l, P4r, P2r);
    count+=createShadowTriangle(list, P4l, P3l, P4r);
    count+=createShadowTriangle(list, P3l, P3r, P4r);
    count+=createShadowTriangle(list, P1l, P3r, P3l);
    count+=createShadowTriangle(list, P1l, P1r, P3r);

    count+=createShadowTriangle(list, P4l, P2l, P1l);
    count+=createShadowTriangle(list, P4l, P1l, P3l);
    count+=createShadowTriangle(list, P2r, P4r, P1r);
    count+=createShadowTriangle(list, P4r, P3r, P1r);


    return count;
}

void trackMesh::buildMeshes(int fromNode)
{
    if(glView->legacyMode) return;
    //rails.clear();
    //crossties.clear();
    rendersupports.clear();
    supports.clear();
    //railshadows.clear();
    //crosstieshadows.clear();
    //heartline.clear();
    supportshadows.clear();
    pipeIndices.clear();
    shadowIndices.clear();

    posList.clear();
    secList.clear();

    trackVertexSize = 0;
    supportsSize = 0;
    heartlineSize = 0;
    railShadowSize = 0;

    if(fromNode < 0) fromNode = 0;
    int fromSection, fromSecI = fromNode;

    for(fromSection = 0; fromSection < trackData->lSections.size(); ++fromSection)
    {
        int tmp = trackData->lSections[fromSection]->lNodes.size()-1;
        if(fromSecI > tmp) fromSecI -= tmp;
        else break;
    }


    float crosstieSpacing = 0.f;

    float meshQuality;
    switch(gloParent->mOptions->meshQuality)
    {
    case 0:
        meshQuality = 1;
        break;
    case 1:
        meshQuality = 2;
        break;
    case 2:
        meshQuality = 4;
        break;
    case 3:
        meshQuality = 6;
        break;
    }
    float minNodeDist = 12.f/(meshQuality), maxNodeDist = 0.3f/(meshQuality);    // minimal and maximal distance between nodes
    float angleNodeDist = 6.f/(meshQuality);                     // after x degrees difference force a new node

    float railSpacing = 0.f;
    float distFromLastNode = 0.f;
    float spineHeight = (trackData->fHeart < 0 ? -1.f : 1.f);
    float spineSize = 0.06f;

    float railWidth = 0.065f;


    mnode *lastNode = NULL;
    curNode = NULL;

    QElapsedTimer timer;
    double mSec;
    timer.start();

    switch(trackData->style)
    {
    case generic:
        numRails = 3;
        railSpacing = 0.5f;
        crosstieSpacing = 1.5f;
        spineHeight = 0.30f * (trackData->fHeart < 0 ? -1.f : 1.f);
        spineSize = 0.22f; // used to be 0.18f
        break;
    case genericflat:
        numRails = 2;
        railSpacing = 0.7f;
        crosstieSpacing = 1.4f;
        break;
    case vekoma:
        numRails = 3;
        railSpacing = 0.60;
        crosstieSpacing = 1.5f;
        spineHeight = 0.85f * (trackData->fHeart < 0 ? -1.f : 1.f);
        spineSize = 0.22f;
        break;
    case bm:
        numRails = 3;
        railSpacing = 0.6f;
        crosstieSpacing = 1.45f;
        spineHeight = 0.5f * (trackData->fHeart < 0 ? -1.f : 1.f);
        spineSize = 0.26f*1.4101f;
        break;
    case triangle:
        numRails = 3;
        railSpacing = 0.5f;
        crosstieSpacing = 1.0f;
        spineSize = railWidth;
        spineHeight = 0.75f * (trackData->fHeart < 0 ? -1.f : 1.f);
        break;
    case box:
        numRails = 4;
        railSpacing = 0.5f;
        crosstieSpacing = 1.0f;
        spineSize = railWidth;
        spineHeight = 1.f * (trackData->fHeart < 0 ? -1.f : 1.f);
        break;
    case smallflat:
        numRails = 2;
        railSpacing = 0.5f;
        crosstieSpacing = 0.8f;
        break;
    case doublespine:
        numRails = 4;
        railSpacing = 0.5f;
        crosstieSpacing = 0.3f;
        spineHeight = 0.3f * (trackData->fHeart < 0 ? -1.f : 1.f);
        spineSize = 0.18f;
        break;
    }

    if(!isWireframe)
    {
        // delete obsolete railnodes
        int railNode;

        if(rails.size() && fromNode >= rails.last().node)
        {
            fromNode = rails.last().node-5;
        }

        for(railNode = 0; railNode < rails.size() && rails[railNode].node < fromNode; ++railNode);
        rails.remove(railNode, rails.size()-railNode);

        railNode = rails.size() ? rails.last().node : 0;

        if(railNode == 0)
        {
            rails.clear();
        }

        int i;

        trackData->getSecNode(railNode, &j, &i);

        if(i < 0) i = 0;

        if(i == j && j == 0) distFromLastNode = 1.f;

        for(; i < trackData->lSections.size(); i++)
        {
            curSection = trackData->lSections[i];
            for(; j < curSection->lNodes.size(); ++j)
            {
                if(i != 0 && j == 1) distFromLastNode = 1.f;
                float angle = curSection->lNodes.at(j)->fFlexion();
                angle /= angleNodeDist;
                angle = std::min(std::max(1.f/minNodeDist, angle), 1.f/maxNodeDist);
                angle *= curSection->lNodes.at(j)->fDistFromLast;;
                distFromLastNode += angle;
                if(distFromLastNode >= 1.f || j == curSection->lNodes.size()-1)
                {
                    if(distFromLastNode >= 1.f)
                    {
                        distFromLastNode -= 1.f;
                    }
                    else
                    {
                        distFromLastNode = 0.f;
                    }
                    if(rails.size() == 0 || rails.last().node != trackData->getNumPoints(curSection) + j)
                    {
                        posList.append(j);
                        secList.append(i);
                    }
                }
            }
            for(j = 0; j < curSection->supList.size(); j+=2)
            {
                glm::vec3 P1 = curSection->supList[j];
                glm::vec3 P2 = curSection->supList[j+1];

                createSupport(rendersupports, 12, 0.2f, 0.2f, P1, P2, true);
                supportsSize += 1;

                //supportsSize += 3;
                //nextPos = curSection->supList[j];
                //appendMeshNode(supports);
            }
            j = 0;
        }

        int jSize = posList.size();
        rails.reserve(jSize*12);
        //railshadows.reserve(jSize*12*8);
        //qDebug("Generated Points: %d", jSize);

        while(nodeList.size() && nodeList.last() >= fromNode) nodeList.removeLast();


        for(railNode = 0; railNode < railshadows.size() && railshadows[railNode].node < fromNode; ++railNode);
        railshadows.remove(railNode, railshadows.size()-railNode);

        for(railNode = 0; railNode < heartline.size() && heartline[railNode].node < fromNode; ++railNode);
        heartline.remove(railNode, heartline.size()-railNode);

        for(int i = 0; i < jSize; ++i)
        {
            j = posList[i];
            section* curSection = trackData->lSections[secList[i]];
            curNode = curSection->lNodes[j];

            heartlineSize += 1;

            nextPos = curNode->vPos;
            nextNode = trackData->getNumPoints(curSection) + j;
            if(nodeList.isEmpty() || (nodeList.size() && nodeList.last() != nextNode)) nodeList.append(nextNode);
            if(!heartline.size() || nextNode != heartline.last().node) appendMeshNode(heartline);
        }

        options.clear();
        pipeoption_t temp;
        temp.edges = 12;
        temp.offset = glm::vec2(-railSpacing, -trackData->fHeart);
        temp.radius = glm::vec2(railWidth, railWidth);
        temp.smooth = true;
        options.append(temp);
        temp.offset.x *= -1;
        options.append(temp);

        switch(trackData->style)
        {
        case bm:
            temp.smooth = false;
            temp.edges=8;
            temp.offset.x = 0;
            temp.offset.y = -trackData->fHeart-spineHeight;
            temp.radius.x = spineSize;
            temp.radius.y = spineSize;
            options.append(temp);
            //createPipe(rails, 4, spineSize*1.2, spineSize, -trackData->fHeart-spineHeight, 0, false);
            //createPipe(rails, 4, 0, 0, -trackData->fHeart-spineHeight, 0, false);
            //createPipe(rails, 4, 0, 0, -trackData->fHeart-spineHeight, 0, false);
            break;
        case generic:
        case vekoma:
            temp.edges = 24;
        case triangle:
            temp.offset.x = 0;
            temp.offset.y = -trackData->fHeart-spineHeight;
            temp.radius.x = spineSize;
            temp.radius.y = spineSize;
            options.append(temp);
            //createPipe(rails, 12, spineSize, spineSize, -trackData->fHeart-spineHeight, 0);
            break;
        case box:
            temp.offset.x = -railSpacing;
            temp.offset.y = -trackData->fHeart-spineHeight;
            temp.radius.x = spineSize;
            temp.radius.y = spineSize;
            options.append(temp);
            temp.offset.x = railSpacing;
            options.append(temp);
            //createPipe(rails, 12, spineSize, spineSize, -trackData->fHeart-spineHeight, -railSpacing);
            //createPipe(rails, 12, spineSize, spineSize, -trackData->fHeart-spineHeight, railSpacing);
            break;
        case genericflat:
        case smallflat:
            break;
        case doublespine:
            temp.edges = 24;
            temp.offset.x = 0;
            temp.offset.y = -trackData->fHeart-spineHeight;
            temp.radius.x = spineSize;
            temp.radius.y = spineSize;
            options.append(temp);
            temp.offset.y = -trackData->fHeart-0.45f-spineHeight;
            options.append(temp);
        }

        createPipes(rails, options);


        createIndices();

        /*trackVertexSize += createPipe(rails, 12, railWidth, railWidth, -trackData->fHeart, -railSpacing);
        createPipe(rails, 12, railWidth, railWidth, -trackData->fHeart, railSpacing);
        switch(trackData->style)
        {
        case bm:
            createPipe(rails, 4, spineSize*1.2, spineSize, -trackData->fHeart-spineHeight, 0, false);
            createPipe(rails, 4, 0, 0, -trackData->fHeart-spineHeight, 0, false);
            createPipe(rails, 4, 0, 0, -trackData->fHeart-spineHeight, 0, false);
            break;
        case generic:
        case vekoma:
        case triangle:
            createPipe(rails, 12, spineSize, spineSize, -trackData->fHeart-spineHeight, 0);
            break;
        case box:
            createPipe(rails, 12, spineSize, spineSize, -trackData->fHeart-spineHeight, -railSpacing);
            createPipe(rails, 12, spineSize, spineSize, -trackData->fHeart-spineHeight, railSpacing);
            break;
        case genericflat:
        case smallflat:
            break;
        }*/



        // crossties

        int iCrosstie = 0, iCrossShadow = 0;
        while(crossties.size() > iCrosstie && fromNode > crossties[iCrosstie].node) iCrosstie++; // cycle through crossties until we have something to change
        while(crosstieshadows.size() > iCrossShadow && fromNode > crosstieshadows[iCrossShadow].node) iCrossShadow++;

        if(iCrosstie == 0)  distFromLastNode = crosstieSpacing/2.f;
        else distFromLastNode = trackData->getPoint(fromNode)->fTotalLength - trackData->getPoint(crossties[iCrosstie-1].node)->fTotalLength;

        crossties.remove(iCrosstie, crossties.size()-iCrosstie);
        crosstieshadows.remove(iCrossShadow, crosstieshadows.size()-iCrossShadow);


        curNode = NULL;

        posList.clear();
        secList.clear();

        j = fromSecI;
        for(int i = fromSection; i < trackData->lSections.size(); i++)
        {
            section* curSection = trackData->lSections[i];
            for(; j < curSection->lNodes.size(); ++j)
            {
                distFromLastNode += curSection->lNodes.at(j)->fDistFromLast;
                if(distFromLastNode >= crosstieSpacing)
                {
                    distFromLastNode -= crosstieSpacing;
                    posList.append(j);
                    secList.append(i);
                }
            }
            j = 0;
        }

        jSize = posList.size();

        int offset = 0;
        switch(trackData->style)
        {
        case generic:
            offset = crossties.size()/48;
            break;
        case genericflat:
            offset = (crossties.size()+24)/48;
            break;
        case vekoma:
            offset = crossties.size()/228;
            break;
        case bm:
            offset = 6*(crossties.size()/444) + ((crossties.size()%444)-12)/72;
            break;
        case triangle:
            offset = (crossties.size()+72)/120;
            break;
        case box:
            offset = (crossties.size()+96)/192;
            break;
        case smallflat:
            offset = crossties.size()/24;
            break;
        case doublespine:
            offset = 2*(crossties.size()/(4*24)) + ((crossties.size()/24)%4)/3;
            break;
        }

        if(crossties.size()) curNode = trackData->getPoint(crossties.last().node);

#define XS (0.06f)
#define S (0.08f)
#define M (0.10f)
#define L (0.12f)
#define XL (0.14f)
#define XXL (0.16f)

#define BOX_INWARD (0.14f)
#define BOX_DIAG0 (0.08f)
#define BOX_DIAG1 (0.18f)
#define BOX_WIDTH (0.05f)


        for(int i = 0; i < jSize; ++i)
        {
            int index = i+offset;
            j = posList[i];
            curSection = trackData->lSections[secList[i]];
            lastNode = curNode;
            curNode = curSection->lNodes[j];
            nextNode = trackData->getNumPoints(curSection) + j;

            /* CROSSTIE GENERATION */
            glm::vec3 P1, P2, P3, P4, P5, P6, P7, P8;
            float mysign = fabs(spineHeight)/spineHeight;
            switch(trackData->style)
            {
            case generic:
                P1 = curNode->vRelPos(-trackData->fHeart+0.1*railWidth*mysign, -railSpacing, -0.15*spineHeight);
                P2 = curNode->vRelPos(-trackData->fHeart+0.1*railWidth*mysign, -railSpacing, 0.15*spineHeight);
                P3 = curNode->vRelPos(-trackData->fHeart, -railSpacing, -0.15*spineHeight);
                P4 = curNode->vRelPos(-trackData->fHeart, -railSpacing, 0.15*spineHeight);
                P5 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.15, 0, -0.15f);//-0.35*spineHeight);
                P6 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.15, 0, 0.15f);//0.35*spineHeight);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, -0.15f);// -0.15*spineHeight);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, 0.15f);// 0.15*spineHeight);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.15, 0, -0.15f);//-0.35*spineHeight);
                P2 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.15, 0, 0.15f);//0.35*spineHeight);
                P3 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, -0.15f);//, -0.15*spineHeight);
                P4 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, 0.15f);//, 0.15*spineHeight);
                P5 = curNode->vRelPos(-trackData->fHeart+0.1*railWidth*mysign, railSpacing, -0.15*spineHeight);
                P6 = curNode->vRelPos(-trackData->fHeart+0.1*railWidth*mysign, railSpacing, 0.15*spineHeight);
                P7 = curNode->vRelPos(-trackData->fHeart, railSpacing, -0.15*spineHeight);
                P8 = curNode->vRelPos(-trackData->fHeart, railSpacing, 0.15*spineHeight);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);
                break;
            case genericflat:
                if(index%6 == 0)
                {
                    P1 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.8 : 1.0), -railSpacing, -M);
                    P2 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.8 : 1.0), -railSpacing, M);
                    P3 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1.0 : 0.8), -railSpacing, -M);
                    P4 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1.0 : 0.8), -railSpacing, M);
                    P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.8 : 1.0), railSpacing, -M);
                    P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.8 : 1.0), railSpacing, M);
                    P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1.0 : 0.8), railSpacing, -M);
                    P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1.0 : 0.8), railSpacing, M);

                }
                else
                {
                    P1 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), -railSpacing, -0.03);
                    P2 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), -railSpacing, 0.03);
                    P3 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), -railSpacing, -0.03);
                    P4 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), -railSpacing, 0.03);
                    P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), railSpacing, -0.03);
                    P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), railSpacing, 0.03);
                    P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), railSpacing, -0.03);
                    P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), railSpacing, 0.03);
                }

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                if(index)
                {
                    P1 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), -railSpacing+0.13, 0.03);
                    P2 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), -railSpacing+0.07, 0.03);
                    P3 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), -railSpacing+0.13, 0.03);
                    P4 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), -railSpacing+0.07, 0.03);
                    P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), railSpacing-0.07, -0.03);
                    P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), railSpacing-0.13, -0.03);
                    P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), railSpacing-0.07, -0.03);
                    P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), railSpacing-0.13, -0.03);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);
                }
                break;
            case vekoma:
                P1 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, -railSpacing-0.5*railWidth, -0.05f);
                P2 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, -railSpacing-0.5*railWidth, +0.05f);
                P3 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -railSpacing-0.5*railWidth, -0.05f);
                P4 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -railSpacing-0.5*railWidth, +0.05f);
                P5 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, -1.2*railSpacing, -0.05f);
                P6 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, -1.2*railSpacing, +0.05f);
                P7 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -1.2*railSpacing, -0.05f);
                P8 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -1.2*railSpacing, +0.05f);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, railSpacing+0.5*railWidth, -0.05f);
                P2 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, railSpacing+0.5*railWidth, +0.05f);
                P3 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, railSpacing+0.5*railWidth, -0.05f);
                P4 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, railSpacing+0.5*railWidth, +0.05f);
                P5 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, 1.2*railSpacing, -0.05f);
                P6 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, 1.2*railSpacing, +0.05f);
                P7 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, 1.2*railSpacing, -0.05f);
                P8 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, 1.2*railSpacing, +0.05f);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);


                P1 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -1.2*railSpacing, -0.05f);
                P2 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -1.2*railSpacing, +0.05f);
                P3 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, -1.3*railSpacing, -0.05f);
                P4 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, -1.3*railSpacing, +0.05f);
                P5 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, -1.2*railSpacing, -0.05f);
                P6 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, -1.2*railSpacing, +0.05f);
                P7 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, -1.3*railSpacing, -0.05f);
                P8 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, -1.3*railSpacing, +0.05f);

                createQuad(crossties, P2, P1, P3, P4);
                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, 1.3*railSpacing, -0.05f);
                P2 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, 1.3*railSpacing, +0.05f);
                P3 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, 1.2*railSpacing, -0.05f);
                P4 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, 1.2*railSpacing, +0.05f);
                P5 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, 1.3*railSpacing, -0.05f);
                P6 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, 1.3*railSpacing, +0.05f);
                P7 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, 1.2*railSpacing, -0.05f);
                P8 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, 1.2*railSpacing, +0.05f);

                createQuad(crossties, P2, P1, P3, P4);
                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, -1.3*railSpacing, -0.05f);
                P2 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, -1.3*railSpacing, +0.05f);
                P3 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, -1.3*railSpacing, -0.05f);
                P4 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, -1.3*railSpacing, +0.05f);
                P5 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, 1.3*railSpacing, -0.05f);
                P6 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, 1.3*railSpacing, +0.05f);
                P7 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, 1.3*railSpacing, -0.05f);
                P8 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, 1.3*railSpacing, +0.05f);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, -0.9*railSpacing, -0.07f);
                P2 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, -0.9*railSpacing, +0.07f);
                P3 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, -0.9*railSpacing, -0.07f);
                P4 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, -0.9*railSpacing, +0.07f);
                P5 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, -1.1*spineSize, -0.07f);
                P6 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, -1.1*spineSize, +0.07f);
                P7 = curNode->vRelPos(-trackData->fHeart-0.9*spineHeight-0.5*mysign*(spineSize-0.05), -1.1*spineSize, -0.07f);
                P8 = curNode->vRelPos(-trackData->fHeart-0.9*spineHeight-0.5*mysign*(spineSize-0.05), -1.1*spineSize, +0.07f);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = P5;
                P2 = P6;
                P3 = P7;
                P4 = P8;
                P5 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, 1.1*spineSize, -0.07f);
                P6 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, 1.1*spineSize, +0.07f);
                P7 = curNode->vRelPos(-trackData->fHeart-0.9*spineHeight-0.5*mysign*(spineSize-0.05), 1.1*spineSize, -0.07f);
                P8 = curNode->vRelPos(-trackData->fHeart-0.9*spineHeight-0.5*mysign*(spineSize-0.05), 1.1*spineSize, +0.07f);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = P5;
                P2 = P6;
                P3 = P7;
                P4 = P8;
                P5 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, 0.9*railSpacing, -0.07f);
                P6 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, 0.9*railSpacing, +0.07f);
                P7 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, 0.9*railSpacing, -0.07f);
                P8 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, 0.9*railSpacing, +0.07f);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);


                P1 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, 0.9*railSpacing, -0.07f);
                P2 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, -0.9*railSpacing, -0.07f);
                P3 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, 0.9*railSpacing, -0.07f);
                P4 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, -0.9*railSpacing, -0.07f);
                P5 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, 0.9*railSpacing, +0.07f);
                P6 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, -0.9*railSpacing, +0.07f);
                P7 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, 0.9*railSpacing, +0.07f);
                P8 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, -0.9*railSpacing, +0.07f);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);
                break;
            case bm:
                P1 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -railSpacing, -0.05f*mysign);
                P2 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -railSpacing, 0.05f*mysign);
                P3 = curNode->vRelPos(-trackData->fHeart, -railSpacing, -0.05f*mysign);
                P4 = curNode->vRelPos(-trackData->fHeart, -railSpacing, 0.05f*mysign);
                P5 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, -0.3*spineSize, -0.05f*mysign);
                P6 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, -0.3*spineSize, 0.05f*mysign);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, -0.71*spineSize, -0.05f*mysign);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, -0.71*spineSize, 0.05f*mysign);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);


                if(index%6 == 0)
                {
                    P1 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, -0.71*spineSize*1.3, -0.05f*mysign);
                    P2 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, -0.71*spineSize*1.3, 0.05f*mysign);
                    P3 = curNode->vRelPos(-trackData->fHeart-spineHeight-1.3*0.71*spineSize*mysign, -0.71*spineSize*1.3, -0.05f*mysign);
                    P4 = curNode->vRelPos(-trackData->fHeart-spineHeight-1.3*0.71*spineSize*mysign, -0.71*spineSize*1.3, 0.05f*mysign);
                    P5 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, 0.71*spineSize*1.3, -0.05f*mysign);
                    P6 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, 0.71*spineSize*1.3, 0.05f*mysign);
                    P7 = curNode->vRelPos(-trackData->fHeart-spineHeight-1.3*0.71*spineSize*mysign, 0.71*spineSize*1.3, -0.05f*mysign);
                    P8 = curNode->vRelPos(-trackData->fHeart-spineHeight-1.3*0.71*spineSize*mysign, 0.71*spineSize*1.3, 0.05f*mysign);

                    createQuad(crossties, P2, P1, P3, P4);
                    createQuad(crossties, P5, P6, P8, P7);
                }
                else
                {
                    P1 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, -0.71*spineSize, -0.05f*mysign);
                    P2 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, -0.71*spineSize, 0.05f*mysign);
                    P3 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, -0.71*spineSize, -0.05f*mysign);
                    P4 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, -0.71*spineSize, 0.05f*mysign);
                    P5 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, 0.71*spineSize, -0.05f*mysign);
                    P6 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, 0.71*spineSize, 0.05f*mysign);
                    P7 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, 0.71*spineSize, -0.05f*mysign);
                    P8 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, 0.71*spineSize, 0.05f*mysign);
                }

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, 0.3*spineSize, -0.05f*mysign);
                P2 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, 0.3*spineSize, 0.05f*mysign);
                P3 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, 0.71*spineSize, -0.05f*mysign);
                P4 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, 0.71*spineSize, 0.05f*mysign);
                P5 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, railSpacing, -0.05f*mysign);
                P6 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, railSpacing, 0.05f*mysign);
                P7 = curNode->vRelPos(-trackData->fHeart, railSpacing, -0.05f*mysign);
                P8 = curNode->vRelPos(-trackData->fHeart, railSpacing, 0.05f*mysign);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);
                break;
            case triangle:
                P1 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), -railSpacing, -BOX_WIDTH);
                P2 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), -railSpacing, BOX_WIDTH);
                P3 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), -railSpacing, -BOX_WIDTH);
                P4 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), -railSpacing, BOX_WIDTH);
                P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), railSpacing, -BOX_WIDTH);
                P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), railSpacing, BOX_WIDTH);
                P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), railSpacing, -BOX_WIDTH);
                P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), railSpacing, BOX_WIDTH);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD+railWidth/2), -BOX_WIDTH);
                P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD+railWidth/2), BOX_WIDTH);
                P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD-railWidth/2), -BOX_WIDTH);
                P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD-railWidth/2), BOX_WIDTH);
                P5 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, -BOX_WIDTH);
                P6 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, BOX_WIDTH);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, -BOX_WIDTH);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, BOX_WIDTH);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD+railWidth/2), -BOX_WIDTH);
                P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD+railWidth/2), BOX_WIDTH);
                P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD-railWidth/2), -BOX_WIDTH);
                P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD-railWidth/2), BOX_WIDTH);
                P5 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, -BOX_WIDTH);
                P6 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, BOX_WIDTH);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, -BOX_WIDTH);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, BOX_WIDTH);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                if(index && index%2)
                {
                    P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD+railWidth/2), -BOX_WIDTH);
                    P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD+railWidth/2), BOX_WIDTH);
                    P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD-railWidth/2), -BOX_WIDTH);
                    P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD-railWidth/2), BOX_WIDTH);
                    P5 = lastNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, -BOX_WIDTH);
                    P6 = lastNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, BOX_WIDTH);
                    P7 = lastNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, -BOX_WIDTH);
                    P8 = lastNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, BOX_WIDTH);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                    P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD+railWidth/2), -BOX_WIDTH);
                    P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD+railWidth/2), BOX_WIDTH);
                    P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD-railWidth/2), -BOX_WIDTH);
                    P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD-railWidth/2), BOX_WIDTH);
                    P5 = lastNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, -BOX_WIDTH);
                    P6 = lastNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, BOX_WIDTH);
                    P7 = lastNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, -BOX_WIDTH);
                    P8 = lastNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, BOX_WIDTH);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);
                }
                else if(index && index%2 == 0)
                {
                    P1 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD+railWidth/2), -BOX_WIDTH);
                    P2 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD+railWidth/2), BOX_WIDTH);
                    P3 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD-railWidth/2), -BOX_WIDTH);
                    P4 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD-railWidth/2), BOX_WIDTH);
                    P5 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, -BOX_WIDTH);
                    P6 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, BOX_WIDTH);
                    P7 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, -BOX_WIDTH);
                    P8 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, BOX_WIDTH);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                    P1 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD+railWidth/2), -BOX_WIDTH);
                    P2 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD+railWidth/2), BOX_WIDTH);
                    P3 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD-railWidth/2), -BOX_WIDTH);
                    P4 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD-railWidth/2), BOX_WIDTH);
                    P5 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, -BOX_WIDTH);
                    P6 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, BOX_WIDTH);
                    P7 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, -BOX_WIDTH);
                    P8 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, BOX_WIDTH);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);
                }

                if(index)
                {
                    P1 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG1, BOX_WIDTH);
                    P2 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG0, BOX_WIDTH);
                    P3 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG1, BOX_WIDTH);
                    P4 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG0, BOX_WIDTH);
                    P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG0, -BOX_WIDTH);
                    P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG1, -BOX_WIDTH);
                    P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG0, -BOX_WIDTH);
                    P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG1, -BOX_WIDTH);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);
                }
                break;
            case box:
                P1 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), -railSpacing, -BOX_WIDTH);
                P2 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), -railSpacing, BOX_WIDTH);
                P3 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), -railSpacing, -BOX_WIDTH);
                P4 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), -railSpacing, BOX_WIDTH);
                P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), railSpacing, -BOX_WIDTH);
                P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), railSpacing, BOX_WIDTH);
                P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), railSpacing, -BOX_WIDTH);
                P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), railSpacing, BOX_WIDTH);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.6 : 0.8), -railSpacing, -BOX_WIDTH);
                P2 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.6 : 0.8), -railSpacing, BOX_WIDTH);
                P3 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.8 : 0.6), -railSpacing, -BOX_WIDTH);
                P4 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.8 : 0.6), -railSpacing, BOX_WIDTH);
                P5 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.6 : 0.8), railSpacing, -BOX_WIDTH);
                P6 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.6 : 0.8), railSpacing, BOX_WIDTH);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.8 : 0.6), railSpacing, -BOX_WIDTH);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.8 : 0.6), railSpacing, BOX_WIDTH);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), -BOX_WIDTH);
                P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), BOX_WIDTH);
                P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), -BOX_WIDTH);
                P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), BOX_WIDTH);
                P5 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), -BOX_WIDTH);
                P6 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), BOX_WIDTH);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), -BOX_WIDTH);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), BOX_WIDTH);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), -BOX_WIDTH);
                P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), BOX_WIDTH);
                P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), -BOX_WIDTH);
                P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), BOX_WIDTH);
                P5 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), -BOX_WIDTH);
                P6 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), BOX_WIDTH);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), -BOX_WIDTH);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), BOX_WIDTH);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                if(index && index%2)
                {
                    P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                    P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);
                    P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                    P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);
                    P5 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                    P6 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);
                    P7 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                    P8 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                    P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                    P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);
                    P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                    P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);
                    P5 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                    P6 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);
                    P7 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                    P8 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                    P1 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG1, -BOX_WIDTH);
                    P2 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG0, -BOX_WIDTH);
                    P3 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG1, -BOX_WIDTH);
                    P4 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG0, -BOX_WIDTH);
                    P5 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG0, BOX_WIDTH);
                    P6 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG1, BOX_WIDTH);
                    P7 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG0, BOX_WIDTH);
                    P8 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG1, BOX_WIDTH);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                    P1 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG1, -BOX_WIDTH);
                    P2 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG0, -BOX_WIDTH);
                    P3 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG1, -BOX_WIDTH);
                    P4 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG0, -BOX_WIDTH);
                    P5 = lastNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG0, BOX_WIDTH);
                    P6 = lastNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG1, BOX_WIDTH);
                    P7 = lastNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG0, BOX_WIDTH);
                    P8 = lastNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG1, BOX_WIDTH);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);
                }
                else if(index && (index%2 == 0))
                {
                    P1 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                    P2 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);
                    P3 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                    P4 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);
                    P5 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                    P6 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);
                    P7 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                    P8 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                    P1 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                    P2 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);
                    P3 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                    P4 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);
                    P5 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                    P6 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);
                    P7 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                    P8 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                    P1 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG1, BOX_WIDTH);
                    P2 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG0, BOX_WIDTH);
                    P3 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG1, BOX_WIDTH);
                    P4 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG0, BOX_WIDTH);
                    P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG0, -BOX_WIDTH);
                    P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG1, -BOX_WIDTH);
                    P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG0, -BOX_WIDTH);
                    P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG1, -BOX_WIDTH);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                    P1 = lastNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG1, BOX_WIDTH);
                    P2 = lastNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG0, BOX_WIDTH);
                    P3 = lastNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG1, BOX_WIDTH);
                    P4 = lastNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG0, BOX_WIDTH);
                    P5 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG0, -BOX_WIDTH);
                    P6 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG1, -BOX_WIDTH);
                    P7 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG0, -BOX_WIDTH);
                    P8 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG1, -BOX_WIDTH);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);
                }
                break;
            case smallflat:
                P1 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), -railSpacing, -BOX_WIDTH);
                P2 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), -railSpacing, BOX_WIDTH);
                P3 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), -railSpacing, -BOX_WIDTH);
                P4 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), -railSpacing, BOX_WIDTH);
                P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), railSpacing, -BOX_WIDTH);
                P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), railSpacing, BOX_WIDTH);
                P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), railSpacing, -BOX_WIDTH);
                P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), railSpacing, BOX_WIDTH);

                createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);
                break;
            case doublespine:
                if(index%2 == 0)
                {
                    P1 = curNode->vRelPos(-trackData->fHeart+0.75*railWidth*mysign, -railSpacing, -0.12*spineHeight);
                    P2 = curNode->vRelPos(-trackData->fHeart+0.75*railWidth*mysign, -railSpacing, 0.12*spineHeight);
                    P3 = curNode->vRelPos(-trackData->fHeart-0.25*railWidth*mysign, -railSpacing, -0.12*spineHeight);
                    P4 = curNode->vRelPos(-trackData->fHeart-0.25*railWidth*mysign, -railSpacing, 0.12*spineHeight);
                    P5 = curNode->vRelPos(-trackData->fHeart+0.75*railWidth*mysign, railSpacing, -0.12*spineHeight);
                    P6 = curNode->vRelPos(-trackData->fHeart+0.75*railWidth*mysign, railSpacing, 0.12*spineHeight);
                    P7 = curNode->vRelPos(-trackData->fHeart-0.25*railWidth*mysign, railSpacing, -0.12*spineHeight);
                    P8 = curNode->vRelPos(-trackData->fHeart-0.25*railWidth*mysign, railSpacing, 0.12*spineHeight);
                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                    P1 = curNode->vRelPos(-trackData->fHeart+0.7*railWidth*mysign, -0.80*railSpacing, -0.10*spineHeight);
                    P2 = curNode->vRelPos(-trackData->fHeart+0.7*railWidth*mysign, -0.80*railSpacing, 0.10*spineHeight);
                    P3 = curNode->vRelPos(-trackData->fHeart-0.2*railWidth*mysign, -0.80*railSpacing, -0.10*spineHeight);
                    P4 = curNode->vRelPos(-trackData->fHeart-0.2*railWidth*mysign, -0.80*railSpacing, 0.10*spineHeight);
                    P5 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.7, 0, -0.20*spineHeight);
                    P6 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.7, 0, 0.20*spineHeight);
                    P7 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, -0.10*spineHeight);
                    P8 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, 0.10*spineHeight);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);

                    P1 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.7, 0, -0.20*spineHeight);
                    P2 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.7, 0, 0.20*spineHeight);
                    P3 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, -0.10*spineHeight);
                    P4 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, 0.10*spineHeight);
                    P5 = curNode->vRelPos(-trackData->fHeart+0.7*railWidth*mysign, 0.80*railSpacing, -0.10*spineHeight);
                    P6 = curNode->vRelPos(-trackData->fHeart+0.7*railWidth*mysign, 0.80*railSpacing, 0.10*spineHeight);
                    P7 = curNode->vRelPos(-trackData->fHeart-0.2*railWidth*mysign, 0.80*railSpacing, -0.10*spineHeight);
                    P8 = curNode->vRelPos(-trackData->fHeart-0.2*railWidth*mysign, 0.80*railSpacing, 0.10*spineHeight);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);
                }
                if((index+3)%4 == 0)
                {
                    P1 = curNode->vRelPos(-trackData->fHeart-spineHeight, 0.65*spineSize, -0.65*spineSize);
                    P2 = curNode->vRelPos(-trackData->fHeart-spineHeight, 0.65*spineSize, 0.65*spineSize);
                    P3 = curNode->vRelPos(-trackData->fHeart-spineHeight, -0.65*spineSize, -0.65*spineSize);
                    P4 = curNode->vRelPos(-trackData->fHeart-spineHeight, -0.65*spineSize, 0.65*spineSize);
                    P5 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.4, 0.65*spineSize, -0.65*spineSize);
                    P6 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.4, 0.65*spineSize, 0.65*spineSize);
                    P7 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.4, -0.65*spineSize, -0.65*spineSize);
                    P8 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.4, -0.65*spineSize, 0.65*spineSize);

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);
                }
                if((index+1)%4 == 0)
                {
                    P1 = curNode->vRelPos(-trackData->fHeart-spineHeight, 0.65*spineSize, -0.65*spineSize);
                    P2 = P1;
                    P3 = P1;
                    P4 = P1;
                    P5 = P1;
                    P6 = P1;
                    P7 = P1;
                    P8 = P1;

                    createBox(crossties, P1, P2, P3, P4, P5, P6, P7, P8);
                    createShadowBox(crosstieshadows, P1, P2, P3, P4, P5, P6, P7, P8);
                }
                break;
            }
        }
        mSec = timer.nsecsElapsed()/1000000.;
        gloParent->showMessage(QString::number(mSec).append(QString("ms used to build meshes")), 3000);
    }
    else // wireframe
    {
        int railNode;

        if(rails.size() && fromNode >= rails.last().node)
        {
            fromNode = rails.last().node-5;
        }

        while(nodeList.size() && nodeList.last() >= fromNode) nodeList.removeLast();

        for(railNode = 0; railNode < rails.size() && rails[railNode].node < fromNode; ++railNode);
        rails.remove(railNode, rails.size()-railNode);

        railNode = rails.size() ? rails.last().node : 0;

        int i;

        trackData->getSecNode(railNode, &j, &i);

        if(i < 0) i = 0;
        if(i == j && j == 0) distFromLastNode = 1.f;

        for(; i < trackData->lSections.size(); i++)
        {
            curSection = trackData->lSections[i];
            for(; j < curSection->lNodes.size(); ++j)
            {
                if(i != 0 && j == 1) distFromLastNode = 1.f;
                float angle = curSection->lNodes.at(j)->fFlexion();
                angle /= angleNodeDist;
                angle = std::min(std::max(1.f/minNodeDist, angle), 1.f/maxNodeDist);
                angle *= curSection->lNodes.at(j)->fDistFromLast;;
                distFromLastNode += angle;
                if(distFromLastNode >= 1.f || j == curSection->lNodes.size()-1)
                {
                    if(distFromLastNode >= 1.f)
                    {
                        distFromLastNode -= 1.f;
                    }
                    else
                    {
                        distFromLastNode = 0.f;
                    }

                    posList.append(j);
                    secList.append(i);
                }
            }
            for(j = 0; j < curSection->supList.size(); j+=2)
            {
                glm::vec3 P1 = curSection->supList[j];
                glm::vec3 P2 = curSection->supList[j+1];

                nextNorm = glm::vec3(0, 0.5, 0);
                nextNode = 0;
                nextPos = P1;
                appendSupportNode(rendersupports);
                nextPos = P2;
                appendSupportNode(rendersupports);
                //createSupport(rendersupports, 12, 0.2f, 0.2f, P1, P2, true);
                supportsSize += 1;

                //supportsSize += 3;
                //nextPos = curSection->supList[j];
                //appendMeshNode(supports);
            }
        }

        for(railNode = 0; railNode < heartline.size() && heartline[railNode].node < fromNode; ++railNode);
        heartline.remove(railNode, heartline.size()-railNode);

        for(int i = 0; i < posList.size(); ++i)
        {
            j = posList[i];
            section* curSection = trackData->lSections[secList[i]];
            curNode = curSection->lNodes[j];

            heartlineSize += 1;

            nextPos = curNode->vPos;
            nextNode = trackData->getNumPoints(curSection) + j;
            if(!heartline.size() || nextNode != heartline.last().node) appendMeshNode(heartline);
            if(nodeList.isEmpty() || (nodeList.size() && nodeList.last() != nextNode)) nodeList.append(nextNode);
        }


        /* RAILS*/

        for(int i = 0; i < posList.size(); ++i)
        {
            j = posList[i];
            curSection = trackData->lSections[secList[i]];
            curNode = curSection->lNodes[j];

            nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing);
            nextNorm = glm::vec3(0, 0.5, 0);
            nextNode = trackData->getNumPoints(curSection) + j;

            //nextPos.y = -trackData->startPos.y;
            appendTrackNode(rails);
            nextPos = curNode->vRelPos(-trackData->fHeart, railSpacing);
            //nextPos.y = -trackData->startPos.y;
            appendTrackNode(rails);

            switch(trackData->style)
            {
            case generic:
            case vekoma:
            case bm:
            case triangle:
                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, 0);
                //nextPos.y = -trackData->startPos.y;
                appendTrackNode(rails);
                break;
            case box:
                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, -railSpacing);
                //nextPos.y = -trackData->startPos.y;
                appendTrackNode(rails);
                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, railSpacing);
                //nextPos.y = -trackData->startPos.y;
                appendTrackNode(rails);
                break;
            case genericflat:
            case smallflat:
                break;
            case doublespine:
                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, 0);
                appendTrackNode(rails);
                nextPos = curNode->vRelPos(-trackData->fHeart-0.45f-spineHeight, 0);
                appendTrackNode(rails);
                break;
            }
        }

        /* CROSSTIE GENERATION */

        int iCrosstie = 0, iCrossShadow = 0;
        while(crossties.size() > iCrosstie && fromNode > crossties[iCrosstie].node) iCrosstie++; // cycle through crossties until we have something to change
        while(crosstieshadows.size() > iCrossShadow && fromNode > crosstieshadows[iCrossShadow].node) iCrossShadow++;

        if(iCrosstie == 0)  distFromLastNode = crosstieSpacing/2.f;
        else distFromLastNode = trackData->getPoint(fromNode)->fTotalLength - trackData->getPoint(crossties[iCrosstie-1].node)->fTotalLength;

        crossties.remove(iCrosstie, crossties.size()-iCrosstie);
        crosstieshadows.remove(iCrossShadow, crosstieshadows.size()-iCrossShadow);


        curNode = NULL;

        posList.clear();
        secList.clear();

        j = fromSecI;
        for(int i = fromSection; i < trackData->lSections.size(); i++)
        {
            section* curSection = trackData->lSections[i];
            for(; j < curSection->lNodes.size(); ++j)
            {
                distFromLastNode += curSection->lNodes.at(j)->fDistFromLast;
                if(distFromLastNode >= crosstieSpacing)
                {
                    distFromLastNode -= crosstieSpacing;
                    posList.append(j);
                    secList.append(i);
                }
            }
            j = 0;
        }

        int offset = 0;
        switch(trackData->style)
        {
        case generic:
            offset = crossties.size()/4;
            break;
        case genericflat:
            offset = (crossties.size()+2)/4;
            break;
        case vekoma:
            offset = crossties.size()/10;
            break;
        case bm:
            offset = crossties.size()/6;
            break;
        case triangle:
            offset = (crossties.size()+6)/12;
            break;
        case box:
            offset = (crossties.size()+8)/16;
            break;
        case smallflat:
            offset = crossties.size()/2;
            break;
        case doublespine:
            offset = 2*(crossties.size()/(4*2)) + ((crossties.size()/2)%4)/3;
            break;
        }

        if(crossties.size()) curNode = trackData->getPoint(crossties.last().node);

        for(int i = 0; i < posList.size(); ++i)
        {
            int index = i+offset;
            j = posList[i];
            curSection = trackData->lSections[secList[i]];
            lastNode = curNode;
            curNode = curSection->lNodes[j];

            nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing);
            nextNorm = glm::vec3(0, 0.5, 0);
            nextNode = trackData->getNumPoints(curSection) + j;

            switch(trackData->style)
            {
            case generic:
                nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, 0);
                appendTrackNode(crossties);

                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, 0);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart, railSpacing);
                appendTrackNode(crossties);
                break;
            case genericflat:
                nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart, railSpacing);
                appendTrackNode(crossties);

                if(index)
                {
                    nextPos = lastNode->vRelPos(-trackData->fHeart, -railSpacing+0.1f);
                    appendTrackNode(crossties);
                    nextPos = curNode->vRelPos(-trackData->fHeart, railSpacing-0.1f);
                    appendTrackNode(crossties);
                }
                break;
            case vekoma:
                nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing-0.1f);
                appendTrackNode(crossties);

                nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing-0.1f);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, -railSpacing-0.1f);
                appendTrackNode(crossties);

                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, -railSpacing-0.1f);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, railSpacing+0.1f);
                appendTrackNode(crossties);

                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, railSpacing+0.1f);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart, railSpacing+0.1f);
                appendTrackNode(crossties);

                nextPos = curNode->vRelPos(-trackData->fHeart, railSpacing+0.1f);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart, railSpacing);
                appendTrackNode(crossties);
                break;
            case bm:
                nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, -railSpacing/3);
                appendTrackNode(crossties);

                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, -railSpacing/3);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, railSpacing/3);
                appendTrackNode(crossties);

                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, railSpacing/3);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart, railSpacing);
                appendTrackNode(crossties);
                break;
            case triangle:
                nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing+BOX_INWARD);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, 0);
                appendTrackNode(crossties);

                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, 0);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart, railSpacing-BOX_INWARD);
                appendTrackNode(crossties);

                nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart, railSpacing);
                appendTrackNode(crossties);

                if(index && index%2)
                {
                    nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing+BOX_INWARD);
                    appendTrackNode(crossties);
                    nextPos = lastNode->vRelPos(-trackData->fHeart-spineHeight, 0);
                    appendTrackNode(crossties);

                    nextPos = curNode->vRelPos(-trackData->fHeart, +railSpacing-BOX_INWARD);
                    appendTrackNode(crossties);
                    nextPos = lastNode->vRelPos(-trackData->fHeart-spineHeight, 0);
                    appendTrackNode(crossties);
                }
                else if(index && index%2 == 0)
                {
                    nextPos = lastNode->vRelPos(-trackData->fHeart, -railSpacing+BOX_INWARD);
                    appendTrackNode(crossties);
                    nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, 0);
                    appendTrackNode(crossties);

                    nextPos = lastNode->vRelPos(-trackData->fHeart, +railSpacing-BOX_INWARD);
                    appendTrackNode(crossties);
                    nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, 0);
                    appendTrackNode(crossties);
                }

                if(index)
                {
                    nextPos = lastNode->vRelPos(-trackData->fHeart, -railSpacing+BOX_INWARD);
                    appendTrackNode(crossties);
                    nextPos = curNode->vRelPos(-trackData->fHeart, +railSpacing-BOX_INWARD);
                    appendTrackNode(crossties);
                }
                break;
            case box:
                nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing+BOX_INWARD);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, -railSpacing+BOX_INWARD);
                appendTrackNode(crossties);

                nextPos = curNode->vRelPos(-trackData->fHeart, railSpacing-BOX_INWARD);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, railSpacing-BOX_INWARD);
                appendTrackNode(crossties);

                nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart, railSpacing);
                appendTrackNode(crossties);

                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, -railSpacing);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, railSpacing);
                appendTrackNode(crossties);

                if(index && index%2)
                {
                    nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing+BOX_INWARD);
                    appendTrackNode(crossties);
                    nextPos = lastNode->vRelPos(-trackData->fHeart-spineHeight, -railSpacing+BOX_INWARD);
                    appendTrackNode(crossties);

                    nextPos = curNode->vRelPos(-trackData->fHeart, +railSpacing-BOX_INWARD);
                    appendTrackNode(crossties);
                    nextPos = lastNode->vRelPos(-trackData->fHeart-spineHeight, +railSpacing-BOX_INWARD);
                    appendTrackNode(crossties);

                    nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing+BOX_INWARD);
                    appendTrackNode(crossties);
                    nextPos = lastNode->vRelPos(-trackData->fHeart, +railSpacing-BOX_INWARD);
                    appendTrackNode(crossties);

                    nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, -railSpacing+BOX_INWARD);
                    appendTrackNode(crossties);
                    nextPos = lastNode->vRelPos(-trackData->fHeart-spineHeight, +railSpacing-BOX_INWARD);
                    appendTrackNode(crossties);
                }
                else if(index && index%2 == 0)
                {
                    nextPos = lastNode->vRelPos(-trackData->fHeart, -railSpacing+BOX_INWARD);
                    appendTrackNode(crossties);
                    nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, -railSpacing+BOX_INWARD);
                    appendTrackNode(crossties);

                    nextPos = lastNode->vRelPos(-trackData->fHeart, +railSpacing-BOX_INWARD);
                    appendTrackNode(crossties);
                    nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, +railSpacing-BOX_INWARD);
                    appendTrackNode(crossties);

                    nextPos = curNode->vRelPos(-trackData->fHeart, +railSpacing-BOX_INWARD);
                    appendTrackNode(crossties);
                    nextPos = lastNode->vRelPos(-trackData->fHeart, -railSpacing+BOX_INWARD);
                    appendTrackNode(crossties);

                    nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, +railSpacing-BOX_INWARD);
                    appendTrackNode(crossties);
                    nextPos = lastNode->vRelPos(-trackData->fHeart-spineHeight, -railSpacing+BOX_INWARD);
                    appendTrackNode(crossties);
                }
                break;
            case smallflat:
                nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing);
                appendTrackNode(crossties);
                nextPos = curNode->vRelPos(-trackData->fHeart, railSpacing);
                appendTrackNode(crossties);
                break;
            case doublespine:
                if(index%2 == 0)
                {
                    nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing);
                    appendTrackNode(crossties);
                    nextPos = curNode->vRelPos(-trackData->fHeart, railSpacing, 0.f);
                    appendTrackNode(crossties);

                    nextPos = curNode->vRelPos(-trackData->fHeart, -railSpacing);
                    appendTrackNode(crossties);
                    nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, 0.f);
                    appendTrackNode(crossties);

                    nextPos = curNode->vRelPos(-trackData->fHeart, railSpacing);
                    appendTrackNode(crossties);
                    nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, 0.f);
                    appendTrackNode(crossties);
                }
                if((index+3)%4 == 0)
                {
                    nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, 0.f);
                    appendTrackNode(crossties);
                    nextPos = curNode->vRelPos(-trackData->fHeart-0.45f-spineHeight, 0.f);
                    appendTrackNode(crossties);
                }
                if((index+1)%4 == 0)
                {
                    nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, 0.f);
                    appendTrackNode(crossties);
                    nextPos = curNode->vRelPos(-trackData->fHeart-spineHeight, 0.f);
                    appendTrackNode(crossties);
                }
                break;
            }
        }
    }
    createIndices();
    updateVertexArrays();
    return;
}

void trackMesh::build3ds(const int _sec, QVector<float> *_vertices, QVector<unsigned int> *_indices, QVector<unsigned int> *_borders)
{
    posList.clear();
    secList.clear();

    _borders->append(_vertices->size());
    _borders->append(_indices->size());

    float crosstieSpacing = 0.f;
    float meshQuality;
    switch(gloParent->mOptions->meshQuality)
    {
    case 0:
        meshQuality = 1;
        break;
    case 1:
        meshQuality = 2;
        break;
    case 2:
        meshQuality = 4;
        break;
    case 3:
        meshQuality = 6;
        break;
    }
    meshQuality = 2;

    float minNodeDist = 12.f/(meshQuality), maxNodeDist = 0.3f/(meshQuality);    // minimal and maximal distance between nodes
    float angleNodeDist = 6.f/(meshQuality);                     // after x degrees difference force a new node

    float railSpacing = 0.f;
    float distFromLastNode = 0.f;
    float spineHeight = (trackData->fHeart < 0 ? -1.f : 1.f);
    float spineSize = 0.06f;

    float railWidth = 0.065f;


    mnode *lastNode = NULL;
    curNode = NULL;

    switch(trackData->style)
    {
    case generic:
        numRails = 3;
        railSpacing = 0.5f;
        crosstieSpacing = 1.5f;
        spineHeight = 0.30f * (trackData->fHeart < 0 ? -1.f : 1.f);
        spineSize = 0.22f; // used to be 0.18f
        break;
    case genericflat:
        numRails = 2;
        railSpacing = 0.7f;
        crosstieSpacing = 1.4f;
        break;
    case vekoma:
        numRails = 3;
        railSpacing = 0.60;
        crosstieSpacing = 1.5f;
        spineHeight = 0.85f * (trackData->fHeart < 0 ? -1.f : 1.f);
        spineSize = 0.22f;
        break;
    case bm:
        numRails = 3;
        railSpacing = 0.6f;
        crosstieSpacing = 1.45f;
        spineHeight = 0.5f * (trackData->fHeart < 0 ? -1.f : 1.f);
        spineSize = 0.26f*1.4101f;
        break;
    case triangle:
        numRails = 3;
        railSpacing = 0.5f;
        crosstieSpacing = 1.0f;
        spineSize = railWidth;
        spineHeight = 0.75f * (trackData->fHeart < 0 ? -1.f : 1.f);
        break;
    case box:
        numRails = 4;
        railSpacing = 0.5f;
        crosstieSpacing = 1.0f;
        spineSize = railWidth;
        spineHeight = 1.f * (trackData->fHeart < 0 ? -1.f : 1.f);
        break;
    case smallflat:
        numRails = 2;
        railSpacing = 0.5f;
        crosstieSpacing = 0.8f;
        break;
    case doublespine:
        numRails = 4;
        railSpacing = 0.5f;
        crosstieSpacing = 0.3f;
        spineHeight = 0.3f * (trackData->fHeart < 0 ? -1.f : 1.f);
        spineSize = 0.18f;
        break;
    }


    curSection = trackData->lSections[_sec];
    for(j = 0; j < curSection->lNodes.size(); ++j)
    {
        if(j == 0) distFromLastNode = 1.f;
        float angle = curSection->lNodes.at(j)->fFlexion();
        angle /= angleNodeDist;
        angle = std::min(std::max(1.f/minNodeDist, angle), 1.f/maxNodeDist);
        angle *= curSection->lNodes.at(j)->fDistFromLast;;
        distFromLastNode += angle;
        if(distFromLastNode >= 1.f || j == curSection->lNodes.size()-1)
        {
            if(distFromLastNode >= 1.f)
            {
                distFromLastNode -= 1.f;
            }
            else
            {
                distFromLastNode = 0.f;
            }
            posList.append(j);
            secList.append(_sec);
        }
    }


    int jSize = posList.size();

    options.clear();
    pipeoption_t temp;
    temp.edges = 6;
    temp.offset = glm::vec2(-railSpacing, -trackData->fHeart);
    temp.radius = glm::vec2(railWidth, railWidth);
    temp.smooth = true;
    options.append(temp);
    temp.offset.x *= -1;
    options.append(temp);

    switch(trackData->style)
    {
    case bm:
        temp.edges=4;
        temp.offset.x = 0;
        temp.offset.y = -trackData->fHeart-spineHeight;
        temp.radius.x = spineSize;
        temp.radius.y = spineSize;
        options.append(temp);
        //createPipe(rails, 4, spineSize*1.2, spineSize, -trackData->fHeart-spineHeight, 0, false);
        //createPipe(rails, 4, 0, 0, -trackData->fHeart-spineHeight, 0, false);
        //createPipe(rails, 4, 0, 0, -trackData->fHeart-spineHeight, 0, false);
        break;
    case generic:
    case vekoma:
        temp.edges = 12;
    case triangle:
        temp.offset.x = 0;
        temp.offset.y = -trackData->fHeart-spineHeight;
        temp.radius.x = spineSize;
        temp.radius.y = spineSize;
        options.append(temp);
        //createPipe(rails, 12, spineSize, spineSize, -trackData->fHeart-spineHeight, 0);
        break;
    case box:
        temp.offset.x = -railSpacing;
        temp.offset.y = -trackData->fHeart-spineHeight;
        temp.radius.x = spineSize;
        temp.radius.y = spineSize;
        options.append(temp);
        temp.offset.x = railSpacing;
        options.append(temp);
        //createPipe(rails, 12, spineSize, spineSize, -trackData->fHeart-spineHeight, -railSpacing);
        //createPipe(rails, 12, spineSize, spineSize, -trackData->fHeart-spineHeight, railSpacing);
        break;
    case genericflat:
    case smallflat:
        break;
    case doublespine:
        temp.edges = 12;
        temp.offset.x = 0;
        temp.offset.y = -trackData->fHeart-spineHeight;
        temp.radius.x = spineSize;
        temp.radius.y = spineSize;
        options.append(temp);
        temp.offset.y = -trackData->fHeart-0.45f-spineHeight;
        options.append(temp);
    }

    create3dsPipes(_vertices, options);


    int edgeCount = 0;
    for(int i = 0; i < options.size(); ++i) edgeCount += options[i].edges;

    QList<int> renderList;
    for(int i = 0; i < posList.size(); ++i)
    {
        renderList.append(i);
    }
    int nodeCount = posList.size();

    QList<int> points;
    int vBorder = 0;

    for(int p = 0; p < options.size(); ++p)
    {
        points.clear();
        int offset = 0;
        int edgeCnt = options[p].edges;
        vBorder += edgeCnt*nodeCount+2;
        for(int i = 0; i < p; ++i)
        {
            offset += options[i].edges*nodeCount+2;
        }
        points.append(offset);
        for(int e = 0; e < edgeCnt; e+=2)
        {
            int e2 = (e+1)%edgeCnt;
            int e3 = (e+2)%edgeCnt;
            int i;
            for(i = 0; i < nodeCount; ++i)
            {
                points.append(offset + 1 + i*edgeCnt + e);
                points.append(offset + 1 + i*edgeCnt + e2);
            }
            points.append(offset + 1 + i*edgeCnt);
            for(i = nodeCount-1; i >= 0; --i)
            {
                points.append(offset + 1 + i*edgeCnt + e2);
                points.append(offset + 1 + i*edgeCnt + e3);
            }
            points.append(offset);
        }

        for(int i = 0; i < points.size()-2; ++i)
        {
            if(i%2)
            {
                _indices->append(points[i]);
                _indices->append(points[i+1]);
                _indices->append(points[i+2]);
            }
            else
            {
                _indices->append(points[i]);
                _indices->append(points[i+2]);
                _indices->append(points[i+1]);
            }
        }
        _borders->append(vBorder*3);
        _borders->append(_indices->size());
    }

    // crossties

    curNode = NULL;

    posList.clear();
    secList.clear();


    curSection = trackData->lSections[_sec];
    for(j=0; j < curSection->lNodes.size(); ++j)
    {
        distFromLastNode += curSection->lNodes.at(j)->fDistFromLast;
        if(distFromLastNode >= crosstieSpacing)
        {
            distFromLastNode -= crosstieSpacing;
            posList.append(j);
            secList.append(_sec);
        }
    }


    jSize = posList.size();

#define XS (0.06f)
#define S (0.08f)
#define M (0.10f)
#define L (0.12f)
#define XL (0.14f)
#define XXL (0.16f)

#define BOX_INWARD (0.14f)
#define BOX_DIAG0 (0.08f)
#define BOX_DIAG1 (0.18f)
#define BOX_WIDTH (0.05f)


    for(int i = 0; i < jSize; ++i)
    {
        int index = i;
        j = posList[i];
        curSection = trackData->lSections[secList[i]];
        lastNode = curNode;
        curNode = curSection->lNodes[j];
        nextNode = trackData->getNumPoints(curSection) + j;

        glm::vec3 P1, P2, P3, P4, P5, P6, P7, P8;
        float mysign = fabs(spineHeight)/spineHeight;
        switch(trackData->style)
        {
        case generic:
            P1 = curNode->vRelPos(-trackData->fHeart+0.1*railWidth*mysign, -railSpacing, -0.15*spineHeight);
            P2 = curNode->vRelPos(-trackData->fHeart+0.1*railWidth*mysign, -railSpacing, 0.15*spineHeight);
            P3 = curNode->vRelPos(-trackData->fHeart, -railSpacing, -0.15*spineHeight);
            P4 = curNode->vRelPos(-trackData->fHeart, -railSpacing, 0.15*spineHeight);
            P5 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.15, 0, -0.15f);//-0.35*spineHeight);
            P6 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.15, 0, 0.15f);//0.35*spineHeight);
            P7 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, -0.15f);// -0.15*spineHeight);
            P8 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, 0.15f);// 0.15*spineHeight);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            P1 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.15, 0, -0.15f);//-0.35*spineHeight);
            P2 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.15, 0, 0.15f);//0.35*spineHeight);
            P3 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, -0.15f);//, -0.15*spineHeight);
            P4 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, 0.15f);//, 0.15*spineHeight);
            P5 = curNode->vRelPos(-trackData->fHeart+0.1*railWidth*mysign, railSpacing, -0.15*spineHeight);
            P6 = curNode->vRelPos(-trackData->fHeart+0.1*railWidth*mysign, railSpacing, 0.15*spineHeight);
            P7 = curNode->vRelPos(-trackData->fHeart, railSpacing, -0.15*spineHeight);
            P8 = curNode->vRelPos(-trackData->fHeart, railSpacing, 0.15*spineHeight);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);
            break;
        case genericflat:
            if(index%6 == 0)
            {
                P1 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.8 : 1.0), -railSpacing, -M);
                P2 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.8 : 1.0), -railSpacing, M);
                P3 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1.0 : 0.8), -railSpacing, -M);
                P4 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1.0 : 0.8), -railSpacing, M);
                P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.8 : 1.0), railSpacing, -M);
                P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.8 : 1.0), railSpacing, M);
                P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1.0 : 0.8), railSpacing, -M);
                P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1.0 : 0.8), railSpacing, M);

            }
            else
            {
                P1 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), -railSpacing, -0.03);
                P2 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), -railSpacing, 0.03);
                P3 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), -railSpacing, -0.03);
                P4 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), -railSpacing, 0.03);
                P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), railSpacing, -0.03);
                P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), railSpacing, 0.03);
                P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), railSpacing, -0.03);
                P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), railSpacing, 0.03);
            }

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            if(index)
            {
                P1 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), -railSpacing+0.13, 0.03);
                P2 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), -railSpacing+0.07, 0.03);
                P3 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), -railSpacing+0.13, 0.03);
                P4 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), -railSpacing+0.07, 0.03);
                P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), railSpacing-0.07, -0.03);
                P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), railSpacing-0.13, -0.03);
                P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), railSpacing-0.07, -0.03);
                P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), railSpacing-0.13, -0.03);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);
            }
            break;
        case vekoma:
            P1 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, -railSpacing-0.5*railWidth, -0.05f);
            P2 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, -railSpacing-0.5*railWidth, +0.05f);
            P3 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -railSpacing-0.5*railWidth, -0.05f);
            P4 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -railSpacing-0.5*railWidth, +0.05f);
            P5 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, -1.2*railSpacing, -0.05f);
            P6 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, -1.2*railSpacing, +0.05f);
            P7 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -1.2*railSpacing, -0.05f);
            P8 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -1.2*railSpacing, +0.05f);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            P1 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, railSpacing+0.5*railWidth, -0.05f);
            P2 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, railSpacing+0.5*railWidth, +0.05f);
            P3 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, railSpacing+0.5*railWidth, -0.05f);
            P4 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, railSpacing+0.5*railWidth, +0.05f);
            P5 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, 1.2*railSpacing, -0.05f);
            P6 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, 1.2*railSpacing, +0.05f);
            P7 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, 1.2*railSpacing, -0.05f);
            P8 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, 1.2*railSpacing, +0.05f);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);


            P1 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -1.2*railSpacing, -0.05f);
            P2 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -1.2*railSpacing, +0.05f);
            P3 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, -1.3*railSpacing, -0.05f);
            P4 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, -1.3*railSpacing, +0.05f);
            P5 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, -1.2*railSpacing, -0.05f);
            P6 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, -1.2*railSpacing, +0.05f);
            P7 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, -1.3*railSpacing, -0.05f);
            P8 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, -1.3*railSpacing, +0.05f);

            create3dsQuad(_vertices, _indices, P2, P1, P3, P4);
            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            P1 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, 1.3*railSpacing, -0.05f);
            P2 = curNode->vRelPos(-trackData->fHeart-0.7*railWidth*mysign, 1.3*railSpacing, +0.05f);
            P3 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, 1.2*railSpacing, -0.05f);
            P4 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, 1.2*railSpacing, +0.05f);
            P5 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, 1.3*railSpacing, -0.05f);
            P6 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, 1.3*railSpacing, +0.05f);
            P7 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, 1.2*railSpacing, -0.05f);
            P8 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, 1.2*railSpacing, +0.05f);

            create3dsQuad(_vertices, _indices, P2, P1, P3, P4);
            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            P1 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, -1.3*railSpacing, -0.05f);
            P2 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, -1.3*railSpacing, +0.05f);
            P3 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, -1.3*railSpacing, -0.05f);
            P4 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, -1.3*railSpacing, +0.05f);
            P5 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, 1.3*railSpacing, -0.05f);
            P6 = curNode->vRelPos(-trackData->fHeart-0.45*mysign, 1.3*railSpacing, +0.05f);
            P7 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, 1.3*railSpacing, -0.05f);
            P8 = curNode->vRelPos(-trackData->fHeart-0.55*mysign, 1.3*railSpacing, +0.05f);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            P1 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, -0.9*railSpacing, -0.07f);
            P2 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, -0.9*railSpacing, +0.07f);
            P3 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, -0.9*railSpacing, -0.07f);
            P4 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, -0.9*railSpacing, +0.07f);
            P5 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, -1.1*spineSize, -0.07f);
            P6 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, -1.1*spineSize, +0.07f);
            P7 = curNode->vRelPos(-trackData->fHeart-0.9*spineHeight-0.5*mysign*(spineSize-0.05), -1.1*spineSize, -0.07f);
            P8 = curNode->vRelPos(-trackData->fHeart-0.9*spineHeight-0.5*mysign*(spineSize-0.05), -1.1*spineSize, +0.07f);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            P1 = P5;
            P2 = P6;
            P3 = P7;
            P4 = P8;
            P5 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, 1.1*spineSize, -0.07f);
            P6 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, 1.1*spineSize, +0.07f);
            P7 = curNode->vRelPos(-trackData->fHeart-0.9*spineHeight-0.5*mysign*(spineSize-0.05), 1.1*spineSize, -0.07f);
            P8 = curNode->vRelPos(-trackData->fHeart-0.9*spineHeight-0.5*mysign*(spineSize-0.05), 1.1*spineSize, +0.07f);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            P1 = P5;
            P2 = P6;
            P3 = P7;
            P4 = P8;
            P5 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, 0.9*railSpacing, -0.07f);
            P6 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, 0.9*railSpacing, +0.07f);
            P7 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, 0.9*railSpacing, -0.07f);
            P8 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, 0.9*railSpacing, +0.07f);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);


            P1 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, 0.9*railSpacing, -0.07f);
            P2 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, -0.9*railSpacing, -0.07f);
            P3 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, 0.9*railSpacing, -0.07f);
            P4 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, -0.9*railSpacing, -0.07f);
            P5 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, 0.9*railSpacing, +0.07f);
            P6 = curNode->vRelPos(-trackData->fHeart-0.48f*mysign, -0.9*railSpacing, +0.07f);
            P7 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, 0.9*railSpacing, +0.07f);
            P8 = curNode->vRelPos(-trackData->fHeart-0.58f*mysign, -0.9*railSpacing, +0.07f);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);
            break;
        case bm:
            P1 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -railSpacing, -0.05f);
            P2 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, -railSpacing, 0.05f);
            P3 = curNode->vRelPos(-trackData->fHeart, -railSpacing, -0.05f);
            P4 = curNode->vRelPos(-trackData->fHeart, -railSpacing, 0.05f);
            P5 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, -0.3*spineSize, -0.05f);
            P6 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, -0.3*spineSize, 0.05f);
            P7 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, -0.71*spineSize, -0.05f);
            P8 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, -0.71*spineSize, 0.05f);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            if(index%6 == 0)
            {
                P1 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, -0.71*spineSize*1.3, -0.05f);
                P2 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, -0.71*spineSize*1.3, 0.05f);
                P3 = curNode->vRelPos(-trackData->fHeart-spineHeight-1.3*0.71*spineSize*mysign, -0.71*spineSize*1.3, -0.05f);
                P4 = curNode->vRelPos(-trackData->fHeart-spineHeight-1.3*0.71*spineSize*mysign, -0.71*spineSize*1.3, 0.05f);
                P5 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, 0.71*spineSize*1.3, -0.05f);
                P6 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, 0.71*spineSize*1.3, 0.05f);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight-1.3*0.71*spineSize*mysign, 0.71*spineSize*1.3, -0.05f);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight-1.3*0.71*spineSize*mysign, 0.71*spineSize*1.3, 0.05f);

                create3dsQuad(_vertices, _indices, P2, P1, P3, P4);
                create3dsQuad(_vertices, _indices, P5, P6, P8, P7);
            }
            else
            {
                P1 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, -0.71*spineSize, -0.05f);
                P2 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, -0.71*spineSize, 0.05f);
                P3 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, -0.71*spineSize, -0.05f);
                P4 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, -0.71*spineSize, 0.05f);
                P5 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, 0.71*spineSize, -0.05f);
                P6 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, 0.71*spineSize, 0.05f);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, 0.71*spineSize, -0.05f);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, 0.71*spineSize, 0.05f);
            }

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            P1 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, 0.3*spineSize, -0.05f);
            P2 = curNode->vRelPos(-trackData->fHeart-mysign*0.1f, 0.3*spineSize, 0.05f);
            P3 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, 0.71*spineSize, -0.05f);
            P4 = curNode->vRelPos(-trackData->fHeart-spineHeight+0.25*0.71*spineSize*mysign, 0.71*spineSize, 0.05f);
            P5 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, railSpacing, -0.05f);
            P6 = curNode->vRelPos(-trackData->fHeart+0.5*railWidth*mysign, railSpacing, 0.05f);
            P7 = curNode->vRelPos(-trackData->fHeart, railSpacing, -0.05f);
            P8 = curNode->vRelPos(-trackData->fHeart, railSpacing, 0.05f);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);
            break;
        case triangle:
            P1 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), -railSpacing, -BOX_WIDTH);
            P2 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), -railSpacing, BOX_WIDTH);
            P3 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), -railSpacing, -BOX_WIDTH);
            P4 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), -railSpacing, BOX_WIDTH);
            P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), railSpacing, -BOX_WIDTH);
            P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), railSpacing, BOX_WIDTH);
            P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), railSpacing, -BOX_WIDTH);
            P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), railSpacing, BOX_WIDTH);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD+railWidth/2), -BOX_WIDTH);
            P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD+railWidth/2), BOX_WIDTH);
            P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD-railWidth/2), -BOX_WIDTH);
            P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD-railWidth/2), BOX_WIDTH);
            P5 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, -BOX_WIDTH);
            P6 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, BOX_WIDTH);
            P7 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, -BOX_WIDTH);
            P8 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, BOX_WIDTH);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD+railWidth/2), -BOX_WIDTH);
            P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD+railWidth/2), BOX_WIDTH);
            P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD-railWidth/2), -BOX_WIDTH);
            P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD-railWidth/2), BOX_WIDTH);
            P5 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, -BOX_WIDTH);
            P6 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, BOX_WIDTH);
            P7 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, -BOX_WIDTH);
            P8 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, BOX_WIDTH);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            if(index && index%2)
            {
                P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD+railWidth/2), -BOX_WIDTH);
                P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD+railWidth/2), BOX_WIDTH);
                P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD-railWidth/2), -BOX_WIDTH);
                P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD-railWidth/2), BOX_WIDTH);
                P5 = lastNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, -BOX_WIDTH);
                P6 = lastNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, BOX_WIDTH);
                P7 = lastNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, -BOX_WIDTH);
                P8 = lastNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, BOX_WIDTH);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD+railWidth/2), -BOX_WIDTH);
                P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD+railWidth/2), BOX_WIDTH);
                P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD-railWidth/2), -BOX_WIDTH);
                P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD-railWidth/2), BOX_WIDTH);
                P5 = lastNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, -BOX_WIDTH);
                P6 = lastNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, BOX_WIDTH);
                P7 = lastNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, -BOX_WIDTH);
                P8 = lastNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, BOX_WIDTH);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);
            }
            else if(index && index%2 == 0)
            {
                P1 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD+railWidth/2), -BOX_WIDTH);
                P2 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD+railWidth/2), BOX_WIDTH);
                P3 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD-railWidth/2), -BOX_WIDTH);
                P4 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(-railSpacing+BOX_INWARD-railWidth/2), BOX_WIDTH);
                P5 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, -BOX_WIDTH);
                P6 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, BOX_WIDTH);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, -BOX_WIDTH);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, BOX_WIDTH);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD+railWidth/2), -BOX_WIDTH);
                P2 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD+railWidth/2), BOX_WIDTH);
                P3 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD-railWidth/2), -BOX_WIDTH);
                P4 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.8, mysign*(railSpacing-BOX_INWARD-railWidth/2), BOX_WIDTH);
                P5 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, -BOX_WIDTH);
                P6 = curNode->vRelPos(-trackData->fHeart-spineHeight, mysign*railWidth/2, BOX_WIDTH);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, -BOX_WIDTH);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight, -mysign*railWidth/2, BOX_WIDTH);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);
            }

            if(index)
            {
                P1 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG1, BOX_WIDTH);
                P2 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG0, BOX_WIDTH);
                P3 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG1, BOX_WIDTH);
                P4 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG0, BOX_WIDTH);
                P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG0, -BOX_WIDTH);
                P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG1, -BOX_WIDTH);
                P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG0, -BOX_WIDTH);
                P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG1, -BOX_WIDTH);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);
            }
            break;
        case box:
            P1 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), -railSpacing, -BOX_WIDTH);
            P2 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), -railSpacing, BOX_WIDTH);
            P3 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), -railSpacing, -BOX_WIDTH);
            P4 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), -railSpacing, BOX_WIDTH);
            P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), railSpacing, -BOX_WIDTH);
            P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.6 : 0.8), railSpacing, BOX_WIDTH);
            P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), railSpacing, -BOX_WIDTH);
            P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.8 : 0.6), railSpacing, BOX_WIDTH);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            P1 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.6 : 0.8), -railSpacing, -BOX_WIDTH);
            P2 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.6 : 0.8), -railSpacing, BOX_WIDTH);
            P3 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.8 : 0.6), -railSpacing, -BOX_WIDTH);
            P4 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.8 : 0.6), -railSpacing, BOX_WIDTH);
            P5 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.6 : 0.8), railSpacing, -BOX_WIDTH);
            P6 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.6 : 0.8), railSpacing, BOX_WIDTH);
            P7 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.8 : 0.6), railSpacing, -BOX_WIDTH);
            P8 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.8 : 0.6), railSpacing, BOX_WIDTH);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), -BOX_WIDTH);
            P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), BOX_WIDTH);
            P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), -BOX_WIDTH);
            P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), BOX_WIDTH);
            P5 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), -BOX_WIDTH);
            P6 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), BOX_WIDTH);
            P7 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), -BOX_WIDTH);
            P8 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), BOX_WIDTH);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), -BOX_WIDTH);
            P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), BOX_WIDTH);
            P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), -BOX_WIDTH);
            P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), BOX_WIDTH);
            P5 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), -BOX_WIDTH);
            P6 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.7), BOX_WIDTH);
            P7 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), -BOX_WIDTH);
            P8 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.7), BOX_WIDTH);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

            if(index && index%2)
            {
                P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);
                P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);
                P5 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                P6 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);
                P7 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                P8 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                P2 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);
                P3 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                P4 = curNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);
                P5 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                P6 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);
                P7 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                P8 = lastNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG1, -BOX_WIDTH);
                P2 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG0, -BOX_WIDTH);
                P3 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG1, -BOX_WIDTH);
                P4 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG0, -BOX_WIDTH);
                P5 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG0, BOX_WIDTH);
                P6 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG1, BOX_WIDTH);
                P7 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG0, BOX_WIDTH);
                P8 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG1, BOX_WIDTH);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG1, -BOX_WIDTH);
                P2 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG0, -BOX_WIDTH);
                P3 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG1, -BOX_WIDTH);
                P4 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG0, -BOX_WIDTH);
                P5 = lastNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG0, BOX_WIDTH);
                P6 = lastNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG1, BOX_WIDTH);
                P7 = lastNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG0, BOX_WIDTH);
                P8 = lastNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG1, BOX_WIDTH);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);
            }
            else if(index && (index%2 == 0))
            {
                P1 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                P2 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);
                P3 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                P4 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);
                P5 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                P6 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                P2 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);
                P3 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                P4 = lastNode->vRelPos(-trackData->fHeart-mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);
                P5 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), -BOX_WIDTH);
                P6 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD-railWidth*0.5), BOX_WIDTH);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), -BOX_WIDTH);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight+mysign*railWidth*0.6, -mysign*(-railSpacing+BOX_INWARD+railWidth*0.5), BOX_WIDTH);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG1, BOX_WIDTH);
                P2 = lastNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG0, BOX_WIDTH);
                P3 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG1, BOX_WIDTH);
                P4 = lastNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG0, BOX_WIDTH);
                P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG0, -BOX_WIDTH);
                P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG1, -BOX_WIDTH);
                P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG0, -BOX_WIDTH);
                P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG1, -BOX_WIDTH);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = lastNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG1, BOX_WIDTH);
                P2 = lastNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), -railSpacing+BOX_DIAG0, BOX_WIDTH);
                P3 = lastNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG1, BOX_WIDTH);
                P4 = lastNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), -railSpacing+BOX_DIAG0, BOX_WIDTH);
                P5 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG0, -BOX_WIDTH);
                P6 = curNode->vRelPos(-trackData->fHeart-spineHeight+railWidth*(mysign < 0 ? 0.4 : 0.6), railSpacing-BOX_DIAG1, -BOX_WIDTH);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG0, -BOX_WIDTH);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight-railWidth*(mysign < 0 ? 0.6 : 0.4), railSpacing-BOX_DIAG1, -BOX_WIDTH);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);
            }
            break;
        case smallflat:
            P1 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), -railSpacing, -BOX_WIDTH);
            P2 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), -railSpacing, BOX_WIDTH);
            P3 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), -railSpacing, -BOX_WIDTH);
            P4 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), -railSpacing, BOX_WIDTH);
            P5 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), railSpacing, -BOX_WIDTH);
            P6 = curNode->vRelPos(-trackData->fHeart+railWidth*(mysign > 0 ? 0.5 : 1), railSpacing, BOX_WIDTH);
            P7 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), railSpacing, -BOX_WIDTH);
            P8 = curNode->vRelPos(-trackData->fHeart-railWidth*(mysign > 0 ? 1 : 0.5), railSpacing, BOX_WIDTH);

            create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);
            break;
        case doublespine:
            if(index%2 == 0)
            {
                P1 = curNode->vRelPos(-trackData->fHeart+0.75*railWidth*mysign, -railSpacing, -0.12*spineHeight);
                P2 = curNode->vRelPos(-trackData->fHeart+0.75*railWidth*mysign, -railSpacing, 0.12*spineHeight);
                P3 = curNode->vRelPos(-trackData->fHeart-0.25*railWidth*mysign, -railSpacing, -0.12*spineHeight);
                P4 = curNode->vRelPos(-trackData->fHeart-0.25*railWidth*mysign, -railSpacing, 0.12*spineHeight);
                P5 = curNode->vRelPos(-trackData->fHeart+0.75*railWidth*mysign, railSpacing, -0.12*spineHeight);
                P6 = curNode->vRelPos(-trackData->fHeart+0.75*railWidth*mysign, railSpacing, 0.12*spineHeight);
                P7 = curNode->vRelPos(-trackData->fHeart-0.25*railWidth*mysign, railSpacing, -0.12*spineHeight);
                P8 = curNode->vRelPos(-trackData->fHeart-0.25*railWidth*mysign, railSpacing, 0.12*spineHeight);
                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart+0.7*railWidth*mysign, -0.80*railSpacing, -0.10*spineHeight);
                P2 = curNode->vRelPos(-trackData->fHeart+0.7*railWidth*mysign, -0.80*railSpacing, 0.10*spineHeight);
                P3 = curNode->vRelPos(-trackData->fHeart-0.2*railWidth*mysign, -0.80*railSpacing, -0.10*spineHeight);
                P4 = curNode->vRelPos(-trackData->fHeart-0.2*railWidth*mysign, -0.80*railSpacing, 0.10*spineHeight);
                P5 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.7, 0, -0.20*spineHeight);
                P6 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.7, 0, 0.20*spineHeight);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, -0.10*spineHeight);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, 0.10*spineHeight);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);

                P1 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.7, 0, -0.20*spineHeight);
                P2 = curNode->vRelPos(-trackData->fHeart-spineHeight*0.7, 0, 0.20*spineHeight);
                P3 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, -0.10*spineHeight);
                P4 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.9*spineSize*mysign, 0, 0.10*spineHeight);
                P5 = curNode->vRelPos(-trackData->fHeart+0.7*railWidth*mysign, 0.80*railSpacing, -0.10*spineHeight);
                P6 = curNode->vRelPos(-trackData->fHeart+0.7*railWidth*mysign, 0.80*railSpacing, 0.10*spineHeight);
                P7 = curNode->vRelPos(-trackData->fHeart-0.2*railWidth*mysign, 0.80*railSpacing, -0.10*spineHeight);
                P8 = curNode->vRelPos(-trackData->fHeart-0.2*railWidth*mysign, 0.80*railSpacing, 0.10*spineHeight);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);
            }
            if((index+3)%4 == 0)
            {
                P1 = curNode->vRelPos(-trackData->fHeart-spineHeight, 0.65*spineSize, -0.65*spineSize);
                P2 = curNode->vRelPos(-trackData->fHeart-spineHeight, 0.65*spineSize, 0.65*spineSize);
                P3 = curNode->vRelPos(-trackData->fHeart-spineHeight, -0.65*spineSize, -0.65*spineSize);
                P4 = curNode->vRelPos(-trackData->fHeart-spineHeight, -0.65*spineSize, 0.65*spineSize);
                P5 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.4, 0.65*spineSize, -0.65*spineSize);
                P6 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.4, 0.65*spineSize, 0.65*spineSize);
                P7 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.4, -0.65*spineSize, -0.65*spineSize);
                P8 = curNode->vRelPos(-trackData->fHeart-spineHeight-0.4, -0.65*spineSize, 0.65*spineSize);

                create3dsBox(_vertices, _indices, P1, P2, P3, P4, P5, P6, P7, P8);
            }
            break;
        }
        if( index%5 == 4 && i != jSize-1 ) {
            _borders->append(_vertices->size());
            _borders->append(_indices->size());
        }
    }
    _borders->append(_vertices->size());
    _borders->append(_indices->size());

    return;
}

void trackMesh::recolorTrack()
{
    if(trackData->lSections.size() == 0) return;

    for(int i = 0; i < rails.size(); ++i)
    {       
        int node = rails[i].node;
        int section;
        trackData->getSecNode(node, &node, &section);

        curSection = trackData->lSections[section];

        rails[i].selected = (trackData == gloParent->curTrack() && curSection == trackData->activeSection) ? (curSection->isInFunction(node, gloParent->selectedFunc) ? 2.f : 1.f) : 0.f;
    }

    for(int i = 0; i < crossties.size(); ++i)
    {
        int node = crossties[i].node;
        int section;
        trackData->getSecNode(node, &node, &section);

        curSection = trackData->lSections[section];

        crossties[i].selected = (trackData == gloParent->curTrack() && curSection == trackData->activeSection) ? (curSection->isInFunction(node, gloParent->selectedFunc) ? 2.f : 1.f) : 0.f;
    }
    updateVertexArrays();
}

void trackMesh::updateVertexArrays()
{
    if(!glView->legacyMode)
    {
    glBindVertexArray(TrackObject[0]);

    glBindBuffer(GL_ARRAY_BUFFER, TrackBuffer[0]);  // Spine
    glBufferData(GL_ARRAY_BUFFER, rails.size()*sizeof(tracknode_t), rails.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 15*sizeof(float), 0);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(3*sizeof(float)));
    glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(6*sizeof(float)));
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(8*sizeof(float)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(9*sizeof(float)));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(10*sizeof(float)));
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(11*sizeof(float)));
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(12*sizeof(float)));
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(13*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glEnableVertexAttribArray(7);
    glEnableVertexAttribArray(8);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TrackIndices[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, pipeIndices.size()*sizeof(int), pipeIndices.data(), GL_STATIC_DRAW);


    glBindVertexArray(TrackObject[3]);

    glBindBuffer(GL_ARRAY_BUFFER, TrackBuffer[3]);  // Crossties
    glBufferData(GL_ARRAY_BUFFER, crossties.size()*sizeof(tracknode_t), crossties.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 15*sizeof(float), 0);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(3*sizeof(float)));
    glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(6*sizeof(float)));
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(8*sizeof(float)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(9*sizeof(float)));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(10*sizeof(float)));
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(11*sizeof(float)));
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(12*sizeof(float)));
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(13*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glEnableVertexAttribArray(7);
    glEnableVertexAttribArray(8);


    glBindVertexArray(TrackObject[4]);

    glBindBuffer(GL_ARRAY_BUFFER, TrackBuffer[6]);  // Supports
    glBufferData(GL_ARRAY_BUFFER, rendersupports.size()*sizeof(tracknode_t), rendersupports.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 15*sizeof(float), 0);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(3*sizeof(float)));
    glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(6*sizeof(float)));
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(8*sizeof(float)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(9*sizeof(float)));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(10*sizeof(float)));
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(11*sizeof(float)));
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(12*sizeof(float)));
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, 15*sizeof(float), (void*)(13*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glEnableVertexAttribArray(7);
    glEnableVertexAttribArray(8);


    glBindVertexArray(HeartObject[0]);

    glBindBuffer(GL_ARRAY_BUFFER, HeartBuffer[0]);  // Heartline
    glBufferData(GL_ARRAY_BUFFER, heartline.size()*sizeof(meshnode_t), heartline.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(HeartObject[1]);

    glBindBuffer(GL_ARRAY_BUFFER, HeartBuffer[1]);  // Rail Shadows
    glBufferData(GL_ARRAY_BUFFER, railshadows.size()*sizeof(meshnode_t), railshadows.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TrackIndices[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, shadowIndices.size()*sizeof(int), shadowIndices.data(), GL_STATIC_DRAW);

    glBindVertexArray(HeartObject[3]);

    glBindBuffer(GL_ARRAY_BUFFER, HeartBuffer[3]);  // Shadow Supports
    glBufferData(GL_ARRAY_BUFFER, supportshadows.size()*sizeof(meshnode_t), supportshadows.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(HeartObject[4]);

    glBindBuffer(GL_ARRAY_BUFFER, HeartBuffer[4]);  // Shadow Crossties
    glBufferData(GL_ARRAY_BUFFER, crosstieshadows.size()*sizeof(meshnode_t), crosstieshadows.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    }
}

void trackMesh::createIndices()
{
    if(nodeList.isEmpty()) return;
    glm::vec3 cameraPos = glView->cameraPos;
    int edgeCount = 0;
    for(int i = 0; i < options.size(); ++i) edgeCount += options[i].edges;

    QList<int> renderList;
    for(int i = 0; i < nodeList.size(); ++i)
    {
        renderList.append(i);
    }
    /*glm::mat4 anchorBase = glm::translate(trackData->startPos) * glm::rotate(trackData->startYaw-90, glm::vec3(0.f, 1.f, 0.f));

    for(int i = 0; i < nodeList.size();)
    {
        mnode* curNode = trackData->getPoint(nodeList[i]);
        glm::vec3 pos = glm::vec3(anchorBase*glm::vec4(curNode->vPos, 1));
        int dist = pow(glm::length(pos-cameraPos)/120.f, 4);
        if(dist < 1) dist = 1;
        if(dist > 16) dist = 16;
        renderList.append(i);
        i+=dist;
    }
    if(renderList.last() != nodeList.size()-1) renderList.append(nodeList.size()-1);*/

    int nodeCount = nodeList.size();


    pipeBorders.clear();
    pipeIndices.clear();
    pipeBorders.append(0);
    if(isWireframe)
    {
        for(int p = 0; p < numRails; ++p)
        {
            for(int i = 0; i < renderList.size(); ++i)
            {
                pipeIndices.append(p+numRails*renderList[i]);
            }
            pipeBorders.append(pipeIndices.size());
        }
    }
    else
    {
        for(int p = 0; p < options.size(); ++p)
        {
            int offset = 0;
            for(int i = 0; i < p; ++i)
            {
                offset += options[i].edges;
            }
            pipeIndices.append(p);
            for(int e = 0; e < options[p].edges; e+=2)
            {
                int e2 = (e+1)%options[p].edges;
                int e3 = (e+2)%options[p].edges;
                int i;
                for(i = 0; i < renderList.size(); ++i)
                {
                    int node = renderList[i];
                    pipeIndices.append(options.size() + offset + edgeCount*node + e);
                    pipeIndices.append(options.size() + offset + edgeCount*node + e2);
                }
                pipeIndices.append(options.size() + edgeCount*nodeCount + p);
                for(i = renderList.size()-1; i >= 0; --i)
                {
                    int node = renderList[i];
                    pipeIndices.append(options.size() + offset + edgeCount*node + e3);
                    pipeIndices.append(options.size() + offset + edgeCount*node + e2);
                }
                pipeIndices.append(p);
            }
            pipeBorders.append(pipeIndices.size());
        }

        shadowIndices.clear();
        for(int p = 0; p < options.size(); ++p)
        {
            int offset = 4*p;
            int disp = 4*options.size();
            int i = 0, oldI = 0;
            shadowIndices.append(offset+0);
            shadowIndices.append(offset+1);
            shadowIndices.append(offset+3);

            shadowIndices.append(offset+1);
            shadowIndices.append(offset+2);
            shadowIndices.append(offset+3);
            for(i = 1; i < railshadows.size()/(4*options.size()); ++i)
            {
                shadowIndices.append(i*disp+offset+0);
                shadowIndices.append(i*disp+offset+1);
                shadowIndices.append(oldI*disp+offset+0);

                shadowIndices.append(i*disp+offset+1);
                shadowIndices.append(oldI*disp+offset+1);
                shadowIndices.append(oldI*disp+offset+0);


                shadowIndices.append(i*disp+offset+1);
                shadowIndices.append(i*disp+offset+2);
                shadowIndices.append(oldI*disp+offset+1);

                shadowIndices.append(i*disp+offset+2);
                shadowIndices.append(oldI*disp+offset+2);
                shadowIndices.append(oldI*disp+offset+1);


                shadowIndices.append(i*disp+offset+2);
                shadowIndices.append(i*disp+offset+3);
                shadowIndices.append(oldI*disp+offset+2);

                shadowIndices.append(i*disp+offset+3);
                shadowIndices.append(oldI*disp+offset+3);
                shadowIndices.append(oldI*disp+offset+2);


                shadowIndices.append(i*disp+offset+3);
                shadowIndices.append(i*disp+offset+0);
                shadowIndices.append(oldI*disp+offset+3);

                shadowIndices.append(i*disp+offset+0);
                shadowIndices.append(oldI*disp+offset+0);
                shadowIndices.append(oldI*disp+offset+3);

                oldI = i;
            }

            shadowIndices.append(oldI*disp+offset+1);
            shadowIndices.append(oldI*disp+offset+0);
            shadowIndices.append(oldI*disp+offset+2);

            shadowIndices.append(oldI*disp+offset+0);
            shadowIndices.append(oldI*disp+offset+3);
            shadowIndices.append(oldI*disp+offset+2);
        }
    }

    updateVertexArrays();
    /*if(!glView->legacyMode)
    {
        glBindVertexArray(TrackObject[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TrackIndices[0]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, pipeIndices.size()*sizeof(int), pipeIndices.data(), GL_STATIC_DRAW);

        glBindVertexArray(HeartObject[1]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TrackIndices[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, shadowIndices.size()*sizeof(int), shadowIndices.data(), GL_STATIC_DRAW);
        glBindVertexArray(0);
    }*/
}
