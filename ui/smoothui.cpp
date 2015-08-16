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

#include "smoothui.h"
#include "ui_smoothui.h"
#include "smoothhandler.h"
#include "mainwindow.h"
#include "trackwidget.h"
#include "lenassert.h"

smoothUi::smoothUi(trackHandler* _track, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::smoothUi)
{
    setWindowFlags(windowFlags() | Qt::CustomizeWindowHint);
    setWindowFlags(windowFlags() & (~Qt::WindowMinimizeButtonHint));
    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));

#ifdef Q_OS_MAC // Damit es im Vordergrund bleibt, evtl. für windows auch sinnvoll, probiere es einfach mal unter windows aus
    setWindowFlags(windowFlags() | Qt::Popup);
#endif

    ui->setupUi(this);

    ui->smoothUnitTree->setItemDelegateForColumn(0, new NoEditDelegate(this));
    ui->smoothUnitTree->setItemDelegateForColumn(2, new NoEditDelegate(this));
    ui->smoothUnitTree->setItemDelegateForColumn(3, new NoEditDelegate(this));
    ui->smoothUnitTree->setItemDelegateForColumn(4, new NoEditDelegate(this));
    ui->smoothUnitTree->setItemDelegateForColumn(5, new NoEditDelegate(this));
    ui->smoothUnitTree->setItemDelegateForColumn(6, new NoEditDelegate(this));


    m_trackwidget = _track->trackWidgetItem;
    m_track = _track->trackData;
    m_widget = _track->graphWidgetItem;

    ui->smoothUnitTree->setColumnWidth(0, 28);
    ui->smoothUnitTree->setColumnWidth(1, 120);
    ui->smoothUnitTree->setColumnWidth(2, 60);
    ui->smoothUnitTree->setColumnWidth(3, 60);
    ui->smoothUnitTree->setColumnWidth(4, 60);
    ui->smoothUnitTree->setColumnWidth(5, 60);
    ui->smoothUnitTree->setColumnWidth(6, 60);

    ui->optsFrame->hide();
    ui->regionFrame->hide();
    ui->removeButton->setEnabled(false);

    ui->upButton->hide();
    ui->downButton->hide();

    curHandler = NULL;

    ui->warningLabel->setText("");
    ui->warningLabel->hide();

    customChar = 'a';

    phantomChanges = false;
}

smoothUi::~smoothUi()
{
    delete ui;
}

void smoothUi::applyRollSmooth(int fromNode)
{
    m_track->removeSmooth(fromNode);

    m_track->anchorNode->fRollSpeed = 0.0;

    int sec, curNode = fromNode < 0 ? 0 : fromNode;
    for(sec = 0; sec < m_track->lSections.size(); ++sec) {
        if(m_track->lSections[sec]->lNodes.size() >= curNode) {
            break;
        }
        curNode -= m_track->lSections[sec]->lNodes.size()-1;
    }

    for(; sec < m_track->lSections.size(); ++sec) {
        section* curSection = m_track->lSections[sec];
        for(int i = curNode; i < curSection->lNodes.size(); ++i) {
			curSection->lNodes[i].fSmoothSpeed = 0.f;
        }
        curNode = 0;
    }

    for(int i = 0; i < m_track->smoothList.size(); ++i) {
        smoothHandler* cur = m_track->smoothList[i];
        if(cur->active == false) continue;

        if(cur->getTo() > fromNode) {
            applyRollSmoothFilter(cur);
        }
    }

    if(active()) {
        m_track->applySmooth(fromNode);
    }
    m_widget->redrawGraphs();
    m_track->hasChanged = true;
    return;
}

