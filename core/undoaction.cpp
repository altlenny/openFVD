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

#include "exportfuncs.h"
#include "undoaction.h"
#include "trackhandler.h"
#include "graphwidget.h"
#include "transitionwidget.h"
#include "trackwidget.h"
#include "mainwindow.h"

extern MainWindow* gloParent;

undoAction::undoAction()
{

}

undoAction::undoAction(trackHandler* _track, eActionType _type)
{
    type = _type;
    hTrack = _track;
    inTrack = _track->trackData;
    if(type == newTrack || type == deleteTrack);
    else if(type != changeAnchorPosX && type != changeAnchorPosY && type != changeAnchorPosZ && type != changeAnchorYaw && type != changeAnchorRoll
            && type != changeAnchorPitch && type != changeAnchorNormal && type != changeAnchorLateral && type != changeAnchorSpeed
            && type != changeTrackHeartline && type != changeTrackFriction && type != changeTrackResistance && type != changeAnchorPitchChange && type != changeAnchorYawChange)
    {
        sectionNumber = inTrack->getSectionNumber(inTrack->activeSection);
        if(type != changeSegmentLength && type != changeCurveDirection && type != changeCurveLeadIn && type != changeCurveLeadOut
                && type != changeCurveRadius && type != changeSpeedState && type != changeSegmentSpeed && type != appendSegment
                && type != removeSegment && type != changeSegmentArgument && type != changeSegmentOrientation)
        {
            inFunction = hTrack->graphWidgetItem->selFunc->parent->type;
            subfunctionNumber = hTrack->graphWidgetItem->selFunc->parent->getSubfunctionNumber(hTrack->graphWidgetItem->selFunc);
        }
    }
    nextAction = NULL;
    info = NULL;
    if(type == removeSubFunction)
    {
        info = new std::stringstream();
        hTrack->graphWidgetItem->selFunc->saveSubFunc(*info);
        info->seekp(0);
    }
    else if(type == removeSegment)
    {
        info = new std::stringstream();
        inTrack->lSections.at(sectionNumber)->saveSection(*info);
        info->seekp(0);
    }
}

undoAction::~undoAction()
{
    if(nextAction)
    {
        delete nextAction;
    }
    if(info)
    {
        delete info;
    }
}

