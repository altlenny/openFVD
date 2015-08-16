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

#include "undohandler.h"
#include "mainwindow.h"

extern MainWindow* gloParent;

undoHandler::undoHandler()
{
}

undoHandler::undoHandler(int _stackSize)
{
    busy = false;
    maxStackSize = _stackSize;
    stackIndex = -1;
}

undoHandler::~undoHandler()
{
    for(int i=0; i < lActions.size(); ++i)
    {
        delete lActions[i];
    }
}

void undoHandler::doUndo()
{
    if(stackIndex == -1) return;

    gloParent->setUpdatesEnabled(false);
    busy = true;
    lActions[stackIndex]->doUndo();
    busy = false;
    gloParent->setUpdatesEnabled(true);

    if(++stackIndex > lActions.size()) stackIndex = -1;
}

void undoHandler::doRedo()
{
    if(stackIndex == 0) return;

    gloParent->setUpdatesEnabled(false);
    busy = true;
    lActions[--stackIndex]->doRedo();
    busy = false;
    gloParent->setUpdatesEnabled(true);
}

void undoHandler::addAction(undoAction* _action)
{
    switch(_action->type)
    {
    case onLengthSpin:
        if(_action->toValue.toDouble() == oldLengthSpinValue)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldLengthSpinValue;
        oldLengthSpinValue = _action->toValue.toDouble();
        break;
    case onChangeSpin:
        if(_action->toValue.toDouble() == oldChangeSpinValue)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldChangeSpinValue;
        oldChangeSpinValue = _action->toValue.toDouble();
        break;
    case onArg1:
        if(_action->toValue.toDouble() == oldArg1Value)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldArg1Value;
        oldArg1Value = _action->toValue.toDouble();
        break;
    case onCenterSpin:
        if(_action->toValue.toDouble() == oldCenterSpinValue)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldCenterSpinValue;
        oldCenterSpinValue = _action->toValue.toDouble();
        break;
    case onTensionSpin:
        if(_action->toValue.toDouble() == oldTensionSpinValue)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldTensionSpinValue;
        oldTensionSpinValue = _action->toValue.toDouble();
        break;
    case onTransitionBox:
        if(_action->toValue.toInt() == oldTransitionBoxValue)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldTransitionBoxValue;
        oldTransitionBoxValue = _action->toValue.toInt();
        break;
    case changeSegmentLength:
        if(_action->toValue.toDouble() == oldSegmentLength)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldSegmentLength;
        oldSegmentLength = _action->toValue.toDouble();
        break;
    case changeCurveDirection:
        if(_action->toValue.toDouble() == oldCurveDirection)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldCurveDirection;
        oldCurveDirection = _action->toValue.toDouble();
        break;
    case changeCurveLeadIn:
        if(_action->toValue.toDouble() == oldCurveLeadIn)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldCurveLeadIn;
        oldCurveLeadIn = _action->toValue.toDouble();
        break;
    case changeCurveLeadOut:
        if(_action->toValue.toDouble() == oldCurveLeadOut)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldCurveLeadOut;
        oldCurveLeadOut = _action->toValue.toDouble();
        break;
    case changeCurveRadius:
        if(_action->toValue.toDouble() == oldCurveRadius)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldCurveRadius;
        oldCurveRadius = _action->toValue.toDouble();
        break;
    case changeSpeedState:
        if(_action->toValue.toBool() == oldSpeedState)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldSpeedState;
        oldSpeedState = _action->toValue.toBool();
        break;
    case changeSegmentSpeed:
        if(_action->toValue.toDouble() == oldSegmentSpeed)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldSegmentSpeed;
        oldSegmentSpeed = _action->toValue.toDouble();
        break;
    case changeAnchorPosX:
        if(_action->toValue.toDouble() == oldAnchorPosX)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldAnchorPosX;
        oldAnchorPosX = _action->toValue.toDouble();
        break;
    case changeAnchorPosY:
        if(_action->toValue.toDouble() == oldAnchorPosY)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldAnchorPosY;
        oldAnchorPosY = _action->toValue.toDouble();
        break;
    case changeAnchorPosZ:
        if(_action->toValue.toDouble() == oldAnchorPosZ)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldAnchorPosZ;
        oldAnchorPosZ = _action->toValue.toDouble();
        break;
    case changeAnchorRoll:
        if(_action->toValue.toDouble() == oldAnchorRoll)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldAnchorRoll;
        oldAnchorRoll = _action->toValue.toDouble();
        break;
    case changeAnchorYaw:
        if(_action->toValue.toDouble() == oldAnchorYaw)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldAnchorYaw;
        oldAnchorYaw = _action->toValue.toDouble();
        break;
    case changeAnchorPitch:
        if(_action->toValue.toDouble() == oldAnchorPitch)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldAnchorPitch;
        oldAnchorPitch = _action->toValue.toDouble();
        break;
    case changeAnchorNormal:
        if(_action->toValue.toDouble() == oldAnchorNormal)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldAnchorNormal;
        oldAnchorNormal = _action->toValue.toDouble();
        break;
    case changeAnchorLateral:
        if(_action->toValue.toDouble() == oldAnchorLateral)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldAnchorLateral;
        oldAnchorLateral = _action->toValue.toDouble();
        break;
    case changeAnchorPitchChange:
        if(_action->toValue.toDouble() == oldAnchorPitchChange)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldAnchorPitchChange;
        oldAnchorPitchChange = _action->toValue.toDouble();
        break;
    case changeAnchorYawChange:
        if(_action->toValue.toDouble() == oldAnchorYawChange)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldAnchorYawChange;
        oldAnchorYawChange = _action->toValue.toDouble();
        break;
    case changeAnchorSpeed:
        if(_action->toValue.toDouble() == oldAnchorSpeed)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldAnchorSpeed;
        oldAnchorSpeed = _action->toValue.toDouble();
        break;
    case changeTrackHeartline:
        if(_action->toValue.toDouble() == oldTrackHeartline)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldTrackHeartline;
        oldTrackHeartline = _action->toValue.toDouble();
        break;
    case changeTrackFriction:
        if(_action->toValue.toDouble() == oldTrackFriction)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldTrackFriction;
        oldTrackFriction = _action->toValue.toDouble();
        break;
    case changeTrackResistance:
        if(_action->toValue.toDouble() == oldTrackResistance)
        {
            delete _action;
            return;
        }
        _action->fromValue = oldTrackResistance;
        oldTrackResistance = _action->toValue.toDouble();
        break;
    default:
        break;
    }

    while(stackIndex > 0)
    {
        delete lActions[0];
        lActions.removeFirst();
        stackIndex--;
    }

    lActions.prepend(_action);
    if(lActions.size() > maxStackSize)
    {
        delete lActions[lActions.size()-1];
        lActions.removeLast();
    }

    stackIndex = 0;
}

void undoHandler::clearActions()
{
    while(lActions.size())
    {
        delete lActions[0];
        lActions.removeFirst();
    }
    stackIndex = -1;
}
