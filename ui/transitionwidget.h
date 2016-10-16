#ifndef TRANSITIONWIDGET_H
#define TRANSITIONWIDGET_H

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
#include "graphwidget.h"

namespace Ui {
class transitionWidget;
}

class transitionWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit transitionWidget(QWidget *parent = 0);
    ~transitionWidget();
    void changeSubfunc(subfunc* _newSubfunc);
    subfunc* getSelectedFunc();
    void adjustLengthSteps(secType _type, bool _argument);
    
    graphWidget* mParent;

public slots:
    void on_lengthSpin_valueChanged(double arg1);

    void on_transitionBox_currentIndexChanged(int index);

    void on_changeSpin_valueChanged(double arg1);

    void on_quadraticBox_currentIndexChanged(int index);

    void on_quarticBox_currentIndexChanged(int index);

    void on_quarticSpin_valueChanged(double arg1);

    void on_quinticBox_currentIndexChanged(int index);

    void on_quinticSpin_valueChanged(double arg1);

    void on_centerSpin_valueChanged(double arg1);

    void on_tensionSpin_valueChanged(double arg1);

    void on_appendButton_released();

    void on_prependButton_released();

    void on_removeButton_released();

    void on_lockCheck_stateChanged(int arg1);

private:
    Ui::transitionWidget *ui;
    subfunc* selectedFunc;
    bool phantomChanges;
};

#endif // TRANSITIONWIDGET_H