void smoothUi::applyRollSmoothFilter(smoothHandler* _handler)
{
    const int iter = _handler->getIterations();
    const int length = _handler->getLength()/iter;
    const int fromNode = _handler->getFrom();
    const int toNode = _handler->getTo();

    QVector<double> adjustValues;

    QVector<double> *cur = new QVector<double>();
    QVector<double> *last = new QVector<double>();
    QVector<double> orig;
    QVector<double> *swap;

    mnode* curNode;

    if(toNode - fromNode - length/2*iter < 0) {
        lenAssert(0 && "Smoothing not possible");
        return;
    }

    double lastValue = 0., firstValue = 0.;
    for(int i = 0; i <= length/2*iter; ++i) {
        curNode = m_track->getPoint(toNode - i);
        lastValue += curNode->fRollSpeed + curNode->fSmoothSpeed;
        curNode =  m_track->getPoint(fromNode + i);
        firstValue += curNode->fRollSpeed + curNode->fSmoothSpeed;
    }
    lastValue /= length/2*iter + 1;
    firstValue /= length/2*iter + 1;

    for(int i = fromNode; i < toNode; ++i) {
        if(length == 0) {
            curNode = m_track->getPoint(i);
            cur->append(curNode->fRollSpeed + curNode->fSmoothSpeed);
            last->append(cur->last());
            orig.append(cur->last());
            continue;
        }
        double t1 = (i - fromNode - length/2.*iter)/(length/2. * iter);
        double t2 = (toNode - length/2.*iter - i)/(length/2. * iter);
        if(t1 < 0) t1 = 1.;
        else t1 = exp(-2*t1*t1);
        if(t2 < 0) t2 = 1.;
        else t2 = exp(-2*t2*t2);
        double t = (1. - t1)*(1. - t2);
        if(t != t) t = 0.;

        if(t2 > t1) {
            if(t > t2) { // max = t
                if(fabs(t1+t2) > std::numeric_limits<double>::epsilon()) {
                    t2 = t2/(t1+t2)*(1.-t);
                    t1 = t1/(t1+t2)*(1.-t);
                }
            } else {   // max = t2
                if(fabs(t1+t) > std::numeric_limits<double>::epsilon()) {
                    t = t/(t1+t)*(1.-t2);
                    t1 = t1/(t1+t)*(1.-t2);
                }
            }
        } else {
            if(t > t1) { // max = t
                if(fabs(t1+t2) > std::numeric_limits<double>::epsilon()) {
                    t2 = t2/(t1+t2)*(1.-t);
                    t1 = t1/(t1+t2)*(1.-t);
                }
            } else {   // max = t1
                if(fabs(t+t2) > std::numeric_limits<double>::epsilon()) {
                    t = t/(t2+t)*(1.-t1);
                    t2 = t2/(t2+t)*(1.-t1);
                }
            }
        }
        if(i < fromNode + length/2 * iter) {
            cur->append(firstValue);
        } else if(i > toNode - length/2*iter) {
            cur->append(lastValue);
        } else {
            curNode = m_track->getPoint(i);
            cur->append(t*(curNode->fRollSpeed + curNode->fSmoothSpeed) + t1*firstValue + t2*lastValue);
        }
        last->append(cur->last());
        orig.append(cur->last());
    }

    for(int iterations = 0; iterations < iter; ++iterations) {
        swap = cur;
        cur = last;
        last = swap;
        for(int i = 0; i < cur->size(); ++i) {
            double temp = 0.0, div = 0.;

            for(int j = -length/2; j <= length/2; ++j) {
                if(i+j < 0) {
                    temp += last->at(0);
                } else if(i+j >= last->size()) {
                    temp += last->last();
                } else {
                    temp += last->at(i+j);
                }
                div = length/2 * 2 + 1;
            }
            cur->replace(i, temp/div);
        }
    }

    for(int i = 0; i < cur->size(); ++i) {
        adjustValues.append(cur->at(i) - orig[i]);
    }

    for(int i = 0; i < adjustValues.size(); ++i) {
        m_track->getPoint(i+fromNode)->fSmoothSpeed += adjustValues[i];
    }

    delete cur;
    delete last;
}

bool smoothUi::active()
{
    for(int i = 0; i < m_track->smoothList.size(); ++i) {
        if(m_track->smoothList[i]->active) return true;
    }
    return false;
}

void smoothUi::on_buttonBox_accepted()
{
    applyRollSmooth();
}

void smoothUi::updateUi()
{
    customChar = 'a';
    for(int i = 0; i < m_track->smoothList.size(); ++i) {
        m_track->smoothList[i]->update(&customChar);
        if(!m_track->smoothList[i]->treeItem->parent()) {
            ui->smoothUnitTree->insertTopLevelItem(i, m_track->smoothList[i]->treeItem);
        }
    }
}

