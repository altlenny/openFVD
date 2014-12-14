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

#include "optionsmenu.h"
#include "ui_optionsmenu.h"
#include "qcolordialog.h"
#include <fstream>
#include "undohandler.h"
#include "osx/common.h"
#include "glviewwidget.h"
#include "mainwindow.h"
#include "trackmesh.h"

extern MainWindow* gloParent;
extern glViewWidget* glView;

optionsMenu::optionsMenu(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::optionsMenu)
{
    maxUndoChanges = 50000;
    phantomChanges = false;

#ifdef Q_OS_MAC
    this->setFixedSize(700, 700);
    this->setMinimumSize(700, 700);
    this->setMaximumSize(700, 700);
#endif

    optionsFile = common::getResource("options.cfg", true);

    if(!QFileInfo(QString(optionsFile)).exists() || !loadFromOptionsFile()) {
        measures = 0;
        drawGrid = true;
        backgroundColor = QColor(80, 140, 160);
#ifndef Q_OS_MAC // on Win / Unix
        glPolicy = 0;
#endif
#ifdef Q_OS_MAC
        glPolicy = 1;
#endif
        meshQuality = 1;
        shadowQuality = 1;
        fov = 90.0;
        rollColor[0] = QColor(120, 0, 0, 255);
        rollColor[1] = QColor(120, 0, 0, 25);
        rollColor[2] = QColor(255, 0, 0, 255);
        rollColor[3] = QColor(255, 0, 0, 25);
        normColor[0] = QColor(0, 0, 120, 255);
        normColor[1] = QColor(0, 0, 120, 25);
        normColor[2] = QColor(0, 0, 255, 255);
        normColor[3] = QColor(0, 0, 255, 25);
        latColor[0] = QColor(0, 120, 0, 255);
        latColor[1] = QColor(0, 120, 0, 25);
        latColor[2] = QColor(0, 255, 0, 255);
        latColor[3] = QColor(0, 255, 0, 25);
        pitchColor[0] = QColor(0, 120, 120, 255);
        pitchColor[1] = QColor(0, 120, 120, 25);
        pitchColor[2] = QColor(0, 255, 255, 255);
        pitchColor[3] = QColor(0, 255, 255, 25);
        yawColor[0] = QColor(120, 120, 0, 255);
        yawColor[1] = QColor(120, 120, 0, 25);
        yawColor[2] = QColor(255, 255, 0, 255);
        yawColor[3] = QColor(255, 255, 0, 25);
        saveToOptionsFile();
    }

    ui->setupUi(this);

    #ifdef Q_OS_MAC
    ui->distanceLabel_2->setVisible(false);
    ui->glBox->setVisible(false);
    #endif

    ui->gridBox->setChecked(drawGrid);

    colorPicker = new QColorDialog(this);
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    QPalette palette = ui->rollColor0->palette();
    palette.setColor(QPalette::ButtonText, backgroundColor);
    ui->backgroundButton->setPalette(palette);
    palette.setColor(QPalette::ButtonText, rollColor[0]);
    ui->rollColor0->setPalette(palette);
    palette.setColor(QPalette::ButtonText, rollColor[1]);
    ui->rollColor1->setPalette(palette);
    palette.setColor(QPalette::ButtonText, rollColor[2]);
    ui->rollColor2->setPalette(palette);
    palette.setColor(QPalette::ButtonText, rollColor[3]);
    ui->rollColor3->setPalette(palette);

    palette.setColor(QPalette::ButtonText, normColor[0]);
    ui->normColor0->setPalette(palette);
    palette.setColor(QPalette::ButtonText, normColor[1]);
    ui->normColor1->setPalette(palette);
    palette.setColor(QPalette::ButtonText, normColor[2]);
    ui->normColor2->setPalette(palette);
    palette.setColor(QPalette::ButtonText, normColor[3]);
    ui->normColor3->setPalette(palette);

    palette.setColor(QPalette::ButtonText, latColor[0]);
    ui->latColor0->setPalette(palette);
    palette.setColor(QPalette::ButtonText, latColor[1]);
    ui->latColor1->setPalette(palette);
    palette.setColor(QPalette::ButtonText, latColor[2]);
    ui->latColor2->setPalette(palette);
    palette.setColor(QPalette::ButtonText, latColor[3]);
    ui->latColor3->setPalette(palette);

    palette.setColor(QPalette::ButtonText, pitchColor[0]);
    ui->pitchColor0->setPalette(palette);
    palette.setColor(QPalette::ButtonText, pitchColor[1]);
    ui->pitchColor1->setPalette(palette);
    palette.setColor(QPalette::ButtonText, pitchColor[2]);
    ui->pitchColor2->setPalette(palette);
    palette.setColor(QPalette::ButtonText, pitchColor[3]);
    ui->pitchColor3->setPalette(palette);

    palette.setColor(QPalette::ButtonText, yawColor[0]);
    ui->yawColor0->setPalette(palette);
    palette.setColor(QPalette::ButtonText, yawColor[1]);
    ui->yawColor1->setPalette(palette);
    palette.setColor(QPalette::ButtonText, yawColor[2]);
    ui->yawColor2->setPalette(palette);
    palette.setColor(QPalette::ButtonText, yawColor[3]);
    ui->yawColor3->setPalette(palette);

    phantomChanges = true;
    ui->fovBox->setValue(fov);
    ui->fovSlider->setValue(fov*10);
    ui->shadowModeBox->setCurrentIndex(shadowQuality);
    ui->meshQualityBox->setCurrentIndex(meshQuality);
    phantomChanges = false;
    this->ui->measureBox->setCurrentIndex(measures);
#ifndef Q_OS_MAC // on Win / Unix
    this->ui->glBox->setCurrentIndex(glPolicy);
#endif
#ifdef Q_OS_MAC
    this->ui->glBox->setCurrentIndex(glPolicy-1);
#endif
}

