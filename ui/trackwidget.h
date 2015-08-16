#ifndef TRACKWIDGET_H
#define TRACKWIDGET_H

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

#include <QWidget>
#include "section.h"
#include "smoothui.h"


class trackHandler;
class sectionHandler;
class QTreeWidgetItem;

namespace Ui {
class trackWidget;
}

class trackWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit trackWidget(QWidget *parent = NULL);
    explicit trackWidget(QWidget *parent, trackHandler* _track);
    int getSection(QTreeWidgetItem *item);
    ~trackWidget();

    void clearSelection();
    void setNames();
    void writeNames();

    void setupAnchorFrame();
    void setupStraightFrame();
    void setupCurvedFrame();
    void setupAdvFrame();
    void updateSectionFrame();
    void updateOptionsFrame();

    void updateAnchorGeometrics();
    void setSelection(int index);

    QList<sectionHandler*> sectionList;
    sectionHandler* selSection;
    trackHandler* inTrack;

    smoothUi* smoothScreen;

    void keyPressEvent(QKeyEvent* event);

signals:
    void done();

public slots:
    void on_addButton_released();
    void addStraightSec();
    void addCurvedSec();
    void addForceSec();
    void addGeometricSec();
    void addSection(secType _type);
    void appendStraightSec();
    void appendCurvedSec();
    void appendForceSec();
    void appendGeometricSec();
    void appendSection(secType _type);
    void on_sectionListWidget_itemSelectionChanged();

    void on_deleteButton_released();

    void on_xBox_valueChanged(double arg1);

    void on_yBox_valueChanged(double arg1);

    void on_zBox_valueChanged(double arg1);

    void on_rBox_valueChanged(double arg1);

    void on_pBox_valueChanged(double arg1);

    void on_jBox_valueChanged(double arg1);

    void on_anchorSpeedBox_valueChanged(double arg1);

    void on_straightSpeedCheck_stateChanged(int arg1);

    void on_straightSpeedBox_valueChanged(double arg1);

    void on_straightLengthBox_valueChanged(double arg1);

    void on_curvedSpeedCheck_stateChanged(int arg1);

    void on_curvedSpeedBox_valueChanged(double arg1);

    void on_curvedRadiusBox_valueChanged(double arg1);

    void on_curvedAngleBox_valueChanged(double arg1);

    void on_curvedDirectionBox_valueChanged(double arg1);

    void on_curvedLeadInBox_valueChanged(double arg1);

    void on_curvedLeadOutBox_valueChanged(double arg1);

    void on_advancedSpeedCheck_stateChanged(int arg1);

    void on_advancedSpeedBox_valueChanged(double arg1);

    void on_sectionListWidget_customContextMenuRequested(const QPoint &pos);

    void on_orientationBox_currentIndexChanged(int index);

    void on_argumentBox_currentIndexChanged(int index);

    void on_normalBox_valueChanged(double arg1);

    void on_lateralBox_valueChanged(double arg1);

    void on_smoothButton_released();

    void on_pitchChangeBox_valueChanged(double arg1);

    void on_yawChangeBox_valueChanged(double arg1);

private slots:
    void on_sectionListWidget_itemChanged(QTreeWidgetItem *item, int column);

    void update();


private:
    void updateSectionIDs();
    Ui::trackWidget *ui;
    bool phantomChanges;
};

#endif // TRACKWIDGET_H
