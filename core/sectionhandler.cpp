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

#include "sectionhandler.h"
#include "track.h"
#include <QTreeWidgetItem>

sectionHandler::sectionHandler(track* _track, enum secType _type, int _id)
{
    type = _type;
    id = _id;
    QString name;
    sectionData = NULL;
    if(type == anchor)
    {
        name = QString("");
        sectionData = NULL;
    }
    else
    {
        name = QString("unnamed");
        _track->newSection(type, id-1);
        sectionData = _track->lSections[id-1];
        sectionData->sName = name;
    }

    listItem = new QTreeWidgetItem();
    listItem->setText(1, name);
    listItem->setText(0, QString().number(id));
    listItem->setTextAlignment(0, Qt::AlignHCenter | Qt::AlignVCenter);
    switch(_type)
    {
    case straight:
        listItem->setText(2, QString("Straight"));
        listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);
        break;
    case curved:
        listItem->setText(2, QString("Curved"));
        listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);
        break;
    case forced:
        listItem->setText(2, QString("Force"));
        listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);
        break;
    case geometric:
        listItem->setText(2, QString("Geometric"));
        listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);
        break;
    case bezier:
        listItem->setText(2, QString("Bezier"));
        listItem->setFlags(listItem->flags() | Qt::ItemIsEditable);
        break;
    case anchor:
        listItem->setText(2, QString("Anchor"));
        break;
    case nolimitscsv:
        listItem->setText(2, QString("NoLimits CSV"));
        break;
    }
}

sectionHandler::~sectionHandler()
{
    delete listItem;
}

void sectionHandler::updateID(int _id)
{
    id = _id;
    listItem->setText(0, QString().number(id));
}
