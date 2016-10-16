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

#include "transitionwidget.h"
#include "optionsmenu.h"
#include "ui_transitionwidget.h"
#include "undohandler.h"
#include "mainwindow.h"
#include "lenassert.h"
#include "trackwidget.h"

extern MainWindow* gloParent;

transitionWidget::transitionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::transitionWidget)
{
    ui->setupUi(this);
    selectedFunc = NULL;
    phantomChanges = false;
}

transitionWidget::~transitionWidget()
{
    delete ui;
}

void transitionWidget::changeSubfunc(subfunc *_newSubfunc)
{
    phantomChanges = true;
    selectedFunc = _newSubfunc;
    gloParent->selectedFunc = selectedFunc;
    if(selectedFunc == NULL) {
        phantomChanges = false;
        return;
    }
    if(_newSubfunc->parent->secParent->type == curved) {
        ui->lengthSpin->setValue(selectedFunc->maxArgument-selectedFunc->minArgument);
        ui->lengthSpin->setSuffix(QString("%1").arg(QChar(0xb0)));
    } else if(_newSubfunc->parent->secParent->bArgument == TIME && _newSubfunc->parent->secParent->type != straight) {
        ui->lengthSpin->setValue(selectedFunc->maxArgument-selectedFunc->minArgument);
        ui->lengthSpin->setSuffix(QString("s"));
    } else {
        ui->lengthSpin->setValue((selectedFunc->maxArgument-selectedFunc->minArgument)*gloParent->mOptions->getLengthFactor());
        ui->lengthSpin->setSuffix(gloParent->mOptions->getLengthString());
    }

    if(_newSubfunc->parent->secParent->bArgument == TIME) {
        ui->changeSpin->setValue(selectedFunc->symArg);
        switch(_newSubfunc->parent->type) {
        case funcNormal:
        case funcLateral:
            ui->changeSpin->setSuffix("g");
            break;
        case funcRoll:
        case funcPitch:
        case funcYaw:
            ui->changeSpin->setSuffix(QString("%1/s").arg(QChar(0xb0)));
            break;
        default:
            lenAssert(0 && "unhandeled case");
            break;
        }
    } else {
        switch(_newSubfunc->parent->type) {
        case funcNormal:
        case funcLateral:
            ui->changeSpin->setValue(selectedFunc->symArg);
            ui->changeSpin->setSuffix("g");
            break;
        case funcRoll:
        case funcPitch:
        case funcYaw:
            ui->changeSpin->setValue(selectedFunc->symArg / gloParent->mOptions->getLengthFactor());
            ui->changeSpin->setSuffix(QString("%1/").arg(QChar(0xb0)).append(gloParent->mOptions->getLengthString()));
            break;
        default:
            lenAssert(0 && "unhandeled case");
            break;
        }
    }

    // HACK: is there a more stable way to do this?
	/*if((_newSubfunc->parent->secParent->type == forced || _newSubfunc->parent->secParent->type == geometric)
      && _newSubfunc->parent->type == funcRoll
      && _newSubfunc->parent->secParent->bArgument == TIME
      && ui->transitionBox->count() == 7)
    {
        ui->transitionBox->addItem("ToZero (experimental)");
    }

    if((_newSubfunc->parent->secParent->type == curved || _newSubfunc->parent->secParent->type == straight || _newSubfunc->parent->type != funcRoll || _newSubfunc->parent->secParent->bArgument == DISTANCE)
      && ui->transitionBox->count() == 8)
    {
        ui->transitionBox->removeItem(7);
	}*/


    ui->buttonFrame->show();
    ui->changeSpin->setEnabled(true);
    switch(selectedFunc->degree) {
    case linear:
        ui->quadraticFrame->hide();
        ui->quarticFrame->hide();
        ui->quinticFrame->hide();
        ui->timewarpFrame->show();
        break;
    case quadratic:
        ui->quadraticFrame->show();
        ui->quarticFrame->hide();
        ui->quinticFrame->hide();
        ui->timewarpFrame->show();
        if(selectedFunc->isSymmetric()) {
            ui->quadraticBox->setCurrentIndex(2);
        } else if(selectedFunc->arg1 > 0.f) {
            ui->quadraticBox->setCurrentIndex(0);
        } else {
            ui->quadraticBox->setCurrentIndex(1);
        }
        break;
    case cubic:
        ui->quadraticFrame->hide();
        ui->quarticFrame->hide();
        ui->quinticFrame->hide();
        ui->timewarpFrame->show();
        break;
    case quartic:
        ui->quadraticFrame->hide();
        ui->quarticFrame->show();
        ui->quinticFrame->hide();
        ui->timewarpFrame->show();
        if(selectedFunc->isSymmetric()) {
            ui->quarticBox->setCurrentIndex(0);
            ui->quarticSpin->setVisible(false);
        } else if(selectedFunc->arg1 > 0.5) {
            ui->quarticSpin->setValue(  (0.5f/(selectedFunc->arg1 - 0.5f)-1)/5.f  );
            ui->quarticBox->setCurrentIndex(2);
            ui->quarticSpin->setVisible(true);
        } else if(selectedFunc->arg1 >= 0) {
            ui->quarticSpin->setValue((0.5f/(0.5f - selectedFunc->arg1)-1)/5.f);
            ui->quarticBox->setCurrentIndex(1);
            ui->quarticSpin->setVisible(true);
        }
        break;
    case quintic:
        ui->quadraticFrame->hide();
        ui->quarticFrame->hide();
        ui->quinticFrame->show();
        ui->timewarpFrame->show();
        if(!selectedFunc->isSymmetric()) {
            ui->quinticSpin->setVisible(false);
            ui->quinticBox->setCurrentIndex(0);
        } else if(selectedFunc->arg1 < 0) {
            ui->quinticSpin->setVisible(true);
            ui->quinticSpin->setValue(fabs(selectedFunc->arg1));
            ui->quinticBox->setCurrentIndex(1);
        } else {
            ui->quinticSpin->setVisible(true);
            ui->quinticSpin->setValue(fabs(selectedFunc->arg1));
            ui->quinticBox->setCurrentIndex(2);
        }
        break;
    case sinusoidal:
    case plateau:
        ui->quadraticFrame->hide();
        ui->quarticFrame->hide();
        ui->quinticFrame->hide();
        ui->timewarpFrame->show();
        break;
    case freeform:
        ui->quadraticFrame->hide();
        ui->quarticFrame->hide();
        ui->quinticFrame->hide();
        ui->timewarpFrame->hide();
        break;
    case tozero:
        ui->quadraticFrame->hide();
        ui->quarticFrame->hide();
        ui->quinticFrame->hide();
        ui->timewarpFrame->hide();
        ui->changeSpin->setEnabled(false);
        break;
    }
    ui->transitionBox->setCurrentIndex(selectedFunc->degree);
    ui->centerSpin->setValue(selectedFunc->centerArg);
    ui->tensionSpin->setValue(selectedFunc->tensionArg);

    if(selectedFunc->parent->secParent->isLockable(selectedFunc->parent)) {
        ui->lockCheck->setEnabled(true);
        ui->lockCheck->setChecked(selectedFunc->locked);
    } else {
        ui->lockCheck->setEnabled(false);
        ui->lockCheck->setChecked(false);
    }

    if(ui->lockCheck->isChecked()) {
        ui->lengthSpin->setEnabled(false);
    } else {
        ui->lengthSpin->setEnabled(true);
    }

    if(selectedFunc->parent->funcList.size() == 1) {
        ui->removeButton->setEnabled(false);
    } else {
        ui->removeButton->setEnabled(true);
    }

    switch(selectedFunc->parent->type) {
    case funcRoll:
        ui->changeSpin->setSingleStep(1.);
        break;
    case funcNormal:
        ui->changeSpin->setSingleStep(0.1);
        break;
    case funcLateral:
        ui->changeSpin->setSingleStep(0.1);
        break;
    case funcPitch:
        ui->changeSpin->setSingleStep(1.);
        break;
    case funcYaw:
        ui->changeSpin->setSingleStep(1.);
        break;
    }

    float diff = selectedFunc->maxArgument - mParent->selTrack->trackData->activeSection->getMaxArgument();

    if(diff > 0) {
        ui->errLabel->setText(QString("Warning: Transition is ").append(QString().number(diff+0.0001, 'f', 2)).append(" longer than Section!"));
        ui->errLabel->show();
    } else {
        ui->errLabel->hide();
    }

    mParent->selTrack->mUndoHandler->oldLengthSpinValue = selectedFunc->maxArgument - selectedFunc->minArgument;
    mParent->selTrack->mUndoHandler->oldChangeSpinValue = selectedFunc->symArg;
    mParent->selTrack->mUndoHandler->oldArg1Value = selectedFunc->arg1;
    mParent->selTrack->mUndoHandler->oldCenterSpinValue = selectedFunc->centerArg;
    mParent->selTrack->mUndoHandler->oldTransitionBoxValue = selectedFunc->degree;

    gloParent->updateInfoPanel();
    phantomChanges = false;
}