optionsMenu::~optionsMenu()
{
    delete ui;
}

QString optionsMenu::getSpeedString()
{
    switch(measures) {
    case 0:
        return QString("m/s");
        break;
    case 1:
        return QString("km/h");
        break;
    case 2:
        return QString("mph");
        break;
    default:
        return QString("error");
        break;
    }
}

QString optionsMenu::getLengthString()
{
    switch(measures) {
    case 0:
        return QString("m");
        break;
    case 1:
        return QString("m");
        break;
    case 2:
        return QString("ft");
        break;
    default:
        return QString("error");
        break;
    }
}

double optionsMenu::getSpeedFactor()
{
    switch(measures) {
    case 0:
        return 1.0;
        break;
    case 1:
        return 3.6;
        break;
    case 2:
        return 2.2369356;
        break;
    default:
        return 0.0;
        break;
    }
}

double optionsMenu::getLengthFactor()
{
    switch(measures) {
    case 0:
        return 1.0;
        break;
    case 1:
        return 1.0;
        break;
    case 2:
        return 1/0.3048;
        break;
    default:
        return 0.0;
        break;
    }
}

void optionsMenu::on_rollColor0_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(rollColor[0]);
    colorPicker->open(this, SLOT(onRollColor0_received()));
}

void optionsMenu::onRollColor0_received()
{
    rollColor[0] = colorPicker->selectedColor();
    QPalette palette = ui->rollColor0->palette();
    palette.setColor(QPalette::ButtonText, rollColor[0]);
    ui->rollColor0->setPalette(palette);
}

void optionsMenu::on_rollColor1_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(rollColor[1]);
    colorPicker->open(this, SLOT(onRollColor1_received()));
}

void optionsMenu::onRollColor1_received()
{
    rollColor[1] = colorPicker->selectedColor();
    QPalette palette = ui->rollColor1->palette();
    palette.setColor(QPalette::ButtonText, rollColor[1]);
    ui->rollColor1->setPalette(palette);
}

void optionsMenu::on_rollColor2_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(rollColor[2]);
    colorPicker->open(this, SLOT(onRollColor2_received()));
}

void optionsMenu::onRollColor2_received()
{
    rollColor[2] = colorPicker->selectedColor();
    QPalette palette = ui->rollColor2->palette();
    palette.setColor(QPalette::ButtonText, rollColor[2]);
    ui->rollColor2->setPalette(palette);
}

void optionsMenu::on_rollColor3_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(rollColor[3]);
    colorPicker->open(this, SLOT(onRollColor3_received()));
}

