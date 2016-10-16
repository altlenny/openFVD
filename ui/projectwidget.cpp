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

#include "projectwidget.h"
#include "ui_projectwidget.h"
#include <QtCore>
#include "trackhandler.h"
#include "exportfuncs.h"
#include "importui.h"
#include "undohandler.h"
#include "nolimitsimporter.h"
#include "mainwindow.h"
#include "undoaction.h"
#include "trackmesh.h"
#include "trackproperties.h"
#include "lenassert.h"
#include "trackwidget.h"
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>

#ifdef Q_OS_MAC
    #include "osx/common.h"
#endif

extern MainWindow* gloParent;
extern glViewWidget* glView;

projectWidget::projectWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::projectWidget)
{
    ui->setupUi(this);
    texPath = QString(":/background.png");
    ui->texEdit->setText(texPath);


    ui->trackListWidget->setItemDelegateForColumn(0, new NoEditDelegate(this));
    ui->trackListWidget->setItemDelegateForColumn(2, new NoEditDelegate(this));
    //ui->trackListWidget->header()->setMovable(false);
    ui->trackListWidget->setColumnWidth(0, 30);
    ui->trackListWidget->setColumnWidth(1, 200);
    ui->trackListWidget->setColumnWidth(2, 30);
    on_trackListWidget_itemSelectionChanged();

    ui->texEdit->setEnabled(false); // change this
    phantomChanges = false;

    properties = new TrackProperties(gloParent);

#ifdef Q_OS_MAC
    properties->setWindowModality(Qt::WindowModal);
    properties->setWindowFlags((properties->windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    properties->setFixedSize(QSize(400, 380));
#endif

}

projectWidget::~projectWidget()
{
    delete ui;
}

int projectWidget::getTrack(QTreeWidgetItem *item)
{
    for(int i = 0; i < trackList.size(); ++i) {
        if(trackList[i]->listItem == item) return i;
    }
    lenAssert(0 && "found no corresponding Track");
    return -1;
}

void projectWidget::init()
{
    if(trackList.size() || ui->trackListWidget->topLevelItemCount() || selTrack) {
        cleanUp();
    }

    qDebug() << glView;

    ui->texEdit->setText(QString("./background.png"));
    glView->loadGroundTexture(":/background.png");

    newEmptyTrack();
}

void projectWidget::cleanUp()
{
    glView->paintMode = false;
    while(trackList.size()) {
        delete trackList[0];
        trackList.removeFirst();
    }
    glView->paintMode = true;
}

void projectWidget::on_editButton_released()
{
    if(!ui->trackListWidget->selectedItems().size()) {
        return;
    }
    QTreeWidgetItem* selected = ui->trackListWidget->selectedItems().at(0);
    int index = getTrack(selected);
    QSize winSize = gloParent->size();
    gloParent->openTab(trackList[index]);
    winSize = gloParent->size();
    gloParent->resize(winSize);
}

void projectWidget::on_addButton_released()
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    menu->addAction("Empty Track", this, SLOT(newEmptyTrack()));
    menu->addAction("From other FVD Project", this, SLOT(importFromProject()));
    menu->addAction("Import NoLimits 1 Track", this, SLOT(importNLTrack()));
    menu->addAction("Import NoLimits 2 CSV", this, SLOT(importNoLimitsCSV()));
    menu->addAction("Import from Pointlist", this, SLOT(importPointList()));

    menu->popup(this->cursor().pos());
}

void projectWidget::newEmptyTrack()
{
    trackHandler* _newTrack = new trackHandler(QString("unnamed Track"), trackList.size()+1);
    trackList.append(_newTrack);
    ui->trackListWidget->addTopLevelItem(_newTrack->listItem);
}

void projectWidget::importFromProject(QString fileName)
{
#ifdef Q_OS_MAC
    if(fileName.isEmpty()) {
        QFileDialog fd(gloParent);
        fd.setWindowTitle(tr("open FVD Data"));
        fd.setFileMode(QFileDialog::ExistingFile);
        fd.setNameFilter(tr("FVD Data(*.fvd);;Backed Up FVD Data(*.bak)"));
        fd.setDirectory(QDir::currentPath());
        fd.setWindowModality(Qt::WindowModal);
        fd.setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
        if(!fd.exec()) return;
        fileName = fd.selectedFiles().at(0);
    }
#else
    fileName = QFileDialog::getOpenFileName(gloParent, "open FVD Data", "", "FVD Data(*.fvd);;Backed Up FVD Data(*.bak)");
#endif

    if(!fileName.isEmpty()) {
        importUi* importer = new importUi(this, fileName);
        importer->show();
    }
}

void projectWidget::importNLTrack()
{
#ifdef Q_OS_MAC
    QFileDialog fd(gloParent);
    fd.setWindowTitle(tr("open NL1 Track"));
    fd.setNameFilter(tr("NL1 Track(*.nltrack)"));
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setDirectory(QDir::currentPath());
    fd.setWindowModality(Qt::WindowModal);
    fd.setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    if(!fd.exec()) return;
    QString fileName = fd.selectedFiles().at(0);
#else
    QString fileName = QFileDialog::getOpenFileName(gloParent, "open NL1 Track", "", "NL1 Track(*.nltrack)");
#endif

    if(!fileName.isEmpty()) {
        newEmptyTrack();
        trackHandler* newTrack = trackList.back();
        noLimitsImporter* importer = new noLimitsImporter(newTrack, fileName);
        importer->importAsNlTrack();
        delete importer;
    }
}

void projectWidget::importNoLimitsCSV()
{
#ifdef Q_OS_MAC
    QFileDialog fd(gloParent);
    fd.setWindowTitle(tr("open NL2 CSV"));
    fd.setNameFilter(tr("NL2 CSV(*.csv)"));
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setDirectory(QDir::currentPath());
    fd.setWindowModality(Qt::WindowModal);
    fd.setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    if(!fd.exec()) return;
    QString fileName = fd.selectedFiles().at(0);
#else
    QString fileName = QFileDialog::getOpenFileName(gloParent, "open NL2 CSV", "", "NL2 CSV(*.csv)");
#endif

    if(!fileName.isEmpty()) {
        newEmptyTrack();
        trackHandler* newTrack = trackList.back();

        newTrack->trackData->fHeart = 0.0f;
        newTrack->trackWidgetItem->addSection(nolimitscsv);

        dynamic_cast<secnlcsv *>(newTrack->trackData->lSections[0])->loadTrack(fileName);
    }
}

void projectWidget::importPointList()
{
#ifdef Q_OS_MAC
    QFileDialog fd(gloParent);
    fd.setWindowTitle(tr("open Text File"));
    fd.setNameFilter(tr("Text(*.txt)"));
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setDirectory(QDir::currentPath());
    fd.setWindowModality(Qt::WindowModal);
    fd.setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    if(!fd.exec()) return;
    QString fileName = fd.selectedFiles().at(0);
#else
    QString fileName = QFileDialog::getOpenFileName(gloParent, "open Text File", "", "Text(*.txt)");
#endif

    if(!fileName.isEmpty()) {
        newEmptyTrack();
        trackHandler* newTrack = trackList.back();
        noLimitsImporter* importer = new noLimitsImporter(newTrack, fileName);
        importer->importAsTxt();
        delete importer;
    }
}

void projectWidget::appendTracks(QList<trackHandler *> &_list)
{
    int i = trackList.size()-1;

    for(int j = 0; j < _list.size(); ++j) {
        ui->trackListWidget->addTopLevelItem(_list[j]->listItem);
    }

    trackList.append(_list);

    while(i < trackList.size()) {
        trackList[i]->changeID(i+1);
        ++i;
    }
}

void projectWidget::on_deleteButton_released()
{
    if(areYouSure()) {
        int i;
        for(i = 0; i < trackList.size(); ++i) {
            if(trackList[i] == selTrack) {
                trackList.removeAt(i);
                delete selTrack;
                break;
            }
        }
        while(i < trackList.size()) {
            trackList[i]->changeID(trackList[i]->getID()-1);
            ++i;
        }
    }
}

void projectWidget::on_trackListWidget_itemSelectionChanged()
{
    if(!ui->trackListWidget->selectedItems().size()) {
        selTrack = NULL;

        ui->editButton->setEnabled(false);
        ui->propertyButton->setEnabled(false);
        ui->deleteButton->setEnabled(false);
        this->adjustSize();
    } else {
        QTreeWidgetItem* selected = ui->trackListWidget->selectedItems().at(0);
        int index = getTrack(selected);

        selTrack = trackList[index];

        phantomChanges = true;

        selTrack->mUndoHandler->oldTrackFriction = selTrack->trackData->fFriction;
        selTrack->mUndoHandler->oldTrackResistance = selTrack->trackData->fResistance;
        selTrack->mUndoHandler->oldTrackHeartline = selTrack->trackData->fHeart;

        phantomChanges = false;

        ui->editButton->setEnabled(true);
        ui->propertyButton->setEnabled(true);
        ui->deleteButton->setEnabled(true);
        gloParent->setUndoButtons();
    }
}

void projectWidget::on_trackListWidget_itemChanged(QTreeWidgetItem *item, int column)
{
    int index = getTrack(item);
    switch(column) {
    case 0:
        return;
    case 1:
        trackList[index]->trackData->name = item->text(1);
        if(trackList[index]->trackWidgetItem) gloParent->renameTab(trackList[index]);
        return;
    case 2:
        if(item->checkState(2) == Qt::Checked) {
            trackList[index]->trackData->drawTrack = true;
        } else {
            trackList[index]->trackData->drawTrack = false;
        }
        return;
    default:
        lenAssert(0 && "unexpected default case");
        break;
    }
}

bool projectWidget::areYouSure()
{
    QTreeWidgetItem* selected = ui->trackListWidget->selectedItems().at(0);
    int index = getTrack(selected);

    QMessageBox mb(gloParent);
    mb.setIcon(QMessageBox::Warning);
#ifdef Q_OS_MAC
    mb.setWindowModality(Qt::WindowModal);
    mb.setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#endif
    mb.setText(QString("Are you sure you want to delete this track (").append(trackList[index]->trackData->name).append(")?"));
    mb.setWindowTitle(tr("Application"));
    mb.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    int ret = mb.exec();

    if (ret == QMessageBox::Yes) {
        return true;
    } else {
        return false;
    }
}

QString projectWidget::saveProject(std::fstream& file)
{
    file << "FVD";
    file << "v0.77";

    int namelength = texPath.length();
    std::string stdName = texPath.toStdString();

    writeBytes(&file, (const char*)&namelength, sizeof(int));
    file << stdName;


    for(int i = 0; i < this->trackList.size(); ++i) {
        trackList[i]->trackWidgetItem->writeNames();
        trackList[i]->trackData->saveTrack(file, trackList[i]->trackWidgetItem);
    }

    file << "EOP";
    return QString("Project Saved!");
}

QString projectWidget::loadProject(std::fstream& file)
{
    int errType = -1;
    std::string temp = readString(&file, 3);
    if(temp != "FVD") return QString("Error while Loading: No FVD File!");
    temp = readString(&file, 5);
    int legacy;
    if(temp == "v0.30") {
        legacy = 1;
    } else if(temp == "v0.77") {
        legacy = 0;
    } else {
        legacy = -1;
    }

    this->cleanUp();
    if(legacy > -1) { // supported versions
        int namelength = readInt(&file);
        texPath = QString(readString(&file, namelength).c_str());

        if(!glView->loadGroundTexture(texPath)) { // error while Loading
            texPath = QString(":/background.png");
            glView->loadGroundTexture(texPath);
            errType = 1;
        }
        ui->texEdit->setText(texPath);

        int i = 0;
        while(1) {
            temp = readString(&file, 3);
            if(temp == "TRC") {
                newEmptyTrack();
                if(legacy == 1) {
                    trackList[i]->trackData->legacyLoadTrack(file, trackList[i]->trackWidgetItem);
                    errType = 0;
                } else {
                    trackList[i]->trackData->loadTrack(file, trackList[i]->trackWidgetItem);

                    trackWidget* _widget = trackList[i]->trackWidgetItem;
                    if(!_widget->smoothScreen) {
                        _widget->smoothScreen = new smoothUi(trackList[i], gloParent);
                        trackList[i]->trackData->smoother = _widget->smoothScreen;
                    }

                    _widget->smoothScreen->updateUi();
                    _widget->smoothScreen->applyRollSmooth();
                }
                trackList[i]->listItem->setText(1, trackList[i]->trackData->name);
                trackList[i]->mUndoHandler->clearActions();
            }
            else if(temp == "EOP") {
                break;
            } else {
                errType = 11;
                break;
            }
            ++i;
        }
    } else {
        errType = 10;
    }

    for(int i = 0; i < trackList.size(); ++i) {
        if(trackList[i]->trackData->drawTrack == false) {
            trackList[i]->listItem->setCheckState(2, Qt::Unchecked);
        }
    }

    switch(errType) {
    case -1:
        return QString("Load Successfull.");
    case 0:
        return QString("Warning: Loaded old File Version. Please save to convert to new File version!");
    case 1:
        return QString("Warning: Could Not find corresponding texture. Loaded default texture instead.");
    case 10:
        this->cleanUp();
        return QString("Error: Unsupported File Version!");
    case 11:
        this->cleanUp();
        return QString("Error: File Corrupted!");
    }
    return QString();
}

void projectWidget::on_texChooser_released()
{
#ifdef Q_OS_MAC
    QFileDialog fd(gloParent);
    fd.setWindowTitle(tr("Load Texture"));
    fd.setNameFilter(tr("PNG Image (*.png)"));
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setDirectory("");
    fd.setWindowModality(Qt::WindowModal);
    fd.setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    if(!fd.exec()) return;
    QString fileName = fd.selectedFiles().at(0);
#else
    QString fileName = QFileDialog::getOpenFileName(gloParent, "open Texture", "", "PNG Image (*.png)");
#endif

    QString relPath = QDir::current().relativeFilePath(fileName);

    if(fileName.isEmpty()) {
        return;
    } else {
        if(fileName.endsWith(".png")) {
            texPath = relPath;
            ui->texEdit->setText(texPath);
            glView->loadGroundTexture(texPath);
        } else {
            QMessageBox::warning(this, tr("Application"),
                                       QString("Texture has to a *.png file!"),
                                       QMessageBox::Ok);
        }
    }
}

void projectWidget::on_trackListWidget_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if(ui->trackListWidget->itemAt(pos) == NULL) {
        menu->addAction("New Empty Track", this, SLOT(newEmptyTrack()));
        menu->addAction("Import Track from Project", this, SLOT(importFromProject()));
        menu->addAction("Import NoLimits 1 Track", this, SLOT(importNLTrack()));
        menu->addAction("Import NoLimits 2 CSV", this, SLOT(importNoLimitsCSV()));
        menu->addAction("Import from Pointlist", this, SLOT(importPointList()));
    } else {
        menu->addAction("New Empty Track", this, SLOT(newEmptyTrack()));
        menu->addAction("Import Track from Project", this, SLOT(importFromProject()));
        menu->addAction("Import NoLimits 1 Track", this, SLOT(importNLTrack()));
        menu->addAction("Import NoLimits 2 CSV", this, SLOT(importNoLimitsCSV()));
        menu->addAction("Import from Pointlist", this, SLOT(importPointList()));
        if(selTrack != NULL) {
            menu->addAction("Edit Track", this, SLOT(on_editButton_released()));
            menu->addAction("Remove Track", this, SLOT(on_deleteButton_released()));
        }
    }

    menu->popup(ui->trackListWidget->mapToGlobal(pos));
}

void projectWidget::on_trackListWidget_itemDoubleClicked(QTreeWidgetItem *, int column)
{
    if(column != 1) {
        on_editButton_released();
    }
}

void projectWidget::keyPressEvent(QKeyEvent* event) {
    switch(event->key()) {
    case Qt::Key_Delete:
        on_deleteButton_released();
        event->accept();
        break;
    default:
        event->ignore();
        break;
    }
}

void projectWidget::on_propertyButton_released()
{
    properties->openForTrack(selTrack);
}