void transitionWidget::on_lengthSpin_valueChanged(double arg1)
{
    if(phantomChanges) return;
    int index = selectedFunc->parent->getSubfuncNumber(selectedFunc);

    const bool need_length_factor = (selectedFunc->parent->secParent->bArgument == DISTANCE && (selectedFunc->parent->secParent->type == forced || selectedFunc->parent->secParent->type == geometric)) || selectedFunc->parent->secParent->type == straight;

    if(need_length_factor) {
        selectedFunc->parent->changeLength(arg1/gloParent->mOptions->getLengthFactor(), index);
    } else {
        selectedFunc->parent->changeLength(arg1, index);
    }

    float diff = selectedFunc->maxArgument - mParent->selTrack->trackData->activeSection->getMaxArgument();

    if(diff > 0) {
        if(need_length_factor) {
            diff*=gloParent->mOptions->getLengthFactor();
        }
        ui->errLabel->setText(QString("Warning: Transition is ").append(QString().number(diff+0.0001, 'f', 2)).append(" longer than Section!"));
        ui->errLabel->show();
    } else {
        ui->errLabel->hide();
    }

    trackHandler* inTrack = mParent->selTrack;

    inTrack->trackData->updateTrack(inTrack->trackData->activeSection, (int)(selectedFunc->minArgument*F_HZ-1.5f));
    mParent->redrawGraphs();

    if(inTrack->trackData->activeSection->type == straight) {
        inTrack->trackWidgetItem->setupStraightFrame();
    } else if(inTrack->trackData->activeSection->type == curved) {
        inTrack->trackWidgetItem->setupCurvedFrame();
    }

    gloParent->updateInfoPanel();

    if(!inTrack->mUndoHandler->busy) {
        undoAction* temp = new undoAction(inTrack, onLengthSpin);
        temp->toValue = QVariant(selectedFunc->maxArgument - selectedFunc->minArgument);
        inTrack->mUndoHandler->addAction(temp);
        gloParent->setUndoButtons();
    }
}

