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

#include "graphhandler.h"
#include "QTreeWidgetItem"
#include "trackhandler.h"
#include "lenassert.h"
#include "qcustomplot.h"

graphHandler::graphHandler(QTreeWidgetItem* _treeItem, enum graphType _type, trackHandler* _track, QCPAxis* _axis, QColor* _color)
{
    treeItem = _treeItem;
    active = false;
    drawn = false;

    for(int i = 0; i < 4; ++i) {
        if(_color) {
            color[i] = _color[i];
        } else {
            color[i] = QColor(0, 0, 0, 0);
        }
    }
    mType = _type;
    mTrack = _track;
    usedAxis = _axis;

    prevCheckState = Qt::Unchecked;

    QColor backColor(color[0]);
    backColor.setAlpha(backColor.alpha()/4);

    treeItem->setBackgroundColor(0, backColor);
    treeItem->setBackgroundColor(1, backColor);
}

void graphHandler::fillGraphList(QCPAxis* xAxis, bool _argument, bool _orientation, bool _drawExterns)
{
    const unsigned max_segments_per_transition = 10000;

    mnode* curNode = mTrack->trackData->anchorNode, *prevNode = NULL;
    track* curTrack = mTrack->trackData;
    QVector<double> x, y;

    while(graphList.size() > 0) {
        delete graphList[0];
        graphList.removeAt(0);
    }

    if(mType == secBoundaries) {
        if(mTrack->trackData->activeSection) fillBoundaryGraphList(xAxis, _argument);
        return;
    }

    for(int i = 0; i < mTrack->trackData->lSections.size(); ++i) {
        x.clear();
        y.clear();
        if(active && mTrack->trackData->activeSection == mTrack->trackData->lSections[i]) {
            fillActiveGraphList(xAxis, _argument, _drawExterns);
            continue;
        }
        else if(!_drawExterns && mTrack->trackData->activeSection != mTrack->trackData->lSections[i]) {
            continue;
        }

        QCPGraph* curGraph = new QCPGraph(xAxis, usedAxis);

        graphList.append(curGraph);

        /*
         *  Set Colors and Style of the new graph
         */
        if(mType == nForceChange || mType == lForceChange || mType == banking || mType == rollAccel){
            curGraph->setPen(QPen(color[0], 1, Qt::DashDotLine));
            QColor brush = color[1];
            brush.setAlpha(brush.alpha()/3);
            curGraph->setBrush(QBrush(brush));
        } else if(mType == smoothedRollSpeed || mType == smoothedNForce || mType == smoothedLForce) {
            QColor temp = color[0];
            temp.setAlpha(temp.alpha()/3);
            curGraph->setPen(QPen(temp, 1, Qt::DotLine));
            temp = color[1];
            temp.setAlpha(temp.alpha()/3);
            curGraph->setBrush(QBrush(temp));
        } else if(active) {
            curGraph->setPen(QPen(color[0], 1));
            curGraph->setBrush(QBrush(color[1]));
        } else {
            curGraph->setPen(QPen(color[0], 1, Qt::DashDotLine));
            QColor brush = color[1];
            brush.setAlpha(brush.alpha()/3);
            curGraph->setBrush(QBrush(brush));
        }

        section* curSection = mTrack->trackData->lSections[i];

        unsigned int n1 = curTrack->getNumPoints(curSection);
        unsigned int n2 = n1 + curSection->lNodes.size()-1;
        unsigned int step = (n2-n1)/max_segments_per_transition;
        step = step < 50 ? 50 : step;

        if(curTrack->lSections.size()-1 == i && n2) {
            --n2;
        }
        unsigned int diff;

        for(unsigned int j = n1; j < n2+step; j+=step) {
            j = j > n2+step ? n2 : j;

            if(j > 19) {
                prevNode = curTrack->getPoint(j-20);
                diff = 20;
            } else {
                prevNode = curTrack->getPoint(j);
                diff = 1;
            }

            curNode = curTrack->getPoint(j);
            if(_argument == TIME) {
                x.append(j/F_HZ);
            } else {
                x.append(curNode->fTotalLength);
            }

            if(_argument == TIME) {
                switch(mType) {
                case banking:
                    y.append(curNode->fRoll);
                    break;
                case rollSpeed:
                    if(_orientation == QUATERNION) {
                        y.append(curNode->fRollSpeed);
                    } else {
                        y.append(curNode->fRollSpeed - glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f))*curNode->getYawChange());
                    }
                    break;
                case smoothedRollSpeed:
                    if(_orientation == QUATERNION) {
                        y.append(curNode->fRollSpeed + curNode->fSmoothSpeed);
                    } else {
                        y.append(curNode->fRollSpeed + curNode->fSmoothSpeed - glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f))*curNode->getYawChange());
                    }
                    break;
                case rollAccel:
                    if(_orientation == QUATERNION) {
                        y.append((curNode->fRollSpeed + curNode->fSmoothSpeed - prevNode->fRollSpeed - prevNode->fSmoothSpeed)*F_HZ/diff);
                    } else {
                        y.append((curNode->fRollSpeed + curNode->fSmoothSpeed - prevNode->fRollSpeed - prevNode->fSmoothSpeed - glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f))*curNode->getYawChange() + glm::dot(prevNode->vDir, glm::vec3(0.f, -1.f, 0.f))*prevNode->getYawChange())*F_HZ/diff);
                    }
                    break;
                case nForce:
                    y.append(curNode->forceNormal);
                    break;
                case smoothedNForce:
                    y.append(curNode->forceNormal + curNode->smoothNormal);
                    break;
                case nForceChange:
                    y.append((curNode->forceNormal + curNode->smoothNormal - prevNode->forceNormal - prevNode->smoothNormal)*F_HZ/diff);
                    break;
                case lForce:
                    y.append(curNode->forceLateral);
                    break;
                case smoothedLForce:
                    y.append(curNode->forceLateral + curNode->smoothLateral);
                    break;
                case lForceChange:
                    y.append((curNode->forceLateral + curNode->smoothLateral - prevNode->forceLateral - prevNode->smoothLateral)*F_HZ/diff);
                    break;
                case pitchChange:
                    y.append(curNode->getPitchChange());
                    break;
                case yawChange:
                    y.append(curNode->getYawChange());
                    break;
                default:
                    break;
                }
            } else {
                double diff = curNode->fTotalLength - prevNode->fTotalLength;
                if(diff < std::numeric_limits<double>::epsilon()){
                    diff = 0.0001;
                }
                switch(mType) {
                case banking:
                    y.append(curNode->fRoll);
                    break;
                case rollSpeed:
                    if(_orientation == QUATERNION) {
                        y.append(curNode->fRollSpeed/curNode->fVel);
                    } else {
                        y.append((curNode->fRollSpeed - glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f))*curNode->getYawChange())/curNode->fVel);
                    }
                    break;
                case smoothedRollSpeed:
                    if(_orientation == QUATERNION) {
                        y.append((curNode->fRollSpeed + curNode->fSmoothSpeed)/curNode->fVel);
                    } else {
                        y.append((curNode->fRollSpeed + curNode->fSmoothSpeed - glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f))*curNode->getYawChange())/curNode->fVel);
                    }
                    break;
                case rollAccel:
                    if(_orientation == QUATERNION) {
                        y.append((curNode->fRollSpeed + curNode->fSmoothSpeed - prevNode->fRollSpeed - prevNode->fSmoothSpeed)/curNode->fVel/diff);
                    } else {
                        y.append((curNode->fRollSpeed + curNode->fSmoothSpeed - prevNode->fRollSpeed - prevNode->fSmoothSpeed - glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f))*curNode->getYawChange() + glm::dot(prevNode->vDir, glm::vec3(0.f, -1.f, 0.f))*prevNode->getYawChange())/curNode->fVel/diff);
                    }
                    break;
                case nForce:
                    y.append(curNode->forceNormal);
                    break;
                case smoothedNForce:
                    y.append(curNode->forceNormal + curNode->smoothNormal);
                    break;
                case nForceChange:
                    y.append((curNode->forceNormal + curNode->smoothNormal - prevNode->forceNormal - prevNode->smoothNormal)/diff);
                    break;
                case lForce:
                    y.append(curNode->forceLateral);
                    break;
                case smoothedLForce:
                    y.append(curNode->forceLateral + curNode->smoothLateral);
                    break;
                case lForceChange:
                    y.append((curNode->forceLateral + curNode->smoothLateral - prevNode->forceLateral - prevNode->smoothLateral)/diff);
                    break;
                case pitchChange:
                    y.append(curNode->getPitchChange()/curNode->fVel);
                    break;
                case yawChange:
                    y.append(curNode->getYawChange()/curNode->fVel);
                    break;
                default:
                    break;
                }
            }

            if(y.last() != y.last()) {
                y.pop_back();
                x.pop_back();
            }
        }
        curGraph->setData(x, y);
        curGraph->setSelectable(false);
    }
}

