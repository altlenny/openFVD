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

#include "exportui.h"
#include "ui_exportui.h"
#include "mainwindow.h"
#include "exportfuncs.h"
#include "lenassert.h"
#include "trackwidget.h"
#include <QFileDialog>
#include <QPushButton>
#include <fstream>

using namespace std;

extern MainWindow* gloParent;

exportUi::exportUi(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Exportui)
{
    ui->setupUi(this);
}

exportUi::~exportUi()
{
    delete ui;
}

exportUi::exportUi(QWidget *parent, projectWidget* _project): QDialog(parent), ui(new Ui::Exportui)
{
    phantomChanges = true;
    ui->setupUi(this);

#ifdef Q_OS_MAC
    this->setFixedSize(435, 300);
#endif

    this->cFile = NULL;
    this->project = _project;
    this->fPerNode = 2.0f;

    for(int i = 0; i < project->trackList.size(); ++i) {
        ui->exportTrackBox->addItem(project->trackList[i]->trackData->name + QString(" (" + QString().number(i+1)) + QString(")"), QVariant().fromValue(i));
        if(project->trackList[i] == project->selTrack) {
            curTrackIndex = i;
        } else {
            curTrackIndex = 0;
        }
    }

    if(project->trackList.size() == 0) {
        curTrackIndex = -1;
        ui->exportFromBox->setDisabled(true);
        ui->exportToBox->setDisabled(true);
    }

    curFromIndex = -1;
    curToIndex = -1;

    ui->exportTrackBox->setCurrentIndex(curTrackIndex);

    if(project->selTrack) {
        track* tTrack = project->selTrack->trackData;

        for(int i = 0; i < tTrack->lSections.size(); ++i) {
            ui->exportFromBox->addItem(tTrack->lSections[i]->sName + QString(" (" + QString().number(i+1)) + QString(")"), QVariant().fromValue(i));
            ui->exportToBox->addItem(tTrack->lSections[i]->sName + QString(" (" + QString().number(i+1)) + QString(")"), QVariant().fromValue(i));
        }

        if(tTrack->lSections.size()) {
            ui->exportFromBox->setCurrentIndex(0);
            ui->exportToBox->setCurrentIndex(tTrack->lSections.size()-1);
            curFromIndex = 0;
            curToIndex = tTrack->lSections.size()-1;
        }
    }
    phantomChanges = false;
}

void exportUi::doExport()
{
    this->fPerNode = ui->segmentLengthBox->value();
    float fRollThresh = sin(ui->relThresBox->value()*F_PI/180.f);

    track* tTrack = project->trackList[curTrackIndex]->trackData;

    if (!fileName.isEmpty()) {
        this->cFile = fileName.toLocal8Bit().data();
        fstream* fout = new fstream(this->cFile, ios::out | ios::binary);

        if(fout == NULL) {
            lenAssert(0 && "File Stream is NULL");
            return;
        }

        writeBytes(fout, (const char*)"MELE", 4);
        writeNulls(fout, 4); // will be replaced with length of data
        writeNulls(fout, 64);
        writeNulls(fout, 4); // will be replaced with No of NL beziers

        float oldHeartLine = tTrack->fHeart;
        if(ui->noHeartLineBox->isChecked())
        {
            tTrack->fHeart = 0.f;
        }
        int iNodes = tTrack->exportTrack4(fout, this->fPerNode, curFromIndex, curToIndex+curFromIndex, fRollThresh);

        tTrack->fHeart = oldHeartLine;

        int iDataLength = iNodes*50+132;

        writeNulls(fout, 69);

        fout->seekp(4);
        writeBytes(fout, (const char*)&iDataLength, 4); // replaced with length of data
        fout->seekp(72);
        writeBytes(fout, (const char*)&iNodes, 4); // replaced with no of NL beziers

        fout->close();
        delete fout;
        gloParent->backupSave();
        gloParent->displayStatusMessage(QString("Export to ").append(fileName).append(" successful!"));
    } else {
        lenAssert(0 && "no FileName");
        return;
    }
}

