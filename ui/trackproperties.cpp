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

#include "track.h"
#include "mainwindow.h"
#include "projectwidget.h"
#include "trackhandler.h"
#include "undoaction.h"
#include "undohandler.h"
#include "trackmesh.h"

#include "trackproperties.h"
#include "ui_trackproperties.h"
#include <QColorDialog>


extern MainWindow* gloParent;

TrackProperties::TrackProperties(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TrackProperties)
{
    ui->setupUi(this);
    curTrack = NULL;
    colorPicker = new QColorDialog(this);
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, false);
}

TrackProperties::~TrackProperties()
{
    delete colorPicker;
    delete ui;
}


void TrackProperties::on_frictionBox_valueChanged(double)
{
}

void TrackProperties::on_heartlineBox_valueChanged(double)
{
}

void TrackProperties::on_defaultColorButton_released()
{
    colorPicker->setCurrentColor(curTrack->trackColors[0]);
    colorPicker->open(this, SLOT(ondefaultColor_received()));
}

void TrackProperties::ondefaultColor_received()
{
    curTrack->trackColors[0] = colorPicker->selectedColor();
    QPalette palette = ui->defaultColorButton->palette();
    palette.setColor(QPalette::ButtonText, curTrack->trackColors[0]);
    ui->defaultColorButton->setPalette(palette);
    curTrack->trackData->hasChanged = true;
}

void TrackProperties::on_sectionColorButton_released()
{
    colorPicker->setCurrentColor(curTrack->trackColors[1]);
    colorPicker->open(this, SLOT(onsectionColor_received()));
}

void TrackProperties::onsectionColor_received()
{
    curTrack->trackColors[1] = colorPicker->selectedColor();
    QPalette palette = ui->sectionColorButton->palette();
    palette.setColor(QPalette::ButtonText, curTrack->trackColors[1]);
    ui->sectionColorButton->setPalette(palette);
    curTrack->trackData->hasChanged = true;
}

void TrackProperties::on_transitionColorButton_released()
{
    colorPicker->setCurrentColor(curTrack->trackColors[2]);
    colorPicker->open(this, SLOT(ontransitionColor_received()));
}

void TrackProperties::ontransitionColor_received()
{
    curTrack->trackColors[2] = colorPicker->selectedColor();
    QPalette palette = ui->transitionColorButton->palette();
    palette.setColor(QPalette::ButtonText, curTrack->trackColors[2]);
    ui->transitionColorButton->setPalette(palette);
    curTrack->trackData->hasChanged = true;
}

void TrackProperties::on_drawBox_currentIndexChanged(int index)
{
    curTrack->trackData->drawHeartline = index;
}

void TrackProperties::on_styleBox_currentIndexChanged(int)
{
}

void TrackProperties::on_wireframeBox_stateChanged(int)
{
}