void transitionWidget::on_transitionBox_currentIndexChanged(int index)
{
    if(phantomChanges) return;
    trackHandler* inTrack = mParent->selTrack;
    undoAction* temp1;
    if(!inTrack->mUndoHandler->busy) {
        temp1 = new undoAction(inTrack, onArg1);
        temp1->fromValue = QVariant(selectedFunc->arg1);
    }

    selectedFunc->changeDegree((eDegree)index);
    selectedFunc->parent->translateValues(selectedFunc);



    inTrack->trackData->updateTrack(inTrack->trackData->activeSection, (int)(selectedFunc->minArgument*F_HZ-1.5f));
    mParent->redrawGraphs();
    gloParent->updateInfoPanel();

    if(!inTrack->mUndoHandler->busy) {
        undoAction* temp = new undoAction(inTrack, onTransitionBox);
        temp1->toValue = QVariant(selectedFunc->arg1);
        temp->toValue = QVariant(selectedFunc->degree);
        temp->nextAction = temp1;
        inTrack->mUndoHandler->addAction(temp);
        gloParent->setUndoButtons();
    }

    changeSubfunc(selectedFunc); // to make sure the frames are up to date (after undo settings because that would fuck them up)
}

void transitionWidget::on_changeSpin_valueChanged(double arg1)
{
    if(phantomChanges) return;
    if(selectedFunc->parent->secParent->bArgument == DISTANCE && selectedFunc->parent->type != funcLateral && selectedFunc->parent->type != funcNormal) {
        selectedFunc->symArg = arg1*gloParent->mOptions->getLengthFactor();
    } else {
        selectedFunc->symArg = arg1;
    }
    selectedFunc->parent->translateValues(selectedFunc);

    trackHandler* inTrack = mParent->selTrack;

    inTrack->trackData->updateTrack(inTrack->trackData->activeSection, (int)(selectedFunc->minArgument*F_HZ-1.5f));
    mParent->redrawGraphs();
    gloParent->updateInfoPanel();

    if(!inTrack->mUndoHandler->busy) {
        undoAction* temp = new undoAction(inTrack, onChangeSpin);
        temp->toValue = QVariant(selectedFunc->symArg);
        inTrack->mUndoHandler->addAction(temp);
        gloParent->setUndoButtons();
    }
}

