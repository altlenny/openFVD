#ifndef FUNCTION_H
#define FUNCTION_H

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

#include <QList>
#include <QVector>
#include "subfunction.h"
#include <fstream>

enum eFunctype
{
    funcRoll,
    funcNormal,
    funcLateral,
    funcPitch,
    funcYaw
};

class section;

class function
{
public:
    function(float min, float max, float start, float end, section* _parent, enum eFunctype newtype);
    ~function();
    void appendSubFunction(float length, int i = -1);
    void removeSubFunction(int i = -1);

    float getValue(float x);

    void setMaxArgument(float newMax);
    float getMaxArgument() const { return funcList[funcList.size()-1]->maxArgument; }

    float getMaxValue();
    float getMinValue();

    void translateValues(subfunction* caller);

    float changeLength(float newlength, int index);

    int getSubfunctionNumber(subfunction* _sub);

    void saveFunction(std::fstream& file);
    void loadFunction(std::fstream& file);
    void legacyLoadFunction(std::fstream& file);
    void saveFunction(std::stringstream& file);
    void loadFunction(std::stringstream& file);

    bool unlock(int _id);
    bool lock(int _id);

    int lockedFunc();
    subfunction* getSubfunction(float x);

    QList<subfunction*> funcList;

    int activeSubfunction;
    const enum eFunctype type;
    section* const secParent;
private:
    float startValue;
};

#endif // FUNCTION_H
