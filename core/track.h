#ifndef TRACK_H
#define TRACK_H

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

#include "mnode.h"
#include "secstraight.h"
#include "seccurved.h"
#include "secforced.h"
#include "secgeometric.h"
#include "secbezier.h"
#include "sectionhandler.h"
#include <QList>
#include <fstream>
#include <QString>

class optionsMenu;
class sectionHandler;
class trackWidget;
class smoothUi;
class smoothHandler;
class trackHandler;

enum trackStyle {
    generic = 0,        // 0,5m
    genericflat,    // 0,7m
    vekoma,         // 0,6m
    bm,             // 0,6m
    triangle,       // 0,5m
    box,            // 0,5m
    smallflat,           // 0,5m
    doublespine
};

class track
{
public:
    track();
    track(trackHandler* _parent ,glm::vec3 startPos, float startYaw, float heartLine = 0.0);
    ~track();
    void removeSection(int index);
    void removeSection(section* fromSection);

    void removeSmooth(int fromNode = 0);
    void applySmooth(int fromNode = 0);

    void updateTrack(int index, int iNode);
    void updateTrack(section* fromSection, int iNode);
    void newSection(enum secType type, int index = -1);

    int exportTrack(std::fstream* file, float mPerNode, int fromIndex, int toIndex, float fRollThresh);
    int exportTrack2(std::fstream* file, float mPerNode, int fromIndex, int toIndex, float fRollThresh);
    int exportTrack3(std::fstream* file, float mPerNode, int fromIndex, int toIndex, float fRollThresh);
    int exportTrack4(std::fstream* file, float mPerNode, int fromIndex, int toIndex, float fRollThresh);

    void exportNL2Track(FILE *file, float mPerNode, int fromIndex, int toIndex);

    QString saveTrack(std::fstream& file, trackWidget* _widget);
    QString loadTrack(std::fstream& file, trackWidget* _widget);
    QString legacyLoadTrack(std::fstream& file, trackWidget* _widget);
    mnode* getPoint(int index);
    int getIndexFromDist(float dist);
    int getNumPoints(section* until = NULL);
    int getSectionNumber(section* _section);

    void getSecNode(int index, int *node, int *section);

    bool hasChanged;
    bool drawTrack;
    int drawHeartline;

    mnode* anchorNode;

    glm::vec3 startPos;
    float startYaw;
    float startPitch;

    section* activeSection;
    float fHeart;
    float fFriction;
    float fResistance;
    QList<section*> lSections;

    optionsMenu* mOptions;
    QString name;

    smoothUi* smoother;

    QList<smoothHandler*> smoothList;

    trackHandler* mParent;

    int smoothedUntil;
    enum trackStyle style;
    glm::vec2 povPos;
};

#endif // TRACK_H