void optionsMenu::onRollColor3_received()
{
    rollColor[3] = colorPicker->selectedColor();
    QPalette palette = ui->rollColor3->palette();
    palette.setColor(QPalette::ButtonText, rollColor[3]);
    ui->rollColor3->setPalette(palette);
}

void optionsMenu::on_normColor0_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(normColor[0]);
    colorPicker->open(this, SLOT(onNormColor0_received()));
}

void optionsMenu::onNormColor0_received()
{
    normColor[0] = colorPicker->selectedColor();
    QPalette palette = ui->normColor0->palette();
    palette.setColor(QPalette::ButtonText, normColor[0]);
    ui->normColor0->setPalette(palette);
}

void optionsMenu::on_normColor1_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(normColor[1]);
    colorPicker->open(this, SLOT(onNormColor1_received()));
}

void optionsMenu::onNormColor1_received()
{
    normColor[1] = colorPicker->selectedColor();
    QPalette palette = ui->normColor1->palette();
    palette.setColor(QPalette::ButtonText, normColor[1]);
    ui->normColor1->setPalette(palette);
}

void optionsMenu::on_normColor2_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(normColor[2]);
    colorPicker->open(this, SLOT(onNormColor2_received()));
}

void optionsMenu::onNormColor2_received()
{
    normColor[2] = colorPicker->selectedColor();
    QPalette palette = ui->normColor2->palette();
    palette.setColor(QPalette::ButtonText, normColor[2]);
    ui->normColor2->setPalette(palette);
}

void optionsMenu::on_normColor3_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(normColor[3]);
    colorPicker->open(this, SLOT(onNormColor3_received()));
}

void optionsMenu::onNormColor3_received()
{
    normColor[3] = colorPicker->selectedColor();
    QPalette palette = ui->normColor3->palette();
    palette.setColor(QPalette::ButtonText, normColor[3]);
    ui->normColor3->setPalette(palette);
}

void optionsMenu::on_latColor0_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(latColor[0]);
    colorPicker->open(this, SLOT(onLatColor0_received()));
}

void optionsMenu::onLatColor0_received()
{
    latColor[0] = colorPicker->selectedColor();
    QPalette palette = ui->latColor0->palette();
    palette.setColor(QPalette::ButtonText, latColor[0]);
    ui->latColor0->setPalette(palette);
}

void optionsMenu::on_latColor1_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(latColor[1]);
    colorPicker->open(this, SLOT(onLatColor1_received()));
}

void optionsMenu::onLatColor1_received()
{
    latColor[1] = colorPicker->selectedColor();
    QPalette palette = ui->latColor1->palette();
    palette.setColor(QPalette::ButtonText, latColor[1]);
    ui->latColor1->setPalette(palette);
}

void optionsMenu::on_latColor2_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(latColor[2]);
    colorPicker->open(this, SLOT(onLatColor2_received()));
}

void optionsMenu::onLatColor2_received()
{
    latColor[2] = colorPicker->selectedColor();
    QPalette palette = ui->latColor2->palette();
    palette.setColor(QPalette::ButtonText, latColor[2]);
    ui->latColor2->setPalette(palette);
}

void optionsMenu::on_latColor3_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(latColor[3]);
    colorPicker->open(this, SLOT(onLatColor3_received()));
}

void optionsMenu::onLatColor3_received()
{
    latColor[3] = colorPicker->selectedColor();
    QPalette palette = ui->latColor3->palette();
    palette.setColor(QPalette::ButtonText, latColor[3]);
    ui->latColor3->setPalette(palette);
}

void optionsMenu::on_pitchColor0_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(pitchColor[0]);
    colorPicker->open(this, SLOT(onPitchColor0_received()));
}

void optionsMenu::onPitchColor0_received()
{
    pitchColor[0] = colorPicker->selectedColor();
    QPalette palette = ui->pitchColor0->palette();
    palette.setColor(QPalette::ButtonText, pitchColor[0]);
    ui->pitchColor0->setPalette(palette);
}

void optionsMenu::on_pitchColor1_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(pitchColor[1]);
    colorPicker->open(this, SLOT(onPitchColor1_received()));
}

