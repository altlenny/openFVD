#ifndef CONVERSIONPANEL_H
#define CONVERSIONPANEL_H

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

namespace Ui {
class conversionPanel;
}

class conversionPanel : public QDialog
{
    Q_OBJECT
    
public:
    explicit conversionPanel(QWidget *parent = 0);
    ~conversionPanel();
    
private slots:
    void on_buttonBox_rejected();

    void on_buttonBox_accepted();

    void on_mpsSpin_valueChanged(double arg1);

    void on_kphSpin_valueChanged(double arg1);

    void on_mphSpin_valueChanged(double arg1);

    void on_meterSpin_valueChanged(double arg1);

    void on_feetSpin_valueChanged(double arg1);

    void on_mileSpin_valueChanged(double arg1);

private:
    Ui::conversionPanel *ui;
    bool phantomChanges;
};

#endif // CONVERSIONPANEL_H
