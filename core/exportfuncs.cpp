
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

#include "exportfuncs.h"

using namespace std;

void writeBytes(fstream *file, const char* data, size_t length )
{
    for(size_t i = 0; i < length; i++) {
        file->write(data+length-1-i, 1);
    }
}

void writeNulls(fstream *file , size_t length )
{
    char data[1] = {0x00};
    for(size_t i = 0; i < length; i++) {
        writeBytes(file, data, 1);
    }
}

string readString(fstream *file, size_t length)
{
    string temp = "";
    char c;
    for(size_t i = 0; i < length; ++i) {
        file->get(c);
        temp.append(1, c);
    }
    return temp;
}

bool readNulls(fstream *file, size_t length)
{
    char c;
    for(size_t i = 0; i < length; ++i) {
        file->get(c);
    }
    return true;
}

glm::vec3 readVec3(fstream *file)
{
    glm::vec3 temp(readFloat(file), readFloat(file), readFloat(file));
    return temp;
}

float readFloat(fstream *file)
{
    union {
        char c[4];
        float f;
    } temp;
    file->get(temp.c[3]);
    file->get(temp.c[2]);
    file->get(temp.c[1]);
    file->get(temp.c[0]);
    return temp.f;
}

int readInt(fstream *file)
{
    union {
        char c[4];
        int i;
    } temp;
    file->get(temp.c[3]);
    file->get(temp.c[2]);
    file->get(temp.c[1]);
    file->get(temp.c[0]);
    return temp.i;
}

bool readBool(fstream *file)
{
    char temp;
    file->get(temp);
    return temp != 0;
}

void readBytes(fstream *file, void* _ptr, size_t length)
{
    for(size_t i = 0; i < length; i++) {
        file->read((char*)_ptr+length-1-i, 1);
    }
}


void writeBytes(stringstream *file, const char* data, size_t length )
{
    for(size_t i = 0; i < length; i++) {
        file->write(data+length-1-i, 1);
    }
}

void writeNulls(stringstream *file , size_t length )
{
    char data[1] = {0x00};
    for(size_t i = 0; i < length; i++) {
        writeBytes(file, data, 1);
    }
}

string readString(stringstream *file, size_t length)
{
    string temp = "";
    char c;
    for(size_t i = 0; i < length; ++i) {
        file->get(c);
        temp.append(1, c);
    }
    return temp;
}

bool readNulls(stringstream *file, size_t length)
{
    char c;
    for(size_t i = 0; i < length; ++i) {
        file->get(c);
        //if(c) return false;
    }
    return true;
}

glm::vec3 readVec3(stringstream *file)
{
    glm::vec3 temp(readFloat(file), readFloat(file), readFloat(file));
    return temp;
}

float readFloat(stringstream *file)
{
    union {
        char c[4];
        float f;
    } temp;
    file->get(temp.c[3]);
    file->get(temp.c[2]);
    file->get(temp.c[1]);
    file->get(temp.c[0]);
    return temp.f;
}

int readInt(stringstream *file)
{
    union {
        char c[4];
        int i;
    } temp;
    file->get(temp.c[3]);
    file->get(temp.c[2]);
    file->get(temp.c[1]);
    file->get(temp.c[0]);
    return temp.i;
}

bool readBool(stringstream *file)
{
    char temp;
    file->get(temp);
    return temp != 0;
}

void readBytes(stringstream *file, void* _ptr, size_t length)
{
    for(size_t i = 0; i < length; i++) {
        file->read((char*)_ptr+length-1-i, 1);
    }
}

void writeToExportFile(std::fstream *file, QList<bezier_t*> &bezList)
{
    for(int i = 0; i < bezList.size(); ++i) {
        writeBytes(file, (const char*)&bezList[i]->Kp1.x, 4);
        writeBytes(file, (const char*)&bezList[i]->Kp1.y, 4);
        writeBytes(file, (const char*)&bezList[i]->Kp1.z, 4);

        writeBytes(file, (const char*)&bezList[i]->Kp2.x, 4);
        writeBytes(file, (const char*)&bezList[i]->Kp2.y, 4);
        writeBytes(file, (const char*)&bezList[i]->Kp2.z, 4);

        writeBytes(file, (const char*)&bezList[i]->P1.x, 4);
        writeBytes(file, (const char*)&bezList[i]->P1.y, 4);
        writeBytes(file, (const char*)&bezList[i]->P1.z, 4);

        writeBytes(file, (const char*)&bezList[i]->roll, 4);

        char cTemp = 0xFF;
        writeBytes(file, &cTemp, 1); // CONT ROLL
        cTemp = bezList[i]->relRoll ? 0xFF : 0x00;
        writeBytes(file, &cTemp, 1); // REL ROLL
        cTemp = 0x00;
        writeBytes(file, &cTemp, 1); // equal dist CP
        writeNulls(file, 7); // were 5
    }
}