void exportUi::doExport2()
{
    this->fPerNode = ui->segmentLengthBox->value();
    float fRollThresh = sin(ui->relThresBox->value()*F_PI/180.f);

    track* tTrack = project->trackList[curTrackIndex]->trackData;

    if (!fileName.isEmpty()) {
        this->cFile = fileName.toLocal8Bit().data();
        fstream* fout = new fstream(this->cFile, ios::out | ios::binary);

        if(fout == NULL) {
            lenAssert(0 && "File Stream is NULL");
            return;
        }

        writeBytes(fout, (const char*)"MELE", 4);
        writeNulls(fout, 4); // will be replaced with length of data
        writeNulls(fout, 64);
        writeNulls(fout, 4); // will be replaced with No of NL beziers

        float oldHeartLine = tTrack->fHeart;
        if(ui->noHeartLineBox->isChecked()) {
            tTrack->fHeart = 0.f;
        }
        int iNodes = tTrack->exportTrack3(fout, this->fPerNode, curFromIndex, curToIndex+curFromIndex, fRollThresh);

        tTrack->fHeart = oldHeartLine;

        int iDataLength = iNodes*50+132;

        writeNulls(fout, 69);

        fout->seekp(4);
        writeBytes(fout, (const char*)&iDataLength, 4); // replaced with length of data
        fout->seekp(72);
        writeBytes(fout, (const char*)&iNodes, 4); // replaced with no of NL beziers

        fout->close();
        delete fout;
        gloParent->backupSave();
        gloParent->displayStatusMessage(QString("Export to ").append(fileName).append(" successful!"));
    } else {
        lenAssert(0 && "no FileName");
        return;
    }
}

