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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVector>

#include "mnode.h"
#include "exportui.h"
#include "optionsmenu.h"
#include "conversionpanel.h"
#include "graphwidget.h"
#include <sstream>
#include "trackwidget.h"
#include <QTimer>
#include "undohandler.h"
#include "undoaction.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include "objectexporter.h"

MainWindow* gloParent;
glViewWidget* glView;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    gloParent = this;
    project = NULL;

    // load options
    mOptions = new optionsMenu(this);
    mOptions->setWindowFlags(mOptions->windowFlags() | Qt::CustomizeWindowHint);
    mOptions->setWindowFlags(mOptions->windowFlags() & (~Qt::WindowMinimizeButtonHint));
    mOptions->setWindowFlags(mOptions->windowFlags() & (~Qt::WindowContextHelpButtonHint));

#ifdef Q_OS_MAC
    mOptions->setWindowModality(Qt::WindowModal);
    mOptions->setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#endif

    ui->setupUi(this);

    glView = new glViewWidget(ui->splitter);
    glView->setObjectName(QStringLiteral("GraphicsView"));
    glView->setMinimumSize(QSize(0, 0));
    glView->setMouseTracking(true);
    glView->setFocusPolicy(Qt::StrongFocus);
    ui->splitter->addWidget(glView);

    // set up all sub widgets etc
    project = ui->projectTab;
    currentFileName.clear();
    this->setWindowTitle(QString("FVD++ - unsaved Work"));


    // set up GL frame
#ifndef Q_OS_MAC
    if(mOptions->glPolicy == 1) {
        glView->legacyMode = true;
    } else
#endif
    if(mOptions->glPolicy == 2) {
        glView->legacyMode = false;
    }
    else if(glView->format().majorVersion() < 3 || (glView->format().majorVersion() == 3 && glView->format().minorVersion() < 1)) {
        glView->legacyMode = true;
    } else {
        glView->legacyMode = false;
    }
    if(glView->legacyMode) {
        delete ui->menuView;
    }

    phantomChanges = false;

    // init conversion panel
    mConversion = new conversionPanel(this);
    mConversion->setWindowFlags(mConversion->windowFlags() | Qt::CustomizeWindowHint);
    mConversion->setWindowFlags(mConversion->windowFlags() & (~Qt::WindowMinimizeButtonHint));
    mConversion->setWindowFlags(mConversion->windowFlags() & (~Qt::WindowContextHelpButtonHint));

    //ui->customPlot->hide();
    mGraphWidget = NULL;

    delete ui->customPlot;
    ui->customPlot = NULL;


    exportScreen = new exportUi(this, ui->projectTab);
    exportScreen->setWindowFlags(exportScreen->windowFlags() | Qt::CustomizeWindowHint);
    exportScreen->setWindowFlags(exportScreen->windowFlags() & (~Qt::WindowMinimizeButtonHint));
    exportScreen->setWindowFlags(exportScreen->windowFlags() & (~Qt::WindowContextHelpButtonHint));

    mObjectExporter = new objectExporter(this);
    mObjectExporter->setWindowFlags(exportScreen->windowFlags());

#ifdef Q_OS_MAC
    exportScreen->setWindowModality(Qt::WindowModal);
    exportScreen->setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);

    mObjectExporter->setWindowModality(Qt::WindowModal);
    mObjectExporter->setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#endif

    setUndoButtons();
    undoChanges = false;

    QTabBar *tabBar = ui->tabChooser->findChild<QTabBar*>();
    #ifndef Q_OS_MAC // on Win / Unix
        tabBar->tabButton(0, QTabBar::RightSide)->resize(0, 0); // hide close button on project tab
    #endif
    #ifdef Q_OS_MAC
        tabBar->tabButton(0, QTabBar::LeftSide)->resize(0, 0); // hide close button on project tab
    #endif

    selectedFunc = NULL;


    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), glView, SLOT(updateGL()));
    connect(timer, SIGNAL(timeout()), this, SLOT(showCurInfoPanel()));
    timer->start(16);

    QTimer *autosave = new QTimer(this);
    connect(autosave, SIGNAL(timeout()), this, SLOT(doAutoSave()));
    autosave->start(1000*60);

    connect(this, SIGNAL(emitMessage(QString,int)), ui->statusBar, SLOT(showMessage(QString,int)));
}

