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

#include "myqdoublespinbox.h"
#include <QWheelEvent>
#include "mainwindow.h"

extern MainWindow* gloParent;

myQDoubleSpinBox::myQDoubleSpinBox(QWidget* parent) : QDoubleSpinBox(parent)
{
}

myQDoubleSpinBox::~myQDoubleSpinBox()
{
}

void myQDoubleSpinBox::stepBy(int steps)
{
    double jumpBy = steps*singleStep();
    if(QApplication::keyboardModifiers() == Qt::ControlModifier) {
        jumpBy *= 0.1;
    }
    if(QApplication::keyboardModifiers() == Qt::ShiftModifier) {
        jumpBy *= 10.;
    }
    double newval = this->value()+jumpBy > this->maximum() ? this->maximum() : this->value()+jumpBy;
    newval = newval < this->minimum() ? this->minimum() : newval;
    this->setValue(newval);
}

void myQDoubleSpinBox::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        stepBy(event->delta()/120);
    } else {
       QDoubleSpinBox::wheelEvent(event);
    }
}

void myQDoubleSpinBox::keyPressEvent(QKeyEvent* event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier) && event->key() == Qt::Key_Z) {
        gloParent->on_actionUndo_triggered();
        event->ignore();
    } else if (event->modifiers().testFlag(Qt::ControlModifier) && event->key() == Qt::Key_Y) {
        gloParent->on_actionRedo_triggered();
        event->ignore();
    } else {
        QDoubleSpinBox::keyPressEvent(event);
    }
}

void myQDoubleSpinBox::keyReleaseEvent(QKeyEvent* event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        gloParent->keyReleased(event);
        event->ignore();
    } else {
        QDoubleSpinBox::keyReleaseEvent(event);
    }
}