void TrackProperties::on_buttonBox_accepted()
{
    if(curTrack->trackData->fFriction != ui->frictionBox->value()) {
        curTrack->trackData->fFriction = ui->frictionBox->value();
        curTrack->trackData->updateTrack(0, 0);

        if(!curTrack->mUndoHandler->busy) {
            undoAction* temp = new undoAction(curTrack, changeTrackFriction);
            temp->toValue = QVariant(ui->frictionBox->value());
            curTrack->mUndoHandler->addAction(temp);
            gloParent->setUndoButtons();
        }
    }

    if(curTrack->trackData->fResistance != ui->resistanceBox->value()*1e-5) {
        curTrack->trackData->fResistance = ui->resistanceBox->value()*1e-5;
        curTrack->trackData->updateTrack(0, 0);

        if(!curTrack->mUndoHandler->busy) {
            undoAction* temp = new undoAction(curTrack, changeTrackResistance);
            temp->toValue = QVariant(ui->resistanceBox->value()*1e-5);
            curTrack->mUndoHandler->addAction(temp);
            gloParent->setUndoButtons();
        }
    }


    if(curTrack->trackData->fHeart != ui->heartlineBox->value()) {
        mnode* anchor = curTrack->trackData->anchorNode;
        glm::vec3 temp = anchor->vPosHeart(curTrack->trackData->fHeart);
        float roll = anchor->fRoll*F_PI/180;
        float pitch = anchor->getPitch()*F_PI/180;
        float yaw = curTrack->trackData->startYaw*F_PI/180;
        float x = -cos(yaw)*temp.z+sin(yaw)*temp.x+curTrack->trackData->startPos.x;
        float y = temp.y+curTrack->trackData->startPos.y;
        float z = cos(yaw)*temp.x+sin(yaw)*temp.z+curTrack->trackData->startPos.z;

        undoAction *tempx, *tempy, *tempz;
        if(!curTrack->mUndoHandler->busy) {
            tempx = new undoAction(curTrack, changeAnchorPosX);
            tempx->fromValue = QVariant(curTrack->trackData->startPos.x);

            tempy = new undoAction(curTrack, changeAnchorPosY);
            tempy->fromValue = QVariant(curTrack->trackData->startPos.y);
            tempy->nextAction = tempx;

            tempz = new undoAction(curTrack, changeAnchorPosZ);
            tempz->fromValue = QVariant(curTrack->trackData->startPos.z);
            tempz->nextAction = tempy;
        }

        curTrack->trackData->fHeart = ui->heartlineBox->value();

        curTrack->trackData->startPos.x = -(cos(yaw)*sin(pitch)*cos(roll)+sin(yaw)*sin(roll))*curTrack->trackData->fHeart + x;
        curTrack->trackData->startPos.y = (cos(pitch)*cos(roll))*curTrack->trackData->fHeart + y;
        curTrack->trackData->startPos.z = (sin(yaw)*sin(pitch)*cos(roll)-cos(yaw)*sin(roll))*curTrack->trackData->fHeart + z;

        curTrack->trackData->updateTrack(0, 0);
        if(!curTrack->mUndoHandler->busy) {
            tempx->toValue = QVariant(curTrack->trackData->startPos.x);
            tempy->toValue = QVariant(curTrack->trackData->startPos.y);
            tempz->toValue = QVariant(curTrack->trackData->startPos.z);

            undoAction* temp = new undoAction(curTrack, changeTrackHeartline);
            temp->toValue = QVariant(ui->heartlineBox->value());
            temp->nextAction = tempz;
            curTrack->mUndoHandler->addAction(temp);
            gloParent->setUndoButtons();
        }
    }


    if(curTrack->trackData->style != (enum trackStyle)ui->styleBox->currentIndex()) {
        curTrack->trackData->style = (enum trackStyle)ui->styleBox->currentIndex();
        curTrack->mMesh->buildMeshes(0);
        curTrack->trackData->hasChanged = true;
    }

    if(curTrack->mMesh->isWireframe != ui->wireframeBox->isChecked()) {
        curTrack->mMesh->isWireframe = ui->wireframeBox->isChecked();
        curTrack->mMesh->buildMeshes(0);
        curTrack->trackData->hasChanged = true;
    }

}

void TrackProperties::on_buttonBox_rejected()
{
    return;
}

void TrackProperties::openForTrack(trackHandler* _curTrack)
{
    if(_curTrack != curTrack) {
        curTrack = _curTrack;

        ui->frictionBox->setValue(curTrack->trackData->fFriction);
        ui->heartlineBox->setValue(curTrack->trackData->fHeart);
        ui->resistanceBox->setValue(curTrack->trackData->fResistance*1e5);

        QPalette palette = ui->defaultColorButton->palette();
        palette.setColor(QPalette::ButtonText, curTrack->trackColors[0]);
        ui->defaultColorButton->setPalette(palette);
        palette.setColor(QPalette::ButtonText, curTrack->trackColors[1]);
        ui->sectionColorButton->setPalette(palette);
        palette.setColor(QPalette::ButtonText, curTrack->trackColors[2]);
        ui->transitionColorButton->setPalette(palette);

        ui->drawBox->setCurrentIndex(curTrack->trackData->drawHeartline);
        ui->styleBox->setCurrentIndex(curTrack->trackData->style);
        ui->wireframeBox->setChecked(curTrack->mMesh->isWireframe);

    }
    this->show();
}