void undoAction::doUndo()
{
    if(type == onLengthSpin || type == onChangeSpin ||  type == onArg1 || type == onCenterSpin
            || type == onTensionSpin || type == onTransitionBox || type == appendSubFunction || type == changeFunctionStatus)
    {
        hTrack->trackWidgetItem->setSelection(sectionNumber);
        //parent->on_treeWidget_itemClicked(parent->getTopLevel(sectionNumber+1), 0);
        switch(inFunction)
        {
        case funcRoll:
            hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->rollFunc->funcList[subfunctionNumber]);
            break;
        case funcNormal:
            hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->normForce->funcList[subfunctionNumber]);
            break;
        case funcLateral:
            hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->latForce->funcList[subfunctionNumber]);
            break;
        case funcPitch:
            hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->normForce->funcList[subfunctionNumber]);
            break;
        case funcYaw:
            hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->latForce->funcList[subfunctionNumber]);
            break;
        default:
            break;
        }
    }
    else if(type == removeSubFunction)
    {
        hTrack->trackWidgetItem->setSelection(sectionNumber);
        //parent->on_treeWidget_itemClicked(parent->getTopLevel(sectionNumber+1), 0);
        if(subfunctionNumber > 0)
        {
            switch(inFunction)
            {
            case funcRoll:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->rollFunc->funcList[subfunctionNumber-1]);
                break;
            case funcNormal:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->normForce->funcList[subfunctionNumber-1]);
                break;
            case funcLateral:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->latForce->funcList[subfunctionNumber-1]);
                break;
            case funcPitch:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->normForce->funcList[subfunctionNumber-1]);
                break;
            case funcYaw:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->latForce->funcList[subfunctionNumber-1]);
                break;
            default:
                break;
            }
        }
        else
        {
            switch(inFunction)
            {
            case funcRoll:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->rollFunc->funcList[subfunctionNumber]);
                break;
            case funcNormal:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->normForce->funcList[subfunctionNumber]);
                break;
            case funcLateral:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->latForce->funcList[subfunctionNumber]);
                break;
            case funcPitch:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->normForce->funcList[subfunctionNumber]);
                break;
            case funcYaw:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->latForce->funcList[subfunctionNumber]);
                break;
            default:
                break;
            }
        }
    }
    else if(type == changeSegmentLength || type == changeCurveDirection || type == changeCurveLeadIn || type == changeCurveLeadOut
            || type == changeCurveRadius || type == changeSpeedState || type == changeSegmentSpeed || type == changeSegmentArgument
            || type == changeSegmentOrientation)
    {
        hTrack->trackWidgetItem->setSelection(sectionNumber);
        //parent->on_treeWidget_itemClicked(parent->getTopLevel(sectionNumber+1), 0);
    }
    else if(type == changeAnchorPosX || type == changeAnchorPosY || type == changeAnchorPosZ || type == changeAnchorYaw || type == changeAnchorRoll
            || type == changeAnchorPitch || type == changeAnchorNormal || type == changeAnchorLateral || type == changeAnchorSpeed
            || type == changeTrackHeartline || type == changeTrackFriction || type == changeTrackResistance || type == changeAnchorPitchChange || type == changeAnchorYawChange)
    {
    }
    else if(type == appendSegment)
    {
        hTrack->trackWidgetItem->setSelection(sectionNumber);
    }
    else if(type == removeSegment)
    {
        hTrack->trackWidgetItem->setSelection(sectionNumber-1);
    }

    std::string temp;

    switch (type)
    {
    case onLengthSpin:
        hTrack->graphWidgetItem->selFunc->parent->changeLength(fromValue.toDouble(), subfunctionNumber);
        hTrack->graphWidgetItem->selectionChanged();
        inTrack->updateTrack(sectionNumber, (int)(hTrack->graphWidgetItem->selFunc->minArgument*F_HZ-1.5f));
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case onChangeSpin:
        hTrack->graphWidgetItem->selFunc->symArg = fromValue.toDouble();
        hTrack->graphWidgetItem->selectionChanged();
        inTrack->updateTrack(sectionNumber, (int)(hTrack->graphWidgetItem->selFunc->minArgument*F_HZ-1.5f));
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case onArg1:
        hTrack->graphWidgetItem->selFunc->arg1 = fromValue.toDouble();
        hTrack->graphWidgetItem->selectionChanged();
        inTrack->updateTrack(sectionNumber, (int)(hTrack->graphWidgetItem->selFunc->minArgument*F_HZ-1.5f));
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case onCenterSpin:
        hTrack->graphWidgetItem->selFunc->centerArg = fromValue.toDouble();
        hTrack->graphWidgetItem->selectionChanged();
        inTrack->updateTrack(sectionNumber, (int)(hTrack->graphWidgetItem->selFunc->minArgument*F_HZ-1.5f));
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case onTensionSpin:
        hTrack->graphWidgetItem->selFunc->tensionArg = fromValue.toDouble();
        hTrack->graphWidgetItem->selectionChanged();
        inTrack->updateTrack(sectionNumber, (int)(hTrack->graphWidgetItem->selFunc->minArgument*F_HZ-1.5f));
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case onTransitionBox:
        hTrack->graphWidgetItem->selFunc->degree = (eDegree)fromValue.toInt();
        hTrack->graphWidgetItem->selectionChanged();
        inTrack->updateTrack(sectionNumber, (int)(hTrack->graphWidgetItem->selFunc->minArgument*F_HZ-1.5f));
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case appendSubFunction:
        hTrack->graphWidgetItem->transitionHandler->on_removeButton_released();
        break;
    case removeSubFunction:
        if(subfunctionNumber > 0)
        {
            hTrack->graphWidgetItem->transitionHandler->on_appendButton_released();
            hTrack->graphWidgetItem->selFunc->loadSubFunc(*info);
            delete info;
            info = new std::stringstream();
            hTrack->graphWidgetItem->selFunc->saveSubFunc(*info);
            info->seekp(0);
            //parent->selectGraph(subfunctionNumber, inFunction);
        }
        else
        {
            hTrack->graphWidgetItem->transitionHandler->on_prependButton_released();
            hTrack->graphWidgetItem->selFunc->loadSubFunc(*info);
            delete info;
            info = new std::stringstream();
            hTrack->graphWidgetItem->selFunc->saveSubFunc(*info);
            info->seekp(0);
            //parent->selectGraph(subfunctionNumber, inFunction);
        }
        hTrack->graphWidgetItem->selectionChanged();
        inTrack->updateTrack(sectionNumber, (int)(hTrack->graphWidgetItem->selFunc->minArgument*F_HZ-1.5f));
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeSegmentLength:
        inTrack->lSections[sectionNumber]->rollFunc->setMaxArgument(fromValue.toDouble());
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->setupStraightFrame();
        hTrack->trackWidgetItem->setupCurvedFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeCurveDirection:
        inTrack->lSections.at(sectionNumber)->fDirection = fromValue.toDouble();
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->setupCurvedFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeCurveLeadIn:
        inTrack->lSections.at(sectionNumber)->fLeadIn = fromValue.toDouble();
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->setupCurvedFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeCurveLeadOut:
        inTrack->lSections.at(sectionNumber)->fLeadOut = fromValue.toDouble();
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->setupCurvedFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeCurveRadius:
        inTrack->lSections.at(sectionNumber)->fRadius = fromValue.toDouble();
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->setupCurvedFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeSpeedState:
        inTrack->lSections.at(sectionNumber)->bSpeed = fromValue.toBool();
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->setupStraightFrame();
        hTrack->trackWidgetItem->setupCurvedFrame();
        hTrack->trackWidgetItem->setupAdvFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeSegmentSpeed:
        inTrack->lSections.at(sectionNumber)->fVel = fromValue.toDouble();
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->setupStraightFrame();
        hTrack->trackWidgetItem->setupCurvedFrame();
        hTrack->trackWidgetItem->setupAdvFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorPosX:
        inTrack->startPos.x = fromValue.toDouble();
        hTrack->trackWidgetItem->setupAnchorFrame();
        break;
    case changeAnchorPosY:
        inTrack->startPos.y = fromValue.toDouble();
        hTrack->trackWidgetItem->setupAnchorFrame();
        break;
    case changeAnchorPosZ:
        inTrack->startPos.z = fromValue.toDouble();
        hTrack->trackWidgetItem->setupAnchorFrame();
        inTrack->hasChanged = true;
        break;
    case changeAnchorRoll:
        inTrack->anchorNode->setRoll(fromValue.toDouble()-inTrack->anchorNode->fRoll);
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorYaw:
        inTrack->startYaw = fromValue.toDouble();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorPitch:
        inTrack->startPitch = fromValue.toDouble();
        inTrack->anchorNode->changePitch((float)fromValue.toDouble()-inTrack->anchorNode->getPitch(), fabs(inTrack->anchorNode->fRoll) >= 90.f);
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorNormal:
        inTrack->anchorNode->forceNormal = fromValue.toDouble();
        hTrack->trackWidgetItem->updateAnchorGeometrics();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorLateral:
        inTrack->anchorNode->forceLateral = fromValue.toDouble();
        hTrack->trackWidgetItem->updateAnchorGeometrics();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorPitchChange:
        hTrack->trackWidgetItem->on_pitchChangeBox_valueChanged(fromValue.toDouble()*F_HZ);
        //inTrack->anchorNode->forceNormal = fromValue.toDouble();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorYawChange:
        hTrack->trackWidgetItem->on_yawChangeBox_valueChanged(fromValue.toDouble()*F_HZ);
        //inTrack->anchorNode->forceLateral = fromValue.toDouble();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorSpeed:
        inTrack->anchorNode->fVel = fromValue.toDouble();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeTrackHeartline:
        inTrack->fHeart = fromValue.toDouble();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        gloParent->updateProjectWidget();
        break;
    case changeTrackFriction:
        inTrack->fFriction = fromValue.toDouble();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        gloParent->updateProjectWidget();
        break;
    case changeTrackResistance:
        inTrack->fResistance = fromValue.toDouble();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        gloParent->updateProjectWidget();
        break;
    case appendSegment:
        hTrack->trackWidgetItem->on_deleteButton_released();
        break;
    case removeSegment:
        temp = readString(info, 3);
        if(temp == "STR")
        {
            hTrack->trackWidgetItem->appendStraightSec();
            inTrack->activeSection->loadSection(*info);
            inTrack->updateTrack(sectionNumber, 0);
        }
        else if(temp == "CUR")
        {
            hTrack->trackWidgetItem->appendCurvedSec();
            inTrack->activeSection->loadSection(*info);
            inTrack->updateTrack(sectionNumber, 0);
        }
        else if(temp == "GEO")
        {
            hTrack->trackWidgetItem->appendGeometricSec();
            inTrack->activeSection->loadSection(*info);
            inTrack->updateTrack(sectionNumber, 0);
        }
        else if(temp == "FRC")
        {
            hTrack->trackWidgetItem->appendForceSec();
            inTrack->activeSection->loadSection(*info);
            inTrack->updateTrack(sectionNumber, 0);
        }
        else if(temp == "BEZ")
        {
            hTrack->trackWidgetItem->appendSection(bezier);
            inTrack->activeSection->loadSection(*info);
            inTrack->updateTrack(sectionNumber, 0);
        }
        delete info;
        hTrack->graphWidgetItem->redrawGraphs();
        info = new std::stringstream();
        inTrack->activeSection->saveSection(*info);
        info->seekp(0);
        break;
    case changeSegmentArgument:
        inTrack->lSections.at(sectionNumber)->bArgument = !inTrack->lSections.at(sectionNumber)->bArgument;
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->updateOptionsFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeSegmentOrientation:
        inTrack->lSections.at(sectionNumber)->bOrientation = !inTrack->lSections.at(sectionNumber)->bOrientation;
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->updateOptionsFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeFunctionStatus:
        if(fromValue.toInt())
        {
            hTrack->graphWidgetItem->selFunc->parent->lock(subfunctionNumber);
        }
        else
        {
            hTrack->graphWidgetItem->selFunc->parent->unlock(subfunctionNumber);
        }
        hTrack->graphWidgetItem->selectionChanged();
        inTrack->updateTrack(sectionNumber, (int)(hTrack->graphWidgetItem->selFunc->minArgument*F_HZ-1.5f)); // SIGSEGV because selFunc was NULL
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    default:
        break;
    }

    gloParent->updateInfoPanel();

    if(nextAction)
    {
        nextAction->doUndo();
    }
}

