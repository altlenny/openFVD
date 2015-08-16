#ifndef MYFRAMEBUFFER_H
#define MYFRAMEBUFFER_H

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

class myTexture;

class myFramebuffer
{
public:
    myFramebuffer(int _width, int _height, GLuint _format, GLuint _intFormat, bool _useRenderbuffer = false);
    void resize(int _width, int _height);
    ~myFramebuffer();
    GLuint getHandle();
    GLuint getTexture();
    void bind();
    void clear();
    void unbind();
    void setClearColor(float f1, float f2, float f3);
private:
    void printFBStatus();
    myTexture* target;
    GLuint handle;
    GLuint renderbuffer;
    int width;
    int height;
    GLuint format;
    GLuint intFormat;
    glm::vec3 clearColor;
};

#endif // MYFRAMEBUFFER_H
