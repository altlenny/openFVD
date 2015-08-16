#ifndef GRAPHHANDLER_H
#define GRAPHHANDLER_H

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

#include <QColor>
#include "section.h"

class QTreeWidgetItem;
class QCPGraph;
class trackHandler;
class QCPAxis;

enum graphType
{
    banking = 0,
    rollSpeed,
    rollAccel,
    nForce,
    nForceChange,
    lForce,
    lForceChange,
    pitchChange,
    yawChange,
    secBoundaries,
    povPos,
    smoothedRollSpeed,
    smoothedNForce,
    smoothedLForce
};

class graphHandler
{
public:
    graphHandler(QTreeWidgetItem* _treeItem, enum graphType _type, trackHandler* _track, QCPAxis* _axis, QColor* _color);

    void fillGraphList(QCPAxis* xAxis, bool _argument = TIME, bool _orientation = QUATERNION, bool _drawExterns = true);

    QTreeWidgetItem* treeItem;
    QColor color[4];
    QList<QCPGraph*> graphList;
    QList<bool> selectable;
    bool active;
    bool drawn;
    enum graphType mType;

    Qt::CheckState prevCheckState;
    trackHandler* mTrack;

    QCPAxis* usedAxis;

private:
    void fillActiveGraphList(QCPAxis* xAxis, bool _argument, bool _drawExterns);
    void fillBoundaryGraphList(QCPAxis *xAxis, bool _argument);
};

#endif // GRAPHHANDLER_H