void transitionWidget::on_quadraticBox_currentIndexChanged(int index)
{
    if(phantomChanges) return;
    switch(index) {
    case 0:
        selectedFunc->arg1 = 1.f;
        break;
    case 1:
        selectedFunc->arg1 = -1.f;
        break;
    case 2:
        selectedFunc->arg1 = 0.f;
        break;
    default:
        lenAssert(0 && "unhandled default case");
        break;
    }
    selectedFunc->parent->translateValues(selectedFunc);

    trackHandler* inTrack = mParent->selTrack;

    inTrack->trackData->updateTrack(inTrack->trackData->activeSection, (int)(selectedFunc->minArgument*F_HZ-1.5f));
    mParent->redrawGraphs();
    gloParent->updateInfoPanel();

    if(!inTrack->mUndoHandler->busy)
    {
        undoAction* temp = new undoAction(inTrack, onArg1);
        temp->toValue = QVariant(selectedFunc->arg1);
        inTrack->mUndoHandler->addAction(temp);
        gloParent->setUndoButtons();
    }
}

void transitionWidget::on_quarticBox_currentIndexChanged(int index)
{
    if(phantomChanges) return;
    if(index > 0)
    {
        ui->quarticSpin->setVisible(true);
        if(selectedFunc->arg1 < -1)
        {
            switch(index)
            {
            case 1:
                selectedFunc->arg1 = 0.5f - 0.5f/(1+5*ui->quarticSpin->value());
                break;
            case 2:
                selectedFunc->arg1 = 0.5f + 0.5f/(1+5*ui->quarticSpin->value());
                break;
            }
        }
        else
        {
            selectedFunc->arg1 = 1-selectedFunc->arg1;
        }
    }
    else
    {
        ui->quarticSpin->setVisible(false);
        selectedFunc->arg1 = -10;
    }
    selectedFunc->parent->translateValues(selectedFunc);

    trackHandler* inTrack = mParent->selTrack;

    inTrack->trackData->updateTrack(inTrack->trackData->activeSection, (int)(selectedFunc->minArgument*F_HZ-1.5f));
    mParent->redrawGraphs();
    gloParent->updateInfoPanel();

    if(!inTrack->mUndoHandler->busy)
    {
        undoAction* temp = new undoAction(inTrack, onArg1);
        temp->toValue = QVariant(selectedFunc->arg1);
        inTrack->mUndoHandler->addAction(temp);
        gloParent->setUndoButtons();
    }
}

