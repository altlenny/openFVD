#ifndef SMOOTHUI_H
#define SMOOTHUI_H

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

#include "track.h"
#include "trackhandler.h"
#include "graphwidget.h"

class trackWidget;
class smoothHandler;

namespace Ui {
class smoothUi;
}

class smoothUi : public QDialog
{
    Q_OBJECT
    
public:
    explicit smoothUi(trackHandler* _track, QWidget *parent = 0);
    ~smoothUi();

    void applyRollSmooth(int fromNode = 0);
    bool active();

    void updateUi();

    char customChar;

public slots:
    void on_buttonBox_accepted();

private slots:
    void on_smoothUnitTree_itemSelectionChanged();

    void on_lengthBox_valueChanged(double arg1);

    void on_iterBox_valueChanged(int arg1);

    void on_fromBox_valueChanged(double arg1);

    void on_toBox_valueChanged(double arg1);

    void on_smoothUnitTree_itemChanged(QTreeWidgetItem *item, int column);

    void on_newButton_released();

    void on_removeButton_released();

private:
    void applyRollSmoothFilter(smoothHandler* _handler);

    void generateWarnings();

    Ui::smoothUi *ui;

    track* m_track;
    graphWidget* m_widget;
    trackWidget* m_trackwidget;

    bool phantomChanges;
    smoothHandler* curHandler;
};

#endif // SMOOTHUI_H