void undoAction::doRedo()
{
    if(type == onLengthSpin || type == onChangeSpin || type == onArg1 || type == onCenterSpin
            || type == onTensionSpin || type == onTransitionBox || type == removeSubFunction || type == changeFunctionStatus)
    {
        hTrack->trackWidgetItem->setSelection(sectionNumber);
        //parent->on_treeWidget_itemClicked(parent->getTopLevel(sectionNumber+1), 0);
        switch(inFunction)
        {
        case funcRoll:
            hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->rollFunc->funcList[subfunctionNumber]);
            break;
        case funcNormal:
            hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->normForce->funcList[subfunctionNumber]);
            break;
        case funcLateral:
            hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->latForce->funcList[subfunctionNumber]);
            break;
        case funcPitch:
            hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->normForce->funcList[subfunctionNumber]);
            break;
        case funcYaw:
            hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->latForce->funcList[subfunctionNumber]);
            break;
        default:
            break;
        }
    }
    else if(type == appendSubFunction)
    {
        hTrack->trackWidgetItem->setSelection(sectionNumber);
        //parent->on_treeWidget_itemClicked(parent->getTopLevel(sectionNumber+1), 0);
        if(subfunctionNumber > 0)
        {
            switch(inFunction)
            {
            case funcRoll:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->rollFunc->funcList[subfunctionNumber-1]);
                break;
            case funcNormal:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->normForce->funcList[subfunctionNumber-1]);
                break;
            case funcLateral:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->latForce->funcList[subfunctionNumber-1]);
                break;
            case funcPitch:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->normForce->funcList[subfunctionNumber-1]);
                break;
            case funcYaw:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->latForce->funcList[subfunctionNumber-1]);
                break;
            default:
                break;
            }
        }
        else
        {
            switch(inFunction)
            {
            case funcRoll:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->rollFunc->funcList[subfunctionNumber]);
                break;
            case funcNormal:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->normForce->funcList[subfunctionNumber]);
                break;
            case funcLateral:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->latForce->funcList[subfunctionNumber]);
                break;
            case funcPitch:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->normForce->funcList[subfunctionNumber]);
                break;
            case funcYaw:
                hTrack->graphWidgetItem->changeSelection(inTrack->activeSection->latForce->funcList[subfunctionNumber]);
                break;
            default:
                break;
            }
        }
    }
    else if(type == changeSegmentLength || type == changeCurveDirection || type == changeCurveLeadIn || type == changeCurveLeadOut
            || type == changeCurveRadius || type == changeSpeedState || type == changeSegmentSpeed || type == changeSegmentArgument
            || type == changeSegmentOrientation)
    {
        hTrack->trackWidgetItem->setSelection(sectionNumber);
        //parent->on_treeWidget_itemClicked(parent->getTopLevel(sectionNumber+1), 0);
    }
    else if(type == changeAnchorPosX || type == changeAnchorPosY || type == changeAnchorPosZ || type == changeAnchorYaw || type == changeAnchorRoll
            || type == changeAnchorPitch || type == changeAnchorNormal || type == changeAnchorLateral || type == changeAnchorSpeed
            || type == changeTrackHeartline || type == changeTrackFriction || type == changeTrackResistance || type == changeAnchorPitchChange || type == changeAnchorYawChange)
    {
    }
    else if(type == appendSegment)
    {
        hTrack->trackWidgetItem->setSelection(sectionNumber-1);
    }
    else if(type == removeSegment)
    {
        hTrack->trackWidgetItem->setSelection(sectionNumber);
    }

    switch (type)
    {
    case onLengthSpin:
        hTrack->graphWidgetItem->selFunc->parent->changeLength(toValue.toDouble(), subfunctionNumber);
        hTrack->graphWidgetItem->selectionChanged();
        inTrack->updateTrack(sectionNumber, (int)(hTrack->graphWidgetItem->selFunc->minArgument*F_HZ-1.5f));
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case onChangeSpin:
        hTrack->graphWidgetItem->selFunc->symArg = toValue.toDouble();
        hTrack->graphWidgetItem->selectionChanged();
        inTrack->updateTrack(sectionNumber, (int)(hTrack->graphWidgetItem->selFunc->minArgument*F_HZ-1.5f));
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case onArg1:
        hTrack->graphWidgetItem->selFunc->arg1 = toValue.toDouble();
        hTrack->graphWidgetItem->selectionChanged();
        inTrack->updateTrack(sectionNumber, (int)(hTrack->graphWidgetItem->selFunc->minArgument*F_HZ-1.5f));
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case onCenterSpin:
        hTrack->graphWidgetItem->selFunc->centerArg = toValue.toDouble();
        hTrack->graphWidgetItem->selectionChanged();
        inTrack->updateTrack(sectionNumber, (int)(hTrack->graphWidgetItem->selFunc->minArgument*F_HZ-1.5f));
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case onTensionSpin:
        hTrack->graphWidgetItem->selFunc->tensionArg = toValue.toDouble();
        hTrack->graphWidgetItem->selectionChanged();
        inTrack->updateTrack(sectionNumber, (int)(hTrack->graphWidgetItem->selFunc->minArgument*F_HZ-1.5f));
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case onTransitionBox:
        hTrack->graphWidgetItem->selFunc->degree = (eDegree)toValue.toInt();
        hTrack->graphWidgetItem->selectionChanged();
        inTrack->updateTrack(sectionNumber, (int)(hTrack->graphWidgetItem->selFunc->minArgument*F_HZ-1.5f));
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case appendSubFunction:
        if(subfunctionNumber > 0)
        {
            hTrack->graphWidgetItem->transitionHandler->on_appendButton_released();
        }
        else
        {
            hTrack->graphWidgetItem->transitionHandler->on_prependButton_released();
        }
        break;
    case removeSubFunction:
        hTrack->graphWidgetItem->transitionHandler->on_removeButton_released();
        break;
    case changeSegmentLength:
        inTrack->lSections[sectionNumber]->rollFunc->setMaxArgument(toValue.toDouble());
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->setupStraightFrame();
        hTrack->trackWidgetItem->setupCurvedFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeCurveDirection:
        inTrack->lSections.at(sectionNumber)->fDirection = toValue.toDouble();
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->setupCurvedFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeCurveLeadIn:
        inTrack->lSections.at(sectionNumber)->fLeadIn = toValue.toDouble();
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->setupCurvedFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeCurveLeadOut:
        inTrack->lSections.at(sectionNumber)->fLeadOut = toValue.toDouble();
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->setupCurvedFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeCurveRadius:
        inTrack->lSections.at(sectionNumber)->fRadius = toValue.toDouble();
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->setupCurvedFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeSpeedState:
        inTrack->lSections.at(sectionNumber)->bSpeed = toValue.toBool();
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->setupStraightFrame();
        hTrack->trackWidgetItem->setupCurvedFrame();
        hTrack->trackWidgetItem->setupAdvFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeSegmentSpeed:
        inTrack->lSections.at(sectionNumber)->fVel = toValue.toDouble();
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->setupStraightFrame();
        hTrack->trackWidgetItem->setupCurvedFrame();
        hTrack->trackWidgetItem->setupAdvFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorPosX:
        inTrack->startPos.x = toValue.toDouble();
        hTrack->trackWidgetItem->setupAnchorFrame();
        break;
    case changeAnchorPosY:
        inTrack->startPos.y = toValue.toDouble();
        hTrack->trackWidgetItem->setupAnchorFrame();
        inTrack->hasChanged = true;
        break;
    case changeAnchorPosZ:
        inTrack->startPos.z = toValue.toDouble();
        hTrack->trackWidgetItem->setupAnchorFrame();
        break;
    case changeAnchorRoll:
        inTrack->anchorNode->setRoll(toValue.toDouble()-inTrack->anchorNode->fRoll);
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorYaw:
        inTrack->startYaw = toValue.toDouble();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorPitch:
        inTrack->startPitch = toValue.toDouble();
        inTrack->anchorNode->changePitch((float)toValue.toDouble()-inTrack->anchorNode->getPitch(), fabs(inTrack->anchorNode->fRoll) >= 90.f);
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorNormal:
        inTrack->anchorNode->forceNormal = toValue.toDouble();
        hTrack->trackWidgetItem->updateAnchorGeometrics();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorLateral:
        inTrack->anchorNode->forceLateral = toValue.toDouble();
        hTrack->trackWidgetItem->updateAnchorGeometrics();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorPitchChange:
        hTrack->trackWidgetItem->on_pitchChangeBox_valueChanged(toValue.toDouble()*F_HZ);
        //inTrack->anchorNode->forceNormal = toValue.toDouble();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorYawChange:
        hTrack->trackWidgetItem->on_yawChangeBox_valueChanged(toValue.toDouble()*F_HZ);
        //inTrack->anchorNode->forceLateral = toValue.toDouble();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeAnchorSpeed:
        inTrack->anchorNode->fVel = toValue.toDouble();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeTrackHeartline:
        inTrack->fHeart = toValue.toDouble();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        gloParent->updateProjectWidget();
        break;
    case changeTrackFriction:
        inTrack->fFriction = toValue.toDouble();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        gloParent->updateProjectWidget();
        break;
    case changeTrackResistance:
        inTrack->fResistance = toValue.toDouble();
        inTrack->updateTrack(0, 0);
        hTrack->trackWidgetItem->setupAnchorFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        gloParent->updateProjectWidget();
        break;
    case appendSegment:
        switch(toValue.toInt())
        {
        case straight:
            hTrack->trackWidgetItem->appendSection(straight);
            //hTrack->trackWidgetItem->appendStraightSec();
            break;
        case curved:
            hTrack->trackWidgetItem->appendCurvedSec();
            break;
        case forced:
            hTrack->trackWidgetItem->appendForceSec();
            break;
        case geometric:
            hTrack->trackWidgetItem->appendGeometricSec();
            break;
        case bezier:
            hTrack->trackWidgetItem->appendSection(bezier);
            break;
        }
        break;
    case removeSegment:
        hTrack->trackWidgetItem->on_deleteButton_released();
        //parent->removeSec();
        break;
    case changeSegmentArgument:
        inTrack->lSections.at(sectionNumber)->bArgument = !inTrack->lSections.at(sectionNumber)->bArgument;
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->updateOptionsFrame();
        hTrack->trackWidgetItem->updateSectionFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeSegmentOrientation:
        inTrack->lSections.at(sectionNumber)->bOrientation = !inTrack->lSections.at(sectionNumber)->bOrientation;
        inTrack->updateTrack(sectionNumber, 0);
        hTrack->trackWidgetItem->updateOptionsFrame();
        hTrack->trackWidgetItem->updateSectionFrame();
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    case changeFunctionStatus:
        if(toValue.toInt())
        {
            hTrack->graphWidgetItem->selFunc->parent->lock(subfunctionNumber);
        }
        else
        {
            hTrack->graphWidgetItem->selFunc->parent->unlock(subfunctionNumber);
        }
        hTrack->graphWidgetItem->selectionChanged();
        inTrack->updateTrack(sectionNumber, (int)(hTrack->graphWidgetItem->selFunc->minArgument*F_HZ-1.5f));
        hTrack->graphWidgetItem->redrawGraphs();
        break;
    default:
        break;
    }

    gloParent->updateInfoPanel();

    if(nextAction)
    {
        nextAction->doRedo();
    }
}
