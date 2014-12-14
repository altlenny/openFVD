#ifndef TRACKHANDLER_H
#define TRACKHANDLER_H

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

#include "glviewwidget.h"

class QTreeWidgetItem;
class trackWidget;
class graphWidget;
class undoHandler;
class trackMesh;

class trackHandler
{
public:
    trackHandler(QString _name, int _id);
    ~trackHandler();
    void changeID(int _id);
    int getID();


    track* trackData;
    QTreeWidgetItem* listItem;
    trackWidget* trackWidgetItem;
    graphWidget* graphWidgetItem;
    int tabId;

    trackMesh* mMesh;
    undoHandler* mUndoHandler;
    QColor trackColors[3];

private:

    int id;
    //mnode* curNode;
    //section* curSection;
    //int j;
};

#endif // TRACKHANDLER_H
