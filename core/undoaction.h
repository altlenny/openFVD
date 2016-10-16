#ifndef UNDOACTION_H
#define UNDOACTION_H

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

#include "function.h"
#include <QVariant>

#include <sstream>

class MainWindow;
class trackHandler;
class track;

enum eActionType
{
    onLengthSpin,
    onChangeSpin,
    onArg1,
    onCenterSpin,
    onTensionSpin,
    onTransitionBox,
    appendSubFunction,
    removeSubFunction,
    changeSegmentLength,
    changeCurveDirection,
    changeCurveLeadIn,
    changeCurveLeadOut,
    changeCurveRadius,
    changeSpeedState,
    changeSegmentSpeed,
    changeAnchorPosX,
    changeAnchorPosY,
    changeAnchorPosZ,
    changeAnchorRoll,
    changeAnchorPitch,
    changeAnchorYaw,
    changeAnchorNormal,
    changeAnchorLateral,
    changeAnchorPitchChange,
    changeAnchorYawChange,
    changeAnchorSpeed,
    changeTrackHeartline,
    changeTrackFriction,
    changeTrackResistance,
    appendSegment,
    removeSegment,
    changeSegmentArgument, //<-----
    changeSegmentOrientation,
    changeFunctionStatus,

    newTrack,
    deleteTrack
};

class undoAction
{
public:
    undoAction();
    undoAction(trackHandler* _track, eActionType _type);
    ~undoAction();

    void doUndo();
    void doRedo();

    trackHandler* hTrack;

    eActionType type;

    QVariant fromValue;
    QVariant toValue;

    std::stringstream* info;

    undoAction* nextAction;

    track* inTrack;
    int sectionNumber;
    eFunctype inFunction;
    int subfuncNumber;
};

#endif // UNDOACTION_H