void smoothUi::on_smoothUnitTree_itemSelectionChanged()
{
    bool oldP = phantomChanges;
    phantomChanges = true;

    if(ui->smoothUnitTree->selectedItems().size() == 0) {
        ui->optsFrame->hide();
        ui->regionFrame->hide();
        ui->removeButton->setEnabled(false);
        return;
    }

    QTreeWidgetItem* selected = ui->smoothUnitTree->selectedItems().at(0);
    int i;
    for(i = 0; i < m_track->smoothList.size(); ++i) {
        if(m_track->smoothList[i]->treeItem == selected) break;
    }
    curHandler = m_track->smoothList[i];
    if(m_track->smoothList[i]->sec != NULL) {
        ui->lengthBox->setValue(curHandler->getLength()/1000.);
        ui->iterBox->setValue(curHandler->getIterations());
        ui->optsFrame->show();
        ui->regionFrame->hide();
        ui->removeButton->setEnabled(false);
    } else {
        ui->lengthBox->setValue(curHandler->getLength()/1000.);
        ui->iterBox->setValue(curHandler->getIterations());
        ui->fromBox->setValue(curHandler->getFrom()/1000.);
        ui->toBox->setValue(curHandler->getTo()/1000.);
        ui->optsFrame->show();
        ui->regionFrame->show();
        ui->removeButton->setEnabled(true);
    }

    phantomChanges = oldP;
}

void smoothUi::on_lengthBox_valueChanged(double arg1)
{
    if(phantomChanges) return;
    phantomChanges = true;
    curHandler->setLength((int)(arg1*1000.+0.5));
    generateWarnings();
    phantomChanges = false;
}

void smoothUi::on_iterBox_valueChanged(int arg1)
{
    if(phantomChanges) return;
    phantomChanges = true;
    curHandler->setIterations(arg1);
    generateWarnings();
    phantomChanges = false;
}

void smoothUi::on_fromBox_valueChanged(double arg1)
{
    if(phantomChanges) return;
    const int max = m_track->getNumPoints();
    const int min = 0;
    int setTo = (int)(arg1*1000.+0.5);
    if(setTo > max) {
        ui->fromBox->setValue(max/1000.);
        return;
    }
    if(setTo < min) {
        ui->fromBox->setValue(min/1000.);
        return;
    }

    phantomChanges = true;
    curHandler->setFrom((int)(arg1*1000.+0.5));
    generateWarnings();
    phantomChanges = false;
}

void smoothUi::on_toBox_valueChanged(double arg1)
{
    if(phantomChanges) return;
    int setTo = (int)(arg1*1000.+0.5);
    const int max = m_track->getNumPoints();
    const int min = 0;
    if(setTo > max) {
        ui->toBox->setValue(max/1000.);
        return;
    }
    if(setTo < min) {
        ui->toBox->setValue(min/1000.);
        return;
    }

    phantomChanges = true;
    curHandler->setTo(setTo);
    generateWarnings();
    phantomChanges = false;
}

void smoothUi::on_smoothUnitTree_itemChanged(QTreeWidgetItem *item, int column)
{
    if(phantomChanges) return;
    phantomChanges = true;

    if(column == 6) {
        int i;
        for(i = 0; i < m_track->smoothList.size(); ++i) {
            if(m_track->smoothList[i]->treeItem == item) break;
        }
        m_track->smoothList[i]->active = (item->checkState(6) == Qt::Checked);
    }

    generateWarnings();
    phantomChanges = false;
}

void smoothUi::on_newButton_released()
{
    m_track->smoothList.append(new smoothHandler(m_track, -2, &customChar, 400, 1, 0, 1000));
    updateUi();
}

void smoothUi::on_removeButton_released()
{
    int i;
    for(i = 0; i < m_track->smoothList.size(); ++i) {
        if(m_track->smoothList[i] == curHandler) break;
    }
    delete curHandler;
    m_track->smoothList.removeAt(i);
}

void smoothUi::generateWarnings()
{
    QString str;
    for(int i = 0; i < m_track->smoothList.size(); ++i) {
        if(m_track->smoothList[i]->active) {
            smoothHandler* cur = m_track->smoothList[i];
            if(cur->getFrom() > cur->getTo()) {
                str.append(QString("Warning: Smoothing item \"").append(cur->treeItem->text(1)).append("\" (").append(cur->treeItem->text(0)).append(") ").append("is set to begin before it ends.\n"));
            } else if(2.f*cur->getLength() > cur->getTo() - cur->getFrom()) {
                str.append(QString("Warning: Smoothing item \"").append(cur->treeItem->text(1)).append("\" (").append(cur->treeItem->text(0)).append(") ").append("might be too short for its current filter length.\n"));
            }
        }
    }


    if(str.isEmpty()) {
        ui->warningLabel->hide();
    } else {
        ui->warningLabel->setText(str);
        ui->warningLabel->show();
    }
}
