#ifndef UNDOHANDLER_H
#define UNDOHANDLER_H

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

#include "undoaction.h"

class MainWindow;

class undoHandler
{
public:
    undoHandler();
    undoHandler(int _stackSize = 15);
    ~undoHandler();
    void doUndo();
    void doRedo();
    void addAction(undoAction* _action);
    void clearActions();

    QList<undoAction*> lActions;
    int maxStackSize;

    int stackIndex;


    double oldLengthSpinValue;
    double oldChangeSpinValue;
    double oldArg1Value;
    double oldCenterSpinValue;
    double oldTensionSpinValue;

    int oldTransitionBoxValue;
    int oldQuadraticBoxValue;

    double oldSegmentLength;
    double oldCurveDirection;
    double oldCurveLeadIn;
    double oldCurveLeadOut;
    double oldCurveRadius;
    double oldSegmentSpeed;

    bool oldSpeedState;

    double oldAnchorPosX;
    double oldAnchorPosY;
    double oldAnchorPosZ;
    double oldAnchorRoll;
    double oldAnchorPitch;
    double oldAnchorYaw;
    double oldAnchorNormal;
    double oldAnchorLateral;
    double oldAnchorPitchChange;
    double oldAnchorYawChange;
    double oldAnchorSpeed;
    double oldTrackHeartline;
    double oldTrackFriction;
    double oldTrackResistance;


    bool busy;
};

#endif // UNDOHANDLER_H