void transitionWidget::on_quarticSpin_valueChanged(double arg1)
{
    if(phantomChanges) return;
    int key = ui->quarticBox->currentIndex();
    switch (key)
    {
    case 1:
        selectedFunc->arg1 = 0.5f - 0.5f/(1+5*arg1);
        break;
    case 2:
        selectedFunc->arg1 = 0.5f + 0.5f/(1+5*arg1);
        break;
    default:
        qWarning("Bad Quartic Spin Change");
        return;
    }
    selectedFunc->parent->translateValues(selectedFunc);

    trackHandler* inTrack = mParent->selTrack;

    inTrack->trackData->updateTrack(inTrack->trackData->activeSection, (int)(selectedFunc->minArgument*F_HZ-1.5f));
    mParent->redrawGraphs();
    gloParent->updateInfoPanel();

    if(!inTrack->mUndoHandler->busy)
    {
        undoAction* temp = new undoAction(inTrack, onArg1);
        temp->toValue = QVariant(selectedFunc->arg1);
        inTrack->mUndoHandler->addAction(temp);
        gloParent->setUndoButtons();
    }
}

void transitionWidget::on_quinticBox_currentIndexChanged(int index)
{
    if(phantomChanges) return;
    switch(index)
    {
    case 0:
        ui->quinticSpin->setVisible(false);
        selectedFunc->arg1 = 0.f;
        break;
    case 1:
        ui->quinticSpin->setVisible(true);
        selectedFunc->arg1 = fabs(selectedFunc->arg1) < 0.005 ? -ui->quinticSpin->value() : -fabs(selectedFunc->arg1);
        phantomChanges = true;
        ui->quinticSpin->setValue(fabs(selectedFunc->arg1));
        phantomChanges = false;
        break;
    case 2:
        ui->quinticSpin->setVisible(true);
        selectedFunc->arg1 = fabs(selectedFunc->arg1) < 0.005 ? ui->quinticSpin->value() : fabs(selectedFunc->arg1);
        phantomChanges = true;
        ui->quinticSpin->setValue(fabs(selectedFunc->arg1));
        phantomChanges = false;
        break;
    }
    selectedFunc->parent->translateValues(selectedFunc);

    trackHandler* inTrack = mParent->selTrack;

    inTrack->trackData->updateTrack(inTrack->trackData->activeSection, (int)(selectedFunc->minArgument*F_HZ-1.5f));
    mParent->redrawGraphs();
    gloParent->updateInfoPanel();

    if(!inTrack->mUndoHandler->busy)
    {
        undoAction* temp = new undoAction(inTrack, onArg1);
        temp->toValue = QVariant(selectedFunc->arg1);
        inTrack->mUndoHandler->addAction(temp);
        gloParent->setUndoButtons();
    }
}

void transitionWidget::on_quinticSpin_valueChanged(double arg1)
{
    if(phantomChanges) return;
    int key = ui->quinticBox->currentIndex();
    switch (key)
    {
    case 1:
        selectedFunc->arg1 = -arg1;
        break;
    case 2:
        selectedFunc->arg1 = arg1;
        break;
    default:
        qWarning("Bad Quintic Spin Call");
        return;
    }

    trackHandler* inTrack = mParent->selTrack;

    inTrack->trackData->updateTrack(inTrack->trackData->activeSection, (int)(selectedFunc->minArgument*F_HZ-1.5f));
    mParent->redrawGraphs();
    gloParent->updateInfoPanel();

    if(!inTrack->mUndoHandler->busy)
    {
        undoAction* temp = new undoAction(inTrack, onArg1);
        temp->toValue = QVariant(selectedFunc->arg1);
        inTrack->mUndoHandler->addAction(temp);
        gloParent->setUndoButtons();
    }
}