void optionsMenu::onPitchColor1_received()
{
    pitchColor[1] = colorPicker->selectedColor();
    QPalette palette = ui->pitchColor1->palette();
    palette.setColor(QPalette::ButtonText, pitchColor[1]);
    ui->pitchColor1->setPalette(palette);
}

void optionsMenu::on_pitchColor2_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(pitchColor[2]);
    colorPicker->open(this, SLOT(onPitchColor2_received()));
}

void optionsMenu::onPitchColor2_received()
{
    pitchColor[2] = colorPicker->selectedColor();
    QPalette palette = ui->pitchColor2->palette();
    palette.setColor(QPalette::ButtonText, pitchColor[2]);
    ui->pitchColor2->setPalette(palette);
}

void optionsMenu::on_pitchColor3_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(pitchColor[3]);
    colorPicker->open(this, SLOT(onPitchColor3_received()));
}

void optionsMenu::onPitchColor3_received()
{
    pitchColor[3] = colorPicker->selectedColor();
    QPalette palette = ui->pitchColor3->palette();
    palette.setColor(QPalette::ButtonText, pitchColor[3]);
    ui->pitchColor3->setPalette(palette);
}

void optionsMenu::on_yawColor0_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(yawColor[0]);
    colorPicker->open(this, SLOT(onYawColor0_received()));
}

void optionsMenu::onYawColor0_received()
{
    yawColor[0] = colorPicker->selectedColor();
    QPalette palette = ui->yawColor0->palette();
    palette.setColor(QPalette::ButtonText, yawColor[0]);
    ui->yawColor0->setPalette(palette);
}

void optionsMenu::on_yawColor1_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(yawColor[1]);
    colorPicker->open(this, SLOT(onYawColor1_received()));
}

void optionsMenu::onYawColor1_received()
{
    yawColor[1] = colorPicker->selectedColor();
    QPalette palette = ui->yawColor1->palette();
    palette.setColor(QPalette::ButtonText, yawColor[1]);
    ui->yawColor1->setPalette(palette);
}

void optionsMenu::on_yawColor2_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(yawColor[2]);
    colorPicker->open(this, SLOT(onYawColor2_received()));
}

void optionsMenu::onYawColor2_received()
{
    yawColor[2] = colorPicker->selectedColor();
    QPalette palette = ui->yawColor2->palette();
    palette.setColor(QPalette::ButtonText, yawColor[2]);
    ui->yawColor2->setPalette(palette);
}

void optionsMenu::on_yawColor3_clicked()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, true);
    colorPicker->setCurrentColor(yawColor[3]);
    colorPicker->open(this, SLOT(onYawColor3_received()));
}

void optionsMenu::onYawColor3_received()
{
    yawColor[3] = colorPicker->selectedColor();
    QPalette palette = ui->yawColor3->palette();
    palette.setColor(QPalette::ButtonText, yawColor[3]);
    ui->yawColor3->setPalette(palette);
}

void optionsMenu::on_buttonBox_accepted()
{
    saveToOptionsFile();
    gloParent->updateInfoPanel();
}

