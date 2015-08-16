#ifndef IMPORTUI_H
#define IMPORTUI_H

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
#include "projectwidget.h"

namespace Ui {
class importUi;
}

class importUi : public QDialog
{
    Q_OBJECT
    
public:
    explicit importUi(QWidget *parent, QString _fileName);
    QList<trackHandler*> trackList;
    QList<int> posList;
    ~importUi();
    
private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::importUi *ui;
    QString fileName;
    bool legacymode;
};

#endif // IMPORTUI_H