MainWindow::~MainWindow()
{
    delete ui;
    exit(0);
}

void MainWindow::initProject()
{
    ui->projectTab->init();
}


track* MainWindow::curTrack()
{
    if(ui->tabChooser->currentIndex() == 0) {
        return NULL;
    } else {
        trackWidget* curWidget = (trackWidget*)ui->tabChooser->currentWidget();
        return curWidget->inTrack->trackData;
    }
}

QList<trackHandler*> MainWindow::getTrackList()
{
    return ui->projectTab->trackList;
}

void MainWindow::on_actionExportAs_triggered()
{
    exportScreen->updateBoxes();
    exportScreen->show();
}

void MainWindow::on_actionNew_triggered()
{
    currentFileName.clear();
    this->setWindowTitle(QString("FVD++ - unsaved Work"));
    ui->projectTab->init();
}

void MainWindow::addProject(QString fileName) {
    ui->projectTab->importFromProject(fileName);
}

void MainWindow::loadProject(QString fileName)
{
    currentFileName.clear();

    saver* gott = new saver(fileName, ui->projectTab, this);
    QString output = gott->doLoad();

    if(output.contains("Warning:") || output.contains("Error:")) {
        QMessageBox mb(this);
        mb.setWindowTitle(tr("Application"));
        mb.setText(output);
        mb.setIcon(QMessageBox::Warning);
        mb.setDefaultButton(QMessageBox::Ok);
#ifdef Q_OS_MAC
        mb.setWindowModality(Qt::WindowModal);
        mb.setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#endif
        mb.exec();

        if(output.contains("Warning:")) currentFileName = fileName;
        else if(output.contains("Error:")) on_actionNew_triggered();
    } else {
        ui->statusBar->showMessage(output, 5000);
        currentFileName = fileName;
    }

    glView->paintMode = true;

    delete gott;
    this->setWindowTitle(QString("FVD++ - " + currentFileName));
}