void graphHandler::fillActiveGraphList(QCPAxis *xAxis, bool _argument, bool _drawExterns)
{
    const unsigned max_segs_per_active_transition = 160;

    QCPGraph* curGraph;
    track* curTrack = mTrack->trackData;
    QVector<double> x, y;
    func* func;

    double n = curTrack->getNumPoints(curTrack->activeSection)/F_HZ;

    switch(mType) {
    case rollSpeed:
        func = curTrack->activeSection->rollFunc;
        break;
    case pitchChange:
    case nForce:
        func = curTrack->activeSection->normForce;
        break;
    case yawChange:
    case lForce:
        func = curTrack->activeSection->latForce;
        break;
    default:
        func = NULL;
        qWarning("Bad Type to draw Active Graph.");
        break;
    }

    lenAssert(func != NULL);
    if(func == NULL) return;

    int lower = 0, upper = 0;
    for(int i = 0; i < func->funcList.size(); ++i) {
        x.clear();
        y.clear();

        subfunc* curFunc = func->funcList[i];

        if(curFunc->locked) curFunc->getValue(-1.f); // make sure maxArg is right

        curGraph = new QCPGraph(xAxis, usedAxis);

        graphList.append(curGraph);

        curGraph->setPen(QPen(color[0], 1));
        curGraph->setBrush(QBrush(color[1]));
        curGraph->setSelectedPen(QPen(color[2], 2));
        curGraph->setSelectedBrush(QBrush(color[3]));

        double key;
        if(curTrack->activeSection->type == straight || curTrack->activeSection->type == curved || curFunc->degree == tozero) {
            int l = upper, r = curTrack->activeSection->lNodes.size()-1;
            if(curFunc->degree == tozero) {
                upper = curFunc->minArgument*F_HZ;
                r = curFunc->maxArgument*F_HZ;
                if(r > curTrack->getNumPoints()) {
                    r = curTrack->getNumPoints();
                }
                if(r > curTrack->activeSection->lNodes.size()-1) {
                    r = curTrack->activeSection->lNodes.size()-1;
                }
            }
            lower = upper;

            if(curTrack->activeSection->isInFunction(r, curFunc)) l = r;
            while(l < r) {
                if(curTrack->activeSection->isInFunction((l+r)/2, curFunc)) {
                    l = (l+r)/2+1;
                } else {
                    r = (l+r)/2-1;
                }
            }
            if(r < 0) r=0;
            upper = r;

            for(unsigned int j = 0; j < max_segs_per_active_transition+1; ++j) {
                key = (upper*j + lower*(max_segs_per_active_transition-j))/(double)max_segs_per_active_transition;

                if(_argument == TIME) {
                    x.append(key/F_HZ+n);
                } else {
					x.append(curTrack->activeSection->lNodes[key].fTotalLength);
                }

                switch(mType)
                {
                case rollSpeed:
                    if(curTrack->activeSection->bOrientation == EULER) {
						y.append(curTrack->activeSection->lNodes[key].fRollSpeed + sin(curTrack->activeSection->lNodes[key].getPitch()*F_PI/180.)*curTrack->activeSection->lNodes[key].getYawChange());
                    } else {
						y.append(curTrack->activeSection->lNodes[key].fRollSpeed);
                    }
                    break;
                case nForce:
					y.append(curTrack->activeSection->lNodes[key].forceNormal);
                    break;
                case lForce:
					y.append(curTrack->activeSection->lNodes[key].forceLateral);
                    break;
                case pitchChange:
					y.append(curTrack->activeSection->lNodes[key].getPitchChange());
                    break;
                case yawChange:
					y.append(curTrack->activeSection->lNodes[key].getYawChange());
                    break;
                default:
                    break;
                }
            }
        } else {
            if(_argument == DISTANCE) {
				n = curTrack->activeSection->lNodes[0].fTotalLength;
            }
            for(int j = 0; j < 251; ++j) {
                double maxArg;
                if(curTrack->activeSection == curTrack->lSections.last() || !_drawExterns) {
                    maxArg = curFunc->maxArgument;
                } else {
                    maxArg= curTrack->activeSection->getMaxArgument() < curFunc->maxArgument ? curTrack->activeSection->getMaxArgument() : curFunc->maxArgument;
                }
                if(maxArg < curFunc->minArgument) {
                    curGraph->setProperty("p", qVariantFromValue((void*)curFunc));
                    curGraph->setData(x, y);
                    curGraph->setSelectable(true);
                    return;
                }
                //double maxArg = curFunc->maxArgument;
                key = maxArg*j/250 + curFunc->minArgument*(250-j)/250;
                x.append(key+n);
                y.append(curFunc->getValue(key));
            }

            if(y.last() != y.last()) {
                y.pop_back();
                x.pop_back();
            }
        }

        if(x.isEmpty()) {
            x.append(curFunc->maxArgument);
            y.append(0.);
        }
        curGraph->setProperty("p", qVariantFromValue((void*)curFunc));
        curGraph->setData(x, y);
        curGraph->setSelectable(true);
    }
}