void optionsMenu::saveToOptionsFile()   // Ercan: Config file Erstellung hier
{
    std::fstream fout(optionsFile.toLocal8Bit().data(), std::ios::out);
    fout << "FVD Options File\ndelete to reset\n\n";
    fout << "Measures " << measures << "\n";
    fout << "DrawGrid " << drawGrid << "\n";
    fout << "backgroundColor " << backgroundColor.red() << " " << backgroundColor.green() << " " << backgroundColor.blue() << "\n";
    fout << "glPolicy " << glPolicy << "\n";
    fout << "fov " << fov << "\n";
    fout << "shadowQuality " << shadowQuality << "\n";
    fout << "meshQuality " << meshQuality << "\n";
    fout << "nSelRollLine " << rollColor[0].red() << " " << rollColor[0].green() << " " << rollColor[0].blue() << " " << rollColor[0].alpha() << "\n";
    fout << "nSelRollBack " << rollColor[1].red() << " " << rollColor[1].green() << " " << rollColor[1].blue() << " " << rollColor[1].alpha() << "\n";
    fout << "selRollLine " << rollColor[2].red() << " " << rollColor[2].green() << " " << rollColor[2].blue() << " " << rollColor[2].alpha() << "\n";
    fout << "selRollBack " << rollColor[3].red() << " " << rollColor[3].green() << " " << rollColor[3].blue() << " " << rollColor[3].alpha() << "\n";

    fout << "nSelNormLine " << normColor[0].red() << " " << normColor[0].green() << " " << normColor[0].blue() << " " << normColor[0].alpha() << "\n";
    fout << "nSelNormBack " << normColor[1].red() << " " << normColor[1].green() << " " << normColor[1].blue() << " " << normColor[1].alpha() << "\n";
    fout << "selNormLine " << normColor[2].red() << " " << normColor[2].green() << " " << normColor[2].blue() << " " << normColor[2].alpha() << "\n";
    fout << "selNormBack " << normColor[3].red() << " " << normColor[3].green() << " " << normColor[3].blue() << " " << normColor[3].alpha() << "\n";

    fout << "nSelLatLine " << latColor[0].red() << " " << latColor[0].green() << " " << latColor[0].blue() << " " << latColor[0].alpha() << "\n";
    fout << "nSelLatBack " << latColor[1].red() << " " << latColor[1].green() << " " << latColor[1].blue() << " " << latColor[1].alpha() << "\n";
    fout << "selLatLine " << latColor[2].red() << " " << latColor[2].green() << " " << latColor[2].blue() << " " << latColor[2].alpha() << "\n";
    fout << "selLatBack " << latColor[3].red() << " " << latColor[3].green() << " " << latColor[3].blue() << " " << latColor[3].alpha() << "\n";

    fout << "nSelPitchLine " << pitchColor[0].red() << " " << pitchColor[0].green() << " " << pitchColor[0].blue() << " " << pitchColor[0].alpha() << "\n";
    fout << "nSelPitchBack " << pitchColor[1].red() << " " << pitchColor[1].green() << " " << pitchColor[1].blue() << " " << pitchColor[1].alpha() << "\n";
    fout << "selPitchLine " << pitchColor[2].red() << " " << pitchColor[2].green() << " " << pitchColor[2].blue() << " " << pitchColor[2].alpha() << "\n";
    fout << "selPitchBack " << pitchColor[3].red() << " " << pitchColor[3].green() << " " << pitchColor[3].blue() << " " << pitchColor[3].alpha() << "\n";

    fout << "nSelYawLine " << yawColor[0].red() << " " << yawColor[0].green() << " " << yawColor[0].blue() << " " << yawColor[0].alpha() << "\n";
    fout << "nSelYawBack " << yawColor[1].red() << " " << yawColor[1].green() << " " << yawColor[1].blue() << " " << yawColor[1].alpha() << "\n";
    fout << "selYawLine " << yawColor[2].red() << " " << yawColor[2].green() << " " << yawColor[2].blue() << " " << yawColor[2].alpha() << "\n";
    fout << "selYawBack " << yawColor[3].red() << " " << yawColor[3].green() << " " << yawColor[3].blue() << " " << yawColor[3].alpha() << "\n";

    fout.close();
}

