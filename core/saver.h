#ifndef SAVER_H
#define SAVER_H

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

#include "projectwidget.h"
#include "track.h"
#include <QString>

class projectWidget;
class QMainWindow;

class saver
{
public:
    saver(const QString& fileName, projectWidget* _project, QMainWindow *_parent);
    QString doSave();
    QString doLoad();

    track* saveTo;
    projectWidget* project;
    QString sFileName;
    QMainWindow *parent;
};

#endif // SAVER_H
