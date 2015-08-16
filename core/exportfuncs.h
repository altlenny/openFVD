#ifndef EXPORTFUNCS_H
#define EXPORTFUNCS_H

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
#include <sstream>
#include "mnode.h"

void writeBytes(std::fstream *file, const char* data, size_t length );

void writeNulls(std::fstream *file , size_t length );

std::string readString(std::fstream *file, size_t length);

bool readNulls(std::fstream *file, size_t length);

glm::vec3 readVec3(std::fstream *file);

float readFloat(std::fstream *file);

int readInt(std::fstream *file);

bool readBool(std::fstream *file);

void readBytes(std::fstream *file, void* _ptr, size_t length);


void writeBytes(std::stringstream *file, const char* data, size_t length );

void writeNulls(std::stringstream *file , size_t length );

std::string readString(std::stringstream *file, size_t length);

bool readNulls(std::stringstream *file, size_t length);

glm::vec3 readVec3(std::stringstream *file);

float readFloat(std::stringstream *file);

int readInt(std::stringstream *file);

bool readBool(std::stringstream *file);

void readBytes(std::stringstream *file, void* _ptr, size_t length);

void writeToExportFile(std::fstream *file, QList<bezier_t*> &bezList);


#endif // EXPORTFUNCS_H
