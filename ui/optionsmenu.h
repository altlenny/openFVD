#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

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

#include <QDialog>
#include <QString>
#include <QApplication>
#include <QDir>

enum eGLPolicy
{
    smart = 0,
    legacy = 1,
    forceCore = 2
};

enum eMeasures
{
    metricMPS = 0,
    metricKPH = 1,
    english = 2
};

class QColorDialog;

namespace Ui {
class optionsMenu;
}

class optionsMenu : public QDialog
{
    Q_OBJECT
    
public:
    explicit optionsMenu(QWidget *parent = 0);

    QString getSpeedString();
    QString getLengthString();
    double getSpeedFactor();
    double getLengthFactor();
    void saveToOptionsFile();
    bool loadFromOptionsFile();
    void setGLVersionString(QString version);
    ~optionsMenu();


    int maxUndoChanges;
    int measures;
    int glPolicy;
    QColor rollColor[4];
    QColor normColor[4];
    QColor latColor[4];
    QColor pitchColor[4];
    QColor yawColor[4];
    QColorDialog* colorPicker;
    int shadowQuality;
    int meshQuality;
    float fov;

    bool drawGrid;
    QColor backgroundColor;

private slots:
    void on_rollColor0_clicked();
    void on_rollColor1_clicked();
    void on_rollColor2_clicked();
    void on_rollColor3_clicked();
    void on_normColor0_clicked();
    void on_normColor1_clicked();
    void on_normColor2_clicked();
    void on_normColor3_clicked();
    void on_latColor0_clicked();
    void on_latColor1_clicked();
    void on_latColor2_clicked();
    void on_latColor3_clicked();
    void on_pitchColor2_clicked();
    void on_pitchColor3_clicked();
    void on_yawColor0_clicked();
    void on_yawColor1_clicked();
    void on_yawColor2_clicked();
    void on_yawColor3_clicked();

    void on_pitchColor1_clicked();
    void on_pitchColor0_clicked();
    void onRollColor0_received();
    void onRollColor1_received();
    void onRollColor2_received();
    void onRollColor3_received();
    void onNormColor0_received();
    void onNormColor1_received();
    void onNormColor2_received();
    void onNormColor3_received();
    void onLatColor0_received();
    void onLatColor1_received();
    void onLatColor2_received();
    void onLatColor3_received();
    void onPitchColor0_received();
    void onPitchColor1_received();
    void onPitchColor2_received();
    void onPitchColor3_received();
    void onYawColor0_received();
    void onYawColor1_received();
    void onYawColor2_received();
    void onYawColor3_received();

    void on_buttonBox_accepted();

    void on_measureBox_currentIndexChanged(int index);

    void on_glBox_currentIndexChanged(int index);

    void on_backgroundButton_released();

    void onbackgroundColor_received();

    void on_gridBox_stateChanged(int arg1);


    void on_shadowModeBox_currentIndexChanged(int index);

    void on_fovBox_valueChanged(double arg1);

    void on_fovSlider_valueChanged(int value);

    void on_meshQualityBox_currentIndexChanged(int index);

private:
    Ui::optionsMenu *ui;

    QString optionsFile;

    bool phantomChanges;
};

#endif // OPTIONSMENU_H
