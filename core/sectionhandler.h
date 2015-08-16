#ifndef SECTIONHANDLER_H
#define SECTIONHANDLER_H

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

#include "section.h"

class track;
class QTreeWidgetItem;

class sectionHandler
{
public:
    sectionHandler(track* _track, enum secType _type, int _id);
    ~sectionHandler();
    void updateID(int _id);

    enum secType type;
    QTreeWidgetItem* listItem;
    section* sectionData;

private:
    int id;
};

#endif // SECTIONHANDLER_H
