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

#include "trackhandler.h"
#include "graphwidget.h"
#include "glviewwidget.h"
#include "undohandler.h"
#include "optionsmenu.h"
#include "mainwindow.h"
#include "trackmesh.h"
#include "trackwidget.h"
#include <QTreeWidgetItem>

extern MainWindow* gloParent;
extern glViewWidget* glView;

trackHandler::trackHandler(QString _name, int _id)
{
    id = _id;
    this->trackData = new track(this, glm::vec3(0.f, 5.f, 0.f), 0, 1.1);
    trackData->name = _name;
    this->listItem = new QTreeWidgetItem(id);
    listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);
    listItem->setText(0, QString().number(id));
    listItem->setTextAlignment(0, Qt::AlignHCenter | Qt::AlignVCenter);
    listItem->setText(1, trackData->name);
    listItem->setCheckState(2, Qt::Checked);
    graphWidgetItem = new graphWidget(NULL, this);
    graphWidgetItem->setPalette(gloParent->palette());
    graphWidgetItem->hide();
    trackWidgetItem = new trackWidget(NULL, this);
    trackWidgetItem->setPalette(gloParent->palette());
    trackWidgetItem->hide();

    tabId = -1;

    trackColors[0] = QColor(20, 20, 130);
    trackColors[1] = QColor(255, 51, 51);
    trackColors[2] = QColor(51, 255, 51);


    mUndoHandler = new undoHandler(gloParent->mOptions->maxUndoChanges);
    mMesh = new trackMesh(trackData);
}

trackHandler::~trackHandler()
{
    delete trackWidgetItem;
    delete graphWidgetItem;
    delete listItem;
    delete trackData;
    delete mUndoHandler;
    delete mMesh;
}

void trackHandler::changeID(int _id)
{
    id = _id;
    listItem->setText(0, QString().number(_id));
}

int trackHandler::getID()
{
    return id;
}