#ifdef Q_OS_MAC
void MainWindow::on_actionLoad_triggered(QString fileName)
{
#endif
#ifndef Q_OS_MAC
void MainWindow::on_actionLoad_triggered()
{
    QString fileName;
#endif
    glView->paintMode = false;
#ifdef Q_OS_MAC
    if(fileName.isEmpty()) {
        QFileDialog fd((QWidget*)this);
        fd.setWindowTitle("open FVD Data");
        fd.setFileMode(QFileDialog::ExistingFile);
        fd.setNameFilter(tr("FVD Data(*.fvd);;Backed Up FVD Data(*.bak)"));
        fd.setDirectory(QDir::currentPath());
        fd.setWindowModality(Qt::WindowModal);
        fd.setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
        if(!fd.exec()) {
            glView->paintMode = true;
            return;
        }
        fileName = fd.selectedFiles().at(0);
    }
#else
    fileName = QFileDialog::getOpenFileName(this, "open FVD Data", "", "FVD Data(*.fvd);;Backed Up FVD Data(*.bak)", 0, 0);
#endif

    if(fileName.isEmpty()) {
        glView->paintMode = true;
        return;
    }

    currentFileName.clear();

    saver* gott = new saver(fileName, ui->projectTab, this);
    QString output = gott->doLoad();

    if(output.contains("Warning:") || output.contains("Error:")) {
        QMessageBox mb(this);
        mb.setWindowTitle(tr("Application"));
        mb.setText(output);
        mb.setIcon(QMessageBox::Warning);
        mb.setDefaultButton(QMessageBox::Ok);
#ifdef Q_OS_MAC
        mb.setWindowModality(Qt::WindowModal);
        mb.setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#endif
        mb.exec();

        if(output.contains("Warning:")) currentFileName = fileName;
        else if(output.contains("Error:")) on_actionNew_triggered();
    } else {
        ui->statusBar->showMessage(output, 5000);
        currentFileName = fileName;
    }

    glView->paintMode = true;

    delete gott;
    this->setWindowTitle(QString("FVD++ - " + currentFileName));
}

void MainWindow::on_actionSave_triggered()
{
    if(glView->moveMode) {
        glView->cameraMov.z = 1.f;
        return;
    }

    if(currentFileName.isEmpty()) {
        on_actionSave_As_triggered();
    }
    saver* gott = new saver(currentFileName, ui->projectTab, this);
    QString output = gott->doSave();

    ui->statusBar->showMessage(output, 5000);

    delete gott;
    this->setWindowTitle(QString("FVD++ - " + currentFileName));
}

void MainWindow::backupSave()
{
    if(currentFileName.isEmpty()) {
        return;
    }
    saver* gott = new saver(QString().append(currentFileName).append(".bak"), ui->projectTab, this);
    gott->doSave();
    delete gott;
    this->setWindowTitle(QString("FVD++ - " + currentFileName));
    showMessage(QString("Executed autosave to ").append(currentFileName).append(".bak"));
}

void MainWindow::on_actionSave_As_triggered()
{
    if(glView->moveMode) {
        return;
    }

#ifdef Q_OS_MAC
    QFileDialog fd(this);
    fd.setWindowTitle(tr("Save File"));
    fd.setNameFilter(tr("FVD Data(*.fvd)"));
    fd.setAcceptMode(QFileDialog::AcceptSave);
    fd.selectFile(currentFileName.length()?currentFileName:"Untitled");
    fd.setDirectory("");
    fd.setWindowModality(Qt::WindowModal);
    fd.setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    if(!fd.exec()) return;
    QString fileName = fd.selectedFiles().at(0);
#else
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("FVD Data (*.fvd)"));
#endif

    if(!fileName.endsWith(".fvd") && !fileName.isEmpty()) {
        fileName.append(".fvd");
    }
    if(fileName.isEmpty()) {
        return;
    }
    currentFileName = fileName;
    saver* gott = new saver(currentFileName, ui->projectTab, this);
    QString output = gott->doSave();

    showMessage(output);

    delete gott;
    this->setWindowTitle(QString("FVD++ - " + currentFileName));
}