void graphHandler::fillBoundaryGraphList(QCPAxis *xAxis, bool _argument)
{
    QCPGraph* curGraph;
    QVector<double> x, y;
    track* curTrack = mTrack->trackData;
    for(int i = 0; i < curTrack->lSections.size(); ++i) {
        x.clear();
        y.clear();

        curGraph = new QCPGraph(xAxis, usedAxis);
        graphList.append(curGraph);

        curGraph->setPen(QPen(QColor(0, 0, 0, 150), 1, Qt::DashDotLine));
        curGraph->setBrush(QBrush(QColor(0, 0, 0, 20)));

        double n1, n2;
        if(_argument == TIME) {
            n1 = curTrack->getNumPoints(mTrack->trackData->lSections[i])/F_HZ;
            n2 = n1 + (mTrack->trackData->lSections[i]->lNodes.size()-1)/F_HZ;
        } else {
			n1 = curTrack->lSections[i]->lNodes[0].fTotalLength;
			n2 = curTrack->lSections[i]->lNodes.last().fTotalLength;
        }
        //if(curTrack->lSections.size()-1 == i) --n2;

        if(curTrack->lSections[i] != curTrack->activeSection) {
            if(i == 0) {
                x.append(0.);
                y.append(0.);
            }
            x.append(n1 + 0.00001);
            y.append(20000.);
        }
        x.append(n2 - 0.00001);
        y.append(20000.);
        x.append(n2);
        y.append(0.);

        curGraph->setData(x, y);
        curGraph->setSelectable(false);

        x.clear();
        y.clear();

        curGraph = new QCPGraph(xAxis, usedAxis);
        graphList.append(curGraph);

        curGraph->setPen(QPen(QColor(0, 0, 0, 150), 1, Qt::DashDotLine));
        curGraph->setBrush(QBrush(QColor(0, 0, 0, 20)));
        if(curTrack->lSections[i] != curTrack->activeSection) {
            if(i == 0) {
                x.append(0.);
                y.append(0.);
            }
            x.append(n1 + 0.00001);
            y.append(-20000.);
        }
        x.append(n2 - 0.00001);
        y.append(-20000.);
        x.append(n2);
        y.append(0.);

        curGraph->setData(x, y);
        curGraph->setSelectable(false);
    }
    return;
}

