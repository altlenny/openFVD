#ifndef SECGEOMETRIC_H
#define SECGEOMETRIC_H

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

#include "track.h"
#include "section.h"

class secgeometric : public section
{
public:
    ~secgeometric();
    secgeometric(track* getParent, mnode* first, float getlength = 10.0);
    virtual int updateSection(int node = 0);
    int updateDistanceSection(int node = 0);
    virtual void saveSection(std::fstream& file);
    virtual void loadSection(std::fstream& file);
    virtual void legacyLoadSection(std::fstream& file);
    virtual void saveSection(std::stringstream& file);
    virtual void loadSection(std::stringstream& file);
    virtual float getMaxArgument();
    virtual bool isLockable(func* _func);
    virtual bool isInFunction(int index, subfunc* func);
};

#endif // SECGEOMETRIC_H
