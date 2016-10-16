#ifndef SUBFUNCTION_H
#define SUBFUNCTION_H

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

#include <fstream>
#include <QList>

class func;

enum eDegree
{
    linear = 0,
    quadratic = 1,
    cubic = 2,
    quartic = 3,
    quintic = 4,
    sinusoidal = 5,
    plateau = 6,
    tozero = 7,
    freeform = 8
};

typedef struct bez_s
{
    float x;
    float y;
} bez_t;

class subfunc
{
public:
    subfunc();
    subfunc(float min, float max, float start, float diff, func* getparent = 0);
    void update(float min, float max, float diff);

    float getValue(float x);

    void changeDegree(eDegree newDegree);
    void updateBez();

    float getMinValue();
    float getMaxValue();

    void translateValues(float newStart);

    bool isSymmetric();
    float endValue();

    void saveSubFunc(std::fstream& file);
    void loadSubFunc(std::fstream& file);
    void legacyLoadSubFunc(std::fstream& file);
    void saveSubFunc(std::stringstream& file);
    void loadSubFunc(std::stringstream& file);

    float minArgument;
    float maxArgument;

    float startValue;

    float arg1;
    float symArg;

    bool locked;

    //timewarp arguments
    float centerArg;
    float tensionArg;

    enum eDegree degree;


    func* parent;
    QList<bez_t> pointList;
    QList<float> valueList;

private:
    float applyTension(float x);
    float applyCenter(float x);
};


#endif // SUBFUNCTION_H
