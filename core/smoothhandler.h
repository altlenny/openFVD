#ifndef SMOOTHHANDLER_H
#define SMOOTHHANDLER_H

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

#include <iostream>

class QTreeWidgetItem;
class track;
class section;

class smoothHandler
{
public:
    smoothHandler(track* _track, int _section, char* customChar = NULL, int _length = 400, int _iterations = 1, int _fromNode = 0, int _toNode = -1);
    ~smoothHandler();

    void update(char* customChar = NULL);


    QTreeWidgetItem* treeItem;

    section* sec;

    int getFrom();
    int getTo();
    int getLength();
    int getIterations();

    void setFrom(int _arg);
    void setTo(int _arg);
    void setLength(int _arg);
    void setIterations(int _arg);

    void saveSmooth(std::fstream& file);
    void loadSmooth(std::fstream& file);
    void legacyLoadSmooth(std::fstream& file);

    bool active;

private:
    track* m_track;
    int fromNode;
    int toNode;
    int length;
    int iterations;
};

#endif // SMOOTHHANDLER_H
