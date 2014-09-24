/*
#    FVD++, an advanced coaster design tool for NoLimits
#    Copyright (C) 2012-2014, Stephan "Lenny" Alt <alt.stephan@web.de>
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

#include "saver.h"

#include <fstream>
#include "exportfuncs.h"

using namespace std;

saver::saver(QString fileName, projectWidget* _project, QMainWindow* _parent)
{
    sFileName = fileName.toLocal8Bit().data();
    project = _project;
    parent = _parent;
}

QString saver::doSave()
{
    fstream fout(this->sFileName, ios::out | ios::binary);
    if(fout == NULL) {
        return QString("Error: File is NULL");
    }

    QString temp = project->saveProject(fout);

    fout.close();
    return temp;
}

QString saver::doLoad()
{
    fstream fin(this->sFileName, ios::in | ios::binary);
    if(fin == NULL) {
        return QString("Error: File is NULL");
    }

    QString temp = project->loadProject(fin);

    fin.close();
    return temp;
}
