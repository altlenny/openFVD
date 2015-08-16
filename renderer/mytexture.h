#ifndef MYTEXTURE_H
#define MYTEXTURE_H

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

#include "glviewwidget.h"

class myTexture
{
public:
    myTexture(QImage& _image, int mode = 0);
    myTexture(const char* _image, int mode = 0);
    myTexture(const char *_negx, const char *_negy, const char *_negz, const char *_posx, const char *_posy, const char *_posz);
    myTexture(int _width, int _height, GLuint _format, GLuint _intFormat);
    ~myTexture();
    GLuint getId();
    GLuint getHandle();
    void resize(int _width, int _height, GLuint _format, GLuint _intFormat);
    void changeTexture(QImage& _image);
    static void initialize();
private:
    int getFreeID();
    int iType;
    GLuint handle;
    int mId;
    static QList<bool> usedIDs;
};

#endif // MYTEXTURE_H
