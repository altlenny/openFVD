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

#include "smoothhandler.h"
#include "track.h"
#include <QTreeWidgetItem>

#include "exportfuncs.h"

/*
  List Columns:
  0     Item Number
  1     Name
  2     From
  3     To
  4     Length
  5     Iterations
  6     Enabled
  */

smoothHandler::smoothHandler(track* _track, int _section, char* customChar, int _length, int _iterations, int _fromNode, int _toNode)
{
    m_track = _track;
    treeItem = new QTreeWidgetItem();
    active = false;

    if(_section == -1)
    {
        sec = (section*)-1;
    }
    else if(_section == -2)
    {
        sec = NULL;
        active = true;
    }
    else
    {
        sec = m_track->lSections[_section];
    }
    if(sec == NULL)
    {
        toNode = _toNode;
        fromNode = _fromNode;
        treeItem->setText(1, QString("custom Region"));
        treeItem->setFlags(treeItem->flags() | Qt::ItemIsEditable);
    }
    length = _length;
    iterations = _iterations;

    update(customChar);
}

smoothHandler::~smoothHandler()
{
    if(treeItem) delete treeItem;
}


void smoothHandler::update(char* customChar)
{
    if(sec == (section*)-1)
    {
        toNode = m_track->getNumPoints();
        fromNode = 0;
    }
    else if(sec != NULL)
    {
        fromNode = m_track->getNumPoints(sec);
        toNode = fromNode + sec->lNodes.size() - 2;
    }

    if(sec == NULL)
    {
        if(customChar == NULL)
        {
            treeItem->setText(0, QString());
        }
        else
        {
            treeItem->setText(0, QString(*customChar));
            (*customChar)++;
        }
    }
    else
    {
        if(sec == (section*)-1)
        {
            treeItem->setText(0, QString::number(0));
            treeItem->setText(1, m_track->name);
        }
        else
        {
            treeItem->setText(0, QString::number(m_track->getSectionNumber(sec)+1));
            treeItem->setText(1, sec->sName);
        }
    }
    treeItem->setText(2, QString::number(fromNode/1000.).append("s"));
    treeItem->setText(3, QString::number(toNode/1000.).append("s"));
    treeItem->setText(4, QString::number(length/1000.).append("s"));
    treeItem->setText(5, QString::number(iterations));
    treeItem->setCheckState(6, active ? Qt::Checked : Qt::Unchecked);
}

int smoothHandler::getFrom()
{
    return fromNode;
}

int smoothHandler::getTo()
{
    return toNode;
}

int smoothHandler::getLength()
{
    return length;
}

int smoothHandler::getIterations()
{
    return iterations;
}

void smoothHandler::setFrom(int _arg)
{
    fromNode = _arg;
    treeItem->setText(2, QString::number(fromNode/1000.).append("s"));
}
void smoothHandler::setTo(int _arg)
{
    toNode = _arg;
    treeItem->setText(3, QString::number(toNode/1000.).append("s"));
}

void smoothHandler::setLength(int _arg)
{
    length = _arg;
    treeItem->setText(4, QString::number(length/1000.).append("s"));
}

void smoothHandler::setIterations(int _arg)
{
    iterations = _arg;
    treeItem->setText(5, QString::number(iterations));
}

void smoothHandler::saveSmooth(std::fstream& file)
{
    QString name = treeItem->text(1);
    int namelength = name.length();
    std::string stdName = name.toStdString();

    writeBytes(&file, (const char*)&namelength, sizeof(int));
    file << stdName;

    writeBytes(&file, (const char*)&fromNode, sizeof(int));
    writeBytes(&file, (const char*)&toNode, sizeof(int));
    writeBytes(&file, (const char*)&length, sizeof(int));
    writeBytes(&file, (const char*)&iterations, sizeof(int));
    writeBytes(&file, (const char*)&active, sizeof(bool));
}

void smoothHandler::loadSmooth(std::fstream &file)
{
    int namelength = readInt(&file);
    QString name = QString(readString(&file, namelength).c_str());

    treeItem->setText(1, name);

    setFrom(readInt(&file));
    setTo(readInt(&file));
    setLength(readInt(&file));
    setIterations(readInt(&file));
    active = readBool(&file);

    update();
}

void smoothHandler::legacyLoadSmooth(std::fstream &file)
{
    int namelength = readInt(&file);
    QString name = QString(readString(&file, namelength).c_str());

    treeItem->setText(1, name);

    setFrom(readInt(&file));
    setTo(readInt(&file));
    setLength(readInt(&file));
    setIterations(readInt(&file));
    active = readBool(&file);

    update();
}