void transitionWidget::on_centerSpin_valueChanged(double arg1)
{
    if(phantomChanges) return;
    selectedFunc->centerArg = arg1;

    trackHandler* inTrack = mParent->selTrack;

    inTrack->trackData->updateTrack(inTrack->trackData->activeSection, (int)(selectedFunc->minArgument*F_HZ-1.5f));
    mParent->redrawGraphs();
    gloParent->updateInfoPanel();

    if(!inTrack->mUndoHandler->busy)
    {
        undoAction* temp = new undoAction(inTrack, onCenterSpin);
        temp->toValue = QVariant(selectedFunc->centerArg);
        inTrack->mUndoHandler->addAction(temp);
        gloParent->setUndoButtons();
    }
}

void transitionWidget::on_tensionSpin_valueChanged(double arg1)
{
    if(phantomChanges) return;
    selectedFunc->tensionArg = arg1;

    trackHandler* inTrack = mParent->selTrack;

    inTrack->trackData->updateTrack(inTrack->trackData->activeSection, (int)(selectedFunc->minArgument*F_HZ-1.5f));
    mParent->redrawGraphs();
    gloParent->updateInfoPanel();

    if(!inTrack->mUndoHandler->busy)
    {
        undoAction* temp = new undoAction(inTrack, onTensionSpin);
        temp->toValue = QVariant(selectedFunc->tensionArg);
        inTrack->mUndoHandler->addAction(temp);
        gloParent->setUndoButtons();
    }
}

subfunc* transitionWidget::getSelectedFunc()
{
    return selectedFunc;
}

void transitionWidget::on_appendButton_released()
{
    trackHandler* inTrack = mParent->selTrack;
    int atIndex = selectedFunc->parent->getSubfuncNumber(selectedFunc);

    selectedFunc->parent->appendSubFunction(1, atIndex);
    inTrack->trackData->updateTrack(inTrack->trackData->activeSection, (int)(selectedFunc->maxArgument*F_HZ-1.5f));
    ui->removeButton->setEnabled(true);
    mParent->changeSelection(selectedFunc->parent->funcList[atIndex+1]);
    mParent->redrawGraphs();


    if(inTrack->trackData->activeSection->type == straight)
    {
        inTrack->trackWidgetItem->setupStraightFrame();
    }
    else if(inTrack->trackData->activeSection->type == curved)
    {
        inTrack->trackWidgetItem->setupCurvedFrame();
    }

    gloParent->updateInfoPanel();

    if(!inTrack->mUndoHandler->busy)
    {
        undoAction* temp = new undoAction(inTrack, appendSubFunction);
        inTrack->mUndoHandler->addAction(temp);
        gloParent->setUndoButtons();
    }
}

void transitionWidget::on_prependButton_released()
{
    trackHandler* inTrack = mParent->selTrack;
    int atIndex = selectedFunc->parent->getSubfuncNumber(selectedFunc)-1;

    selectedFunc->parent->appendSubFunction(1, atIndex);
    inTrack->trackData->updateTrack(inTrack->trackData->activeSection, (int)(selectedFunc->parent->funcList[atIndex+1]->minArgument*F_HZ-1.5f));
    ui->removeButton->setEnabled(true);
    mParent->changeSelection(selectedFunc->parent->funcList[atIndex+1]);
    mParent->redrawGraphs();

    if(mParent->selTrack->trackData->activeSection->type == straight)
    {
        mParent->selTrack->trackWidgetItem->setupStraightFrame();
    }
    else if(mParent->selTrack->trackData->activeSection->type == curved)
    {
        mParent->selTrack->trackWidgetItem->setupCurvedFrame();
    }

    gloParent->updateInfoPanel();

    if(!inTrack->mUndoHandler->busy)
    {
        undoAction* temp = new undoAction(inTrack, appendSubFunction);
        inTrack->mUndoHandler->addAction(temp);
        gloParent->setUndoButtons();
    }
}

