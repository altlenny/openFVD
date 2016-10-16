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

#include "nolimitsimporter.h"
#include <fstream>
#include "exportfuncs.h"
#include "secbezier.h"
#include "trackwidget.h"

using namespace std;

noLimitsImporter::noLimitsImporter(trackHandler* _track, QString _fileName)
{
    inTrack = _track;
    fileName = _fileName;
}

bool noLimitsImporter::importAsNlTrack()
{
    fstream fin(fileName.toLocal8Bit().data(), ios::in | ios::binary);
    if(!fin)
    {
        return false;
    }

    inTrack->trackData->fHeart = 0.f;
    inTrack->trackWidgetItem->addSection(bezier);

    fin.seekg (0, ios::end);

    int length = fin.tellg();
    string temp;
    QList<bezier_t*> *bList = &inTrack->trackData->activeSection->bezList;

    QList<glm::vec3> *lineList = &inTrack->trackData->activeSection->supList;

    QList<glm::vec3> TubeNodes[3];

    int bezCount, fundCount, freeCount, tubeCount;
    glm::vec3 anchor;
    bool closeTrack = false;

    for(int i = 0; i < length; ++i)
    {
        fin.seekg(i);
        temp = readString(&fin, 4);
        if(temp == "SEGM")
        {
            readNulls(&fin, 4);
            int segCount = readInt(&fin);
            closeTrack = readBool(&fin);
            readNulls(&fin, 16);
            for(int i = 0; i < segCount; ++i)
            {
                int type = readInt(&fin);
                readNulls(&fin, 29);
                if(type == 0)
                {
                    readNulls(&fin, 4);
                    bList->at(i)->fVel = 0.f;
                }
                else if(type == 1)   // station
                {
                    readNulls(&fin, 54);
                    bList->at(i)->fVel = readFloat(&fin);
                    readNulls(&fin, 72);
                }
                else if(type == 2) // lift
                {
                    readNulls(&fin, 8);
                    bList->at(i)->fVel = readFloat(&fin);
                    readNulls(&fin, 28);
                }
                else if(type == 3) // transport
                {
                    readNulls(&fin, 4);
                    bList->at(i)->fVel = readFloat(&fin);
                    readNulls(&fin, 24);
                }
                else if(type == 4) // brake
                {
                    readNulls(&fin, 4);
                    bList->at(i)->fVel = readFloat(&fin);
                    readNulls(&fin, 46);
                }
                else
                {
                    qWarning("something wrong importing NL Track");
                }
                if(bList->at(i)->fVel != bList->at(i)->fVel)
                {
                    qWarning("read nan while importing NL Track");
                }

            }
        }
        if(temp == "BEZR")
        {
            readInt(&fin);  // size
            readNulls(&fin, 16);
            bezCount = readInt(&fin);
            for(int b = 0; b < bezCount; ++b)
            {
                bList->append(new bezier_t);
                bList->at(b)->P1.x = readFloat(&fin);
                bList->at(b)->P1.y = readFloat(&fin);
                bList->at(b)->P1.z = readFloat(&fin);
                bList->at(b)->Kp1.x = readFloat(&fin);
                bList->at(b)->Kp1.y = readFloat(&fin);
                bList->at(b)->Kp1.z = readFloat(&fin);
                bList->at(b)->Kp2.x = readFloat(&fin);
                bList->at(b)->Kp2.y = readFloat(&fin);
                bList->at(b)->Kp2.z = readFloat(&fin);
                bList->at(b)->roll = readFloat(&fin);
                bList->at(b)->contRoll = readBool(&fin);
                bList->at(b)->equalDist = readBool(&fin);
                bList->at(b)->relRoll = readBool(&fin);
                readNulls(&fin, 17);
                if(b == 0) anchor = bList->at(b)->P1;
                bList->at(b)->P1 -= anchor;
                bList->at(b)->Kp1 -= anchor;
                bList->at(b)->Kp2 -= anchor;

                bList->at(b)->ptf = 0.f;
                bList->at(b)->fvdRoll = 0.f;
                bList->at(b)->fVel = 0.f;
            }
        }
        if(temp == "FUND")
        {
            readInt(&fin);  //size
            readNulls(&fin, 16);
            fundCount = readInt(&fin);
            for(int b = 0; b < fundCount; ++b)
            {
                glm::vec3 temp;
                /*for(int i = 0; i < 4; ++i)
                {
                    temp.x = readFloat(&fin);
                }*/
                readNulls(&fin, 16);
                temp.x = readFloat(&fin);
                temp.y = readFloat(&fin);
                temp.z = readFloat(&fin);
                readNulls(&fin, 64);
                TubeNodes[0].append(temp);
                /*for(int i = 0; i < 12; ++i)
                {
                    temp.x = readFloat(&fin);
                }*/
            }
        }
        if(temp == "FREN")
        {
            readInt(&fin);  // size
            readNulls(&fin, 16);
            freeCount = readInt(&fin);
            for(int b = 0; b < freeCount; ++b)
            {
                glm::vec3 temp;
                temp.x = readFloat(&fin);
                temp.y = readFloat(&fin);
                temp.z = readFloat(&fin);
                TubeNodes[1].append(temp);
                readNulls(&fin, 16);
            }
        }
        if(temp == "TUBE")
        {
            int type, index;
            readInt(&fin);  // size
            readNulls(&fin, 16);
            tubeCount = readInt(&fin);
            for(int b = 0; b < tubeCount; ++b)
            {
                type = readInt(&fin);
                readInt(&fin);  // segment
                index = readInt(&fin);
                if(type != 3)
                {
                    lineList->append(TubeNodes[type-1][index] - anchor);
                }
                readNulls(&fin, 4);
                type = readInt(&fin);
                readInt(&fin);  // segment
                index = readInt(&fin);
                if(type != 3 && lineList->size()%2)
                {
                    lineList->append(TubeNodes[type-1][index] - anchor);
                }
                else if(type == 3 && lineList->size()%2)
                {
                    lineList->removeLast();
                }
                readNulls(&fin, 20);
            }
        }
    }

    if(closeTrack)
    {
        bList->append(new bezier_t);
        bList->last()->P1 = bList->at(0)->P1;
        bList->last()->Kp1 = bList->at(0)->Kp1;
        bList->last()->Kp2 = bList->at(0)->Kp2;
        bList->last()->roll = bList->at(0)->roll;
        bList->last()->contRoll = bList->at(0)->contRoll;
        bList->last()->equalDist = bList->at(0)->equalDist;
        bList->last()->relRoll = bList->at(0)->relRoll;
        bList->last()->ptf = 0.f;
        bList->last()->fvdRoll = 0.f;
    }

    inTrack->trackData->updateTrack(0, 0);
    fin.close();
    return true;
}

