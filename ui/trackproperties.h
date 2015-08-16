#ifndef TRACKPROPERTIES_H
#define TRACKPROPERTIES_H

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
#include <QColor>

class QColorDialog;
class trackHandler;

namespace Ui {
class TrackProperties;
}

class TrackProperties : public QDialog
{
    Q_OBJECT
    
public:
    explicit TrackProperties(QWidget *parent = 0);
    ~TrackProperties();

    void openForTrack(trackHandler* _curTrack);

    trackHandler* curTrack;
    
public slots:
    void on_frictionBox_valueChanged(double arg1);

    void on_heartlineBox_valueChanged(double arg1);

    void on_defaultColorButton_released();

    void ondefaultColor_received();

    void on_sectionColorButton_released();

    void onsectionColor_received();

    void on_transitionColorButton_released();

    void ontransitionColor_received();

    void on_drawBox_currentIndexChanged(int index);

    void on_styleBox_currentIndexChanged(int index);

    void on_wireframeBox_stateChanged(int arg1);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    QColorDialog* colorPicker;
    Ui::TrackProperties *ui;
};

#endif // TRACKPROPERTIES_H