void MainWindow::on_actionQuit_triggered()
{
    this->close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (areYouSure()) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::areYouSure()
{
    QMessageBox mb(this);
    mb.setIcon(QMessageBox::Warning);
#ifdef Q_OS_MAC
    mb.setWindowModality(Qt::WindowModal);
    mb.setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#endif
    mb.setText(tr("Are you sure you want to quit?"));
    mb.setWindowTitle(tr("Application"));
    mb.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    int ret = mb.exec();

    if (ret == QMessageBox::Yes) {
        return true;
    } else {
        return false;
    }
}

void MainWindow::on_actionUndo_triggered()
{
    trackWidget* mWidget;
    if(ui->tabChooser->currentIndex() == 0) {
        if(ui->projectTab->selTrack) {
            mWidget = ui->projectTab->selTrack->trackWidgetItem;
        } else {
            ui->actionRedo->setEnabled(false);
            ui->actionUndo->setEnabled(false);
            return;
        }
    } else {
        mWidget = (trackWidget*)ui->tabChooser->currentWidget();
    }
    mWidget->inTrack->mUndoHandler->doUndo();
    setUndoButtons();
}

void MainWindow::on_actionRedo_triggered()
{
    trackWidget* mWidget;
    if(ui->tabChooser->currentIndex() == 0) {
        if(ui->projectTab->selTrack) {
            mWidget = ui->projectTab->selTrack->trackWidgetItem;
        } else {
            ui->actionRedo->setEnabled(false);
            ui->actionUndo->setEnabled(false);
            return;
        }
    } else {
        mWidget = (trackWidget*)ui->tabChooser->currentWidget();
    }
    mWidget->inTrack->mUndoHandler->doRedo();
    setUndoButtons();
}

void MainWindow::on_actionUseShader0_triggered()
{
    useShader(0);
}

void MainWindow::on_actionUseShader1_triggered()
{
    useShader(1);
}

void MainWindow::on_actionUseShader2_triggered()
{
    useShader(2);
}

void MainWindow::on_actionUseShader3_triggered()
{
    useShader(3);
}

void MainWindow::on_actionUseShader4_triggered()
{
    useShader(4);
}

void MainWindow::on_actionUseShader5_triggered()
{
    useShader(5);
}

void MainWindow::useShader(int shader)
{
    glView->curTrackShader = shader;
    glView->hasChanged = true;
    switch(shader) {
    case 0:
        ui->actionUseShader0->setChecked(true);
        ui->actionUseShader1->setChecked(false);
        ui->actionUseShader2->setChecked(false);
        ui->actionUseShader3->setChecked(false);
        ui->actionUseShader4->setChecked(false);
        ui->actionUseShader5->setChecked(false);
        break;
    case 1:
        ui->actionUseShader0->setChecked(false);
        ui->actionUseShader1->setChecked(true);
        ui->actionUseShader2->setChecked(false);
        ui->actionUseShader3->setChecked(false);
        ui->actionUseShader4->setChecked(false);
        ui->actionUseShader5->setChecked(false);
        break;
    case 2:
        ui->actionUseShader0->setChecked(false);
        ui->actionUseShader1->setChecked(false);
        ui->actionUseShader2->setChecked(true);
        ui->actionUseShader3->setChecked(false);
        ui->actionUseShader4->setChecked(false);
        ui->actionUseShader5->setChecked(false);
        break;
    case 3:
        ui->actionUseShader0->setChecked(false);
        ui->actionUseShader1->setChecked(false);
        ui->actionUseShader2->setChecked(false);
        ui->actionUseShader3->setChecked(true);
        ui->actionUseShader4->setChecked(false);
        ui->actionUseShader5->setChecked(false);
        break;
    case 4:
        ui->actionUseShader0->setChecked(false);
        ui->actionUseShader1->setChecked(false);
        ui->actionUseShader2->setChecked(false);
        ui->actionUseShader3->setChecked(false);
        ui->actionUseShader4->setChecked(true);
        ui->actionUseShader5->setChecked(false);
        break;
    case 5:
        ui->actionUseShader0->setChecked(false);
        ui->actionUseShader1->setChecked(false);
        ui->actionUseShader2->setChecked(false);
        ui->actionUseShader3->setChecked(false);
        ui->actionUseShader4->setChecked(false);
        ui->actionUseShader5->setChecked(true);
        break;
    }
}

void MainWindow::showCurInfoPanel()
{
    if(!glView->povMode) {
        return;
    }
    updateInfoPanel(glView->povNode);
}

void MainWindow::updateInfoPanel()
{
    if(curTrack() == NULL) {
        return;
    }

    trackWidget* temp = (trackWidget*)ui->tabChooser->currentWidget();
    temp->updateSectionFrame();
    mnode* lastnode;
    if(curTrack()->lSections.size() != 0 && curTrack()->activeSection != NULL) {
        lastnode = curTrack()->activeSection->lNodes.at(curTrack()->activeSection->lNodes.size()-1);
    } else {
        lastnode = curTrack()->anchorNode;
    }
    updateInfoPanel(lastnode);
}

void MainWindow::updateInfoPanel(mnode* lastnode)
{
    if(curTrack() == NULL) {
        return;
    }

    float heart = curTrack()->fHeart;
    glm::mat4 anchorBase = glm::translate(curTrack()->startPos) * glm::rotate(TO_RAD(curTrack()->startYaw-90.f), glm::vec3(0.f, 1.f, 0.f));

    glm::vec3 worldPos = glm::vec3(anchorBase * glm::vec4(lastnode->vPosHeart(heart), 1.f));

    QString str1, str2, str3;

    str1 = QString::number(worldPos.x*mOptions->getLengthFactor(), 'f', 3);
    str2 = QString::number(worldPos.y*mOptions->getLengthFactor(), 'f', 3);
    str3 = QString::number(worldPos.z*mOptions->getLengthFactor(), 'f', 3);
    if(worldPos.x >= 0.f) str1.prepend('+');
    if(worldPos.y >= 0.f) str2.prepend('+');
    if(worldPos.z >= 0.f) str3.prepend('+');
    ui->infoPosLabel->setText(QString("X: ").append(str1).append(mOptions->getLengthString()).append(QString("  Y: ")).append(str2).append(mOptions->getLengthString()).append(QString("  Z: ")).append(str3).append(mOptions->getLengthString()));

    str1 = QString::number(lastnode->fVel*mOptions->getSpeedFactor(), 'f', 3);
    if(lastnode->fVel >= 0.f) str1.prepend('+');
    ui->infoSpeedLabel->setText(QString("Speed: ").append(str1).append(mOptions->getSpeedString()));

    str1 = QString::number(lastnode->getDirection()+curTrack()->startYaw, 'f', 3);
    str2 = QString::number(lastnode->getYawChange(), 'f', 3);
    if(!str1.startsWith('-')) str1.prepend('+');
    if(!str2.startsWith('-')) str2.prepend('+');
    ui->infoDirLabel->setText(QString("Yaw: ").append(str1).append(QString("%1 (").arg(QChar(0xb0))).append(str2).append(QString("%1/s)").arg(QChar(0xb0))));

    str1 = QString::number(lastnode->getPitch(), 'f', 3);
    str2 = QString::number(lastnode->getPitchChange(), 'f', 3);
    if(!str1.startsWith('-')) str1.prepend('+');
    if(!str2.startsWith('-')) str2.prepend('+');
    ui->infoPitchLabel->setText(QString("Pitch: ").append(str1).append(QString("%1 (").arg(QChar(0xb0))).append(str2).append(QString("%1/s)").arg(QChar(0xb0))));

    float temp = (lastnode->fRollSpeed + lastnode->fSmoothSpeed);
    str1 = QString::number(lastnode->fRoll, 'f', 3);
    str2 = QString::number(temp, 'f', 3);
    if(!str1.startsWith('-')) str1.prepend('+');
    if(!str2.startsWith('-')) str2.prepend('+');
    ui->infoRollLabel->setText(QString("Roll: ").append(str1).append(QString("%1 (").arg(QChar(0xb0))).append(str2).append(QString("%1/s)").arg(QChar(0xb0))));


    str1 = QString::number(lastnode->forceNormal + lastnode->smoothNormal, 'f', 3);
    str2 = QString::number(lastnode->forceLateral + lastnode->smoothLateral, 'f', 3);
    if(!str1.startsWith('-')) str1.prepend('+');
    if(!str2.startsWith('-')) str2.prepend('+');
    ui->infoNormalLabel->setText(QString("y-Accel: ").append(str1).append(QString("g")));
    ui->infoLateralLabel->setText(QString("x-Accel: ").append(str2).append(QString("g")));
}

void MainWindow::setUndoButtons()
{
    trackWidget* mWidget;
    if(ui->tabChooser->currentIndex() == 0) {
        if(ui->projectTab->selTrack) {
            mWidget = ui->projectTab->selTrack->trackWidgetItem;
        } else {
            ui->actionRedo->setEnabled(false);
            ui->actionUndo->setEnabled(false);
            return;
        }
    } else {
        mWidget = (trackWidget*)ui->tabChooser->currentWidget();
    }

    if(mWidget == NULL) {
        ui->actionRedo->setEnabled(false);
        ui->actionUndo->setEnabled(false);
        return;
    }

    if(mWidget->inTrack->mUndoHandler->stackIndex > 0) {
        ui->actionRedo->setEnabled(true);
    } else {
        ui->actionRedo->setEnabled(false);
    }

    if(mWidget->inTrack->mUndoHandler->stackIndex != -1 && mWidget->inTrack->mUndoHandler->stackIndex != mWidget->inTrack->mUndoHandler->lActions.size()) {
        ui->actionUndo->setEnabled(true);
    } else {
        ui->actionUndo->setEnabled(false);
    }
}

void MainWindow::displayStatusMessage(QString message)
{
    ui->statusBar->showMessage(message, 5000);
}

void MainWindow::on_actionOptions_triggered()
{
    mOptions->setGLVersionString(glView->getGLVersionString());
    mOptions->show();
}

void MainWindow::on_actionConversion_Panel_triggered()
{
    mConversion->show();
}

int MainWindow::getPovPos()
{
    return glView->povPos;
}

QString MainWindow::getCurrentFileName() {
    return currentFileName;
}

void MainWindow::openTab(trackHandler* _track)
{
    this->setUpdatesEnabled(false);
    _track->trackWidgetItem->on_sectionListWidget_itemSelectionChanged();
    if(_track->tabId == -1) {
        _track->tabId = ui->tabChooser->addTab(_track->trackWidgetItem, _track->trackData->name);
    }
    ui->tabChooser->setCurrentWidget(_track->trackWidgetItem);
    this->setUpdatesEnabled(true);
}

void MainWindow::renameTab(trackHandler* _track)
{
    ui->tabChooser->setTabText(_track->tabId, _track->trackData->name);
}

void MainWindow::sectionChanged()
{

}

void MainWindow::on_tabChooser_currentChanged(int index)
{
    if(phantomChanges) return;

    phantomChanges = true;

    if(mGraphWidget) {
        mGraphWidget->setParent(NULL);
        mGraphWidget = NULL;
    }
    if(index) {
        //this->updatesEnabled(false);
        trackWidget* widget = (trackWidget*)ui->tabChooser->widget(index);
        mGraphWidget = widget->inTrack->graphWidgetItem;
        ui->vertSplitter->insertWidget(1, mGraphWidget);
        mGraphWidget->show();
        mGraphWidget->update();
        ui->tabChooser->setCurrentIndex(0); // go to index 0 to apply size changes
        for(int i = 0; i < ui->tabChooser->count(); ++i) {
            ui->tabChooser->widget(i)->setGeometry(0, 0, ui->tabChooser->widget(i)->width(), ui->tabChooser->widget(i)->height()-150);
        }
        ui->tabChooser->setGeometry(0, 0, ui->tabChooser->width(), ui->tabChooser->height()-150);
        ui->tabChooser->setCurrentIndex(index);
    }
    setUndoButtons();
    phantomChanges = false;
}

void MainWindow::on_tabChooser_tabCloseRequested(int index)
{
    if(!index) {
        return;
    }

    trackWidget* widget = (trackWidget*)ui->tabChooser->widget(index);
    widget->clearSelection();
    widget->inTrack->tabId = -1;

    ui->tabChooser->removeTab(index);
}

void MainWindow::keyPressed(QEvent *event)
{
    keyPressEvent((QKeyEvent*)event);
}

void MainWindow::keyReleased(QEvent *event)
{
    keyReleaseEvent((QKeyEvent*)event);
}

void MainWindow::updateProjectWidget()
{
    ui->projectTab->on_trackListWidget_itemSelectionChanged();
}

void MainWindow::showMessage(QString msg, int msec)
{
    emit emitMessage(msg, msec);
}

void MainWindow::doAutoSave()
{
    backupSave();
}

void MainWindow::hideAll()
{
    ui->centralWidget->layout()->setContentsMargins(0, 0, 0, 0);
    ui->menuBar->hide();
    ui->infoFrame->hide();
    if(mGraphWidget) mGraphWidget->hide();
    ui->tabFrame->hide();
    ui->statusBar->hide();
}

void MainWindow::showAll()
{
    ui->centralWidget->layout()->setContentsMargins(9, 9, 9, 9);
    ui->menuBar->show();
    ui->infoFrame->show();
    if(mGraphWidget) mGraphWidget->show();
    ui->tabFrame->show();
    ui->statusBar->show();
}

void MainWindow::on_actionExport_Model_As_triggered()
{
    mObjectExporter->update();
    mObjectExporter->show();
}

void MainWindow::on_actionExport_triggered()
{
    if(exportScreen->updateBoxes()) {
        exportScreen->doFastExport();
    } else {
        on_actionExportAs_triggered();
    }
}