bool noLimitsImporter::importAsTxt()
{
    fstream fin(fileName.toLocal8Bit().data(), ios::in);
    if(!fin)
    {
        return false;
    }

    inTrack->trackData->fHeart = 0.f;
    inTrack->trackWidgetItem->addSection(bezier);

    fin.seekg (0, ios::end);

    QList<bezier_t*> *bList = &inTrack->trackData->activeSection->bezList;

    fin.seekg(0);

    float x, y, z;
    int b = 0;

    while(!fin.eof())
    {
        fin >> z;
        fin >> x;
        fin >> y;
        bList->append(new bezier_t);
        bList->at(b)->P1.x = x;
        bList->at(b)->P1.y = y;
        bList->at(b)->P1.z = z;
        b++;
    }

    glm::vec3 anchor = bList->at(0)->P1;
    bList->at(0)->Kp2 = 2.f/3.f * bList->at(0)->P1 + 1.f/3.f * bList->at(1)->P1;
    bList->at(0)->Kp1 = 2.f*bList->at(0)->P1 - bList->at(0)->Kp2;

    bList->last()->Kp1 = 2.f/3.f * bList->last()->P1 + 1.f/3.f * bList->at(bList->size()-2)->P1;
    bList->last()->Kp2 = 2.f*bList->last()->P1 - bList->last()->Kp1;

    for(b = 1; b < bList->size()-1; ++b)
    {
        bList->at(b)->Kp1 = 2.f/3.f * bList->at(b)->P1 + 1.f/3.f * bList->at(b-1)->P1;
        bList->at(b)->Kp2 = 2.f/3.f * bList->at(b)->P1 + 1.f/3.f * bList->at(b+1)->P1;
        bList->at(b)->roll = 0.f;
        bList->at(b)->contRoll = true;
        bList->at(b)->equalDist = true;
        bList->at(b)->relRoll = false;
        bList->at(b)->ptf = 0.f;
        bList->at(b)->fvdRoll = 0.f;
        bList->at(b)->fVel = 0.f;
    }

    for(b = 0; b < bList->size(); ++b)
    {
        //bList->at(b)->P1 = 0.5f*(bList->at(b)->Kp1 + bList->at(b)->Kp2);
        bList->at(b)->P1 -= anchor;
        bList->at(b)->Kp1 -= anchor;
        bList->at(b)->Kp2 -= anchor;
    }

    inTrack->trackData->updateTrack(0, 0);
    fin.close();
    return true;
}