void transitionWidget::on_removeButton_released()
{
    trackHandler* inTrack = mParent->selTrack;
    func* parentFunc = selectedFunc->parent;
    int pos = parentFunc->getSubfuncNumber(selectedFunc);

    if(!inTrack->mUndoHandler->busy)
    {
        undoAction* temp = new undoAction(inTrack, removeSubFunction);
        inTrack->mUndoHandler->addAction(temp);
        gloParent->setUndoButtons();
    }

    parentFunc->removeSubFunction(pos);
    if(pos == selectedFunc->parent->funcList.size())
    {
        mParent->selFunc = parentFunc->funcList[pos-1];
        this->changeSubfunc(parentFunc->funcList[pos-1]);
    }
    else
    {
        mParent->selFunc = parentFunc->funcList[pos];
        this->changeSubfunc(parentFunc->funcList[pos]);
    }
    inTrack->trackData->updateTrack(inTrack->trackData->activeSection, (int)(selectedFunc->minArgument*F_HZ-1.5f));
    mParent->redrawGraphs();

    if(inTrack->trackData->activeSection->type == straight)
    {
        inTrack->trackWidgetItem->setupStraightFrame();
    }
    else if(inTrack->trackData->activeSection->type == curved)
    {
        inTrack->trackWidgetItem->setupCurvedFrame();
    }

    gloParent->updateInfoPanel();
}

void transitionWidget::adjustLengthSteps(secType _type, bool _argument)
{
    switch(_type)
    {
    case straight:
        ui->lengthSpin->setSingleStep(1.);
        break;
    case curved:
        ui->lengthSpin->setSingleStep(10.);
        break;
    case forced:
    case geometric:
        if(_argument == TIME)
        {
            ui->lengthSpin->setSingleStep(0.1);
        }
        else
        {
            ui->lengthSpin->setSingleStep(1.);
        }
        break;
    default:
        qWarning("Bad Call to adjust Length Steps");
        return;
    }
}

void transitionWidget::on_lockCheck_stateChanged(int arg1)
{
    if(phantomChanges) return;
    undoAction* temp1 = NULL;
    trackHandler* inTrack = mParent->selTrack;

    int id = selectedFunc->parent->getSubfuncNumber(selectedFunc);
    if(arg1)
    {
        if(!inTrack->mUndoHandler->busy)
        {
            temp1 = new undoAction(inTrack, onLengthSpin);
            temp1->fromValue = selectedFunc->maxArgument - selectedFunc->minArgument;
        }
        selectedFunc->parent->lock(id);
    }
    else
    {
        selectedFunc->parent->unlock(id);
    }

    bool oldP = phantomChanges;
    phantomChanges = true;
    if(ui->lockCheck->isChecked())
    {
        ui->lengthSpin->setEnabled(false);
        selectedFunc->getValue(-1.f);
        ui->lengthSpin->setValue(selectedFunc->maxArgument-selectedFunc->minArgument);
    }
    else
    {
        ui->lengthSpin->setEnabled(true);
        selectedFunc->getValue(-1.f);
        ui->lengthSpin->setValue(selectedFunc->maxArgument-selectedFunc->minArgument);
    }
    phantomChanges = oldP;


    inTrack->trackData->updateTrack(inTrack->trackData->activeSection, (int)(selectedFunc->minArgument*F_HZ-1.5f));
    mParent->redrawGraphs();
    gloParent->updateInfoPanel();

    if(!inTrack->mUndoHandler->busy)
    {
        undoAction* temp = new undoAction(inTrack, changeFunctionStatus);
        if(temp1)
        {
            temp1->toValue = selectedFunc->maxArgument - selectedFunc->minArgument;
            temp->nextAction = temp1;
        }
        temp->toValue = arg1;
        temp->fromValue = !arg1;
        inTrack->mUndoHandler->addAction(temp);
        gloParent->setUndoButtons();
    }
}