void exportUi::doNL2Export()
{
    fPerNode = ui->segmentLengthBox->value();

    track* tTrack = project->trackList[curTrackIndex]->trackData;

    if (!fileName.isEmpty()) {
        this->cFile = fileName.toLocal8Bit().data();
        FILE* fout = fopen(cFile, "w");

        fprintf(fout, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        fprintf(fout, "<root>\n");
        fprintf(fout, "\t<element>\n");
        fprintf(fout, "\t\t<description>FVD++ Export Data</description>\n");

        tTrack->exportNL2Track(fout, fPerNode, curFromIndex, curFromIndex+curToIndex);

        fprintf(fout, "\t</element>\n");
        fprintf(fout, "</root>\n");
        gloParent->backupSave();
        gloParent->displayStatusMessage(QString("Export to ").append(fileName).append(" successful!"));
        fclose(fout);
    } else {
        return;
    }
}

void exportUi::doFastExport()
{
    switch(ui->exportTypeBox->currentIndex()) {
    case 0:
        doNL2Export();
        break;
    case 1:
        doExport();
        break;
    case 2:
        doExport2();
        break;
    }
}

void exportUi::on_buttonBox_accepted()
{
#ifdef Q_OS_MAC
    QFileDialog fd(gloParent);
    fd.setWindowTitle(tr("Save File"));
    if(ui->exportTypeBox->currentIndex() == 0) {
        fd.setNameFilter(tr("NL Element (*.nl2elem)"));
    } else {
        fd.setNameFilter(tr("NL Element (*.nlelem)"));
    }
    fd.setAcceptMode(QFileDialog::AcceptSave);
    fd.selectFile(gloParent->getCurrentFileName().length()?gloParent->getCurrentFileName().replace(".fvd", ""):"Untitled");
    fd.setDirectory("");
    fd.setWindowModality(Qt::WindowModal);
    fd.setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    if(!fd.exec()) return;

    fileName = fd.selectedFiles().at(0);
#else
    QString filter;
    if(ui->exportTypeBox->currentIndex() == 0) {
        filter = QString("NL Element (*.nl2elem)");
    } else {
        filter = QString("NL Element (*.nlelem)");
    }
    fileName = QFileDialog::getSaveFileName(gloParent, "Save File", ".", filter, 0, 0);
#endif


    switch(ui->exportTypeBox->currentIndex()) {
    case 0:
        doNL2Export();
        break;
    case 1:
        doExport();
        break;
    case 2:
        doExport2();
        break;
    default:
        lenAssert(0 && "unknown exporter type");
        break;
    }
}

void exportUi::on_buttonBox_rejected()
{
    this->close();
}

void exportUi::on_exportTypeBox_currentIndexChanged(int index)
{
    lenAssert(ui->exportTypeBox->currentIndex() == index);
    switch(index) {
    case 0:
        ui->relThresBox->setDisabled(true);
        ui->noHeartLineBox->setDisabled(true);
        break;
    case 1:
    case 2:
        ui->relThresBox->setEnabled(true);
        ui->noHeartLineBox->setEnabled(true);
        break;
    default:
        lenAssert(0 && "unknown exporter type");
        break;
    }
}

void exportUi::on_exportFromBox_currentIndexChanged(int index)
{
    if(phantomChanges) return;

    track* tTrack = project->trackList[curTrackIndex]->trackData;

    phantomChanges = true;
    ui->exportToBox->clear();
    for(int i = index; i < tTrack->lSections.size(); ++i) {
        ui->exportToBox->addItem(tTrack->lSections[i]->sName + QString(" (" + QString().number(i+1)) + QString(")"), QVariant().fromValue(i));
    }

    lenAssert(ui->exportToBox->count() > curToIndex-index+curFromIndex);

    ui->exportToBox->setCurrentIndex(curToIndex-index+curFromIndex);
    curToIndex -= (index-curFromIndex);
    curFromIndex = index;
    phantomChanges = false;
}

void exportUi::on_exportToBox_currentIndexChanged(int index)
{
    if(phantomChanges) return;

    track* tTrack = project->trackList[curTrackIndex]->trackData;

    phantomChanges = true;
    ui->exportFromBox->clear();
    for(int i = 0; i <= index+curFromIndex; ++i) {
        ui->exportFromBox->addItem(tTrack->lSections[i]->sName + QString(" (" + QString().number(i+1)) + QString(")"), QVariant().fromValue(i));
    }

    lenAssert(ui->exportFromBox->count() > curFromIndex);

    ui->exportFromBox->setCurrentIndex(curFromIndex);
    curToIndex = index;
    phantomChanges = false;
}

void exportUi::on_exportTrackBox_currentIndexChanged(int index)
{
    if(phantomChanges) return;
    curTrackIndex = index;

    updateBoxes();
}

bool exportUi::updateBoxes()
{
    phantomChanges = true;

    on_exportTypeBox_currentIndexChanged(ui->exportTypeBox->currentIndex());

    ui->exportTrackBox->clear();
    if(curTrackIndex >= project->trackList.size()) {
        curTrackIndex = project->trackList.size() ? 0 : -1;
    }

    for(int i = 0; i < project->trackList.size(); ++i) {
        ui->exportTrackBox->addItem(project->trackList[i]->trackData->name + QString(" (" + QString().number(i+1)) + QString(")"), QVariant().fromValue(i));
    }
    if(project->trackList.size() == 0) {
        curTrackIndex = -1;
        ui->exportFromBox->setDisabled(true);
        ui->exportToBox->setDisabled(true);
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return false;
    } else {
        curTrackIndex = curTrackIndex < 0 ? 0 : curTrackIndex;
        ui->exportTrackBox->setCurrentIndex(curTrackIndex);
        ui->exportFromBox->setEnabled(true);
        ui->exportToBox->setEnabled(true);
    }

    project->trackList[curTrackIndex]->trackWidgetItem->writeNames();

    if(curTrackIndex > -1) {
        if(project->trackList[curTrackIndex]->trackData->lSections.size() == 0) {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        } else {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
    }

    track* tTrack = project->trackList[curTrackIndex]->trackData;

    ui->exportFromBox->clear();
    ui->exportToBox->clear();

    if(curFromIndex >= tTrack->lSections.size()) {
        curFromIndex = -1;
        curToIndex = -1;
    } else if(curFromIndex+curToIndex >= tTrack->lSections.size()) {
        curToIndex = tTrack->lSections.size()-curFromIndex-1;
    }

    if(curFromIndex == -1 && curToIndex == -1) {
        for(int i = 0; i < tTrack->lSections.size(); ++i) {
            ui->exportFromBox->addItem(tTrack->lSections[i]->sName + QString(" (" + QString().number(i+1)) + QString(")"), QVariant().fromValue(i));
            ui->exportToBox->addItem(tTrack->lSections[i]->sName + QString(" (" + QString().number(i+1)) + QString(")"), QVariant().fromValue(i));
        }

        if(tTrack->lSections.size() == 0) {
            curFromIndex = -1;
            curToIndex = -1;
        } else {
            ui->exportFromBox->setCurrentIndex(0);
            ui->exportToBox->setCurrentIndex(tTrack->lSections.size()-1);
            curFromIndex = 0;
            curToIndex = tTrack->lSections.size()-1;
        }
        phantomChanges = false;
        return false;
    } else {
        for(int i = 0; i <= curToIndex+curFromIndex; ++i) {
            ui->exportFromBox->addItem(tTrack->lSections[i]->sName + QString(" (" + QString().number(i+1)) + QString(")"), QVariant().fromValue(i));
        }

        for(int i = curFromIndex; i < tTrack->lSections.size(); ++i) {
            ui->exportToBox->addItem(tTrack->lSections[i]->sName + QString(" (" + QString().number(i+1)) + QString(")"), QVariant().fromValue(i));
        }

        if(curToIndex >= tTrack->lSections.size() || curFromIndex >= tTrack->lSections.size()) {
            ui->exportFromBox->setCurrentIndex(0);
            ui->exportToBox->setCurrentIndex(tTrack->lSections.size()-1);
            curFromIndex = 0;
            curToIndex = tTrack->lSections.size()-1;
            phantomChanges = false;
            return false;
        } else {
            ui->exportFromBox->setCurrentIndex(curFromIndex);
            ui->exportToBox->setCurrentIndex(curToIndex);
            phantomChanges = false;
            return !fileName.isEmpty();
        }
    }
    return true;
}