bool optionsMenu::loadFromOptionsFile()   // Ercan: Config file Auslesen hier
{
    std::fstream fin(optionsFile.toLocal8Bit().data(), std::ios::in);
    bool ok;
    char input[100];
    for(int i = 0; i < 7; ++i)
    {
        fin >> input;
    }
    /*do {
        fin >> input;
    } while(QString(input) != QString("Measures"));*/
    fin >> input;
    measures = QString(input).toInt(&ok);
    if(!ok) return false;
    fin >> input;
    fin >> input;
    drawGrid = QString(input).toInt(&ok);
    if(!ok) return false;
    fin >> input;
    fin >> input;
    backgroundColor.setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    backgroundColor.setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    backgroundColor.setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    fin >> input;
    glPolicy = QString(input).toInt(&ok);
    if(!ok) return false;
    fin >> input;
    fin >> input;
    fov = QString(input).toFloat(&ok);
    if(!ok) return false;
    fin >> input;
    fin >> input;
    shadowQuality = QString(input).toInt(&ok);
    if(!ok) return false;
    fin >> input;
    fin >> input;
    meshQuality = QString(input).toInt(&ok);
    if(!ok) return false;
    fin >> input;
    fin >> input;
    rollColor[0].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    rollColor[0].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    rollColor[0].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    rollColor[0].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    rollColor[1].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    rollColor[1].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    rollColor[1].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    rollColor[1].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    rollColor[2].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    rollColor[2].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    rollColor[2].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    rollColor[2].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    rollColor[3].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    rollColor[3].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    rollColor[3].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    rollColor[3].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    normColor[0].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    normColor[0].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    normColor[0].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    normColor[0].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    normColor[1].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    normColor[1].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    normColor[1].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    normColor[1].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    normColor[2].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    normColor[2].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    normColor[2].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    normColor[2].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    normColor[3].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    normColor[3].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    normColor[3].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    normColor[3].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    latColor[0].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    latColor[0].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    latColor[0].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    latColor[0].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    latColor[1].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    latColor[1].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    latColor[1].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    latColor[1].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    latColor[2].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    latColor[2].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    latColor[2].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    latColor[2].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    latColor[3].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    latColor[3].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    latColor[3].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    latColor[3].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    pitchColor[0].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    pitchColor[0].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    pitchColor[0].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    pitchColor[0].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    pitchColor[1].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    pitchColor[1].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    pitchColor[1].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    pitchColor[1].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    pitchColor[2].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    pitchColor[2].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    pitchColor[2].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    pitchColor[2].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    pitchColor[3].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    pitchColor[3].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    pitchColor[3].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    pitchColor[3].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    yawColor[0].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    yawColor[0].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    yawColor[0].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    yawColor[0].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    yawColor[1].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    yawColor[1].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    yawColor[1].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    yawColor[1].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    yawColor[2].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    yawColor[2].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    yawColor[2].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    yawColor[2].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin >> input;
    fin >> input;
    yawColor[3].setRed(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    yawColor[3].setGreen(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    yawColor[3].setBlue(QString(input).toInt(&ok));
    if(!ok) return false;
    fin >> input;
    yawColor[3].setAlpha(QString(input).toInt(&ok));
    if(!ok) return false;

    fin.close();
    return true;
}

void optionsMenu::on_measureBox_currentIndexChanged(int index)
{
    measures = index;
}

#ifndef Q_OS_MAC // on Win / Unix
void optionsMenu::on_glBox_currentIndexChanged(int index)
{
    glPolicy = index;
}
#endif
#ifdef Q_OS_MAC
void optionsMenu::on_glBox_currentIndexChanged(int index)
{
    glPolicy = ++index;
}
#endif

void optionsMenu::setGLVersionString(QString version)
{
    this->ui->glInfoLabel->setText(QString("detected OpenGL driver:     ").append(version));
}

void optionsMenu::on_backgroundButton_released()
{
    colorPicker->setOption(QColorDialog::ShowAlphaChannel, false);
    colorPicker->setCurrentColor(backgroundColor);
    colorPicker->open(this, SLOT(onbackgroundColor_received()));
}

void optionsMenu::onbackgroundColor_received()
{
    glView->setBackgroundColor(colorPicker->selectedColor());
    backgroundColor = colorPicker->selectedColor();
    QPalette palette = ui->backgroundButton->palette();
    palette.setColor(QPalette::ButtonText, backgroundColor);
    ui->backgroundButton->setPalette(palette);
}

void optionsMenu::on_gridBox_stateChanged(int arg1)
{
    drawGrid = arg1;
}

void optionsMenu::on_shadowModeBox_currentIndexChanged(int index)
{
    shadowQuality = index;
}

void optionsMenu::on_fovBox_valueChanged(double arg1)
{
    phantomChanges = true;
    fov = arg1;
    ui->fovSlider->setValue(arg1*10);
    phantomChanges = false;
}

void optionsMenu::on_fovSlider_valueChanged(int value)
{
    phantomChanges = true;
    fov = value/10.f;
    ui->fovBox->setValue(fov);
    phantomChanges = false;
}

void optionsMenu::on_meshQualityBox_currentIndexChanged(int index)
{
    meshQuality = index;
    if(gloParent->project == NULL) return;
    QList<trackHandler*> TL = gloParent->getTrackList();
    for(int i = 0; i < TL.size(); ++i)
    {
        TL[i]->mMesh->buildMeshes(0);
    }
}
