#ifndef EXPORTUI_H
#define EXPORTUI_H

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
#include "projectwidget.h"
#include <QString>

namespace Ui {
class Exportui;
}

class exportUi : public QDialog
{
    Q_OBJECT
    
public:
    explicit exportUi(QWidget *parent = 0);
    exportUi(QWidget *parent, projectWidget* _project);
    ~exportUi();
    void doExport();
    void doExport2();
    void doNL2Export();
    void doFastExport();
    bool updateBoxes();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_exportFromBox_currentIndexChanged(int index);

    void on_exportToBox_currentIndexChanged(int index);

    void on_exportTrackBox_currentIndexChanged(int index);

    void on_exportTypeBox_currentIndexChanged(int index);

private:
    Ui::Exportui *ui;
    const char* cFile;
    projectWidget* project;
    float fPerNode;
    bool phantomChanges;
    int curTrackIndex;
    int curFromIndex;
    int curToIndex;
    QString fileName;
};

#endif // EXPORTUI_H
