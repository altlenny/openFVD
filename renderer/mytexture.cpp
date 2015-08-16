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

#include "mytexture.h"
#include <QFile>

QList<bool> myTexture::usedIDs;

myTexture::myTexture(QImage &_image, int mode)
{
    mId = getFreeID();
    myTexture::usedIDs[mId] = true;
    glActiveTexture(GL_TEXTURE0 + mId);
    lenAssert(!_image.isNull());
    QImage conv = QGLWidget::convertToGLFormat(_image);
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    GLfloat fLargest;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    if(mode == 0)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else if(mode == 1)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    }

    glTexImage2D(GL_TEXTURE_2D, 0 /* MipMap level */, GL_RGBA, conv.width(),conv.height(), 0 /* no border */, GL_RGBA, GL_UNSIGNED_BYTE, conv.bits() );

    glGenerateMipmap(GL_TEXTURE_2D);

    iType = 0;
}

myTexture::myTexture(const char* _image, int mode)
{
    mId = getFreeID();
    myTexture::usedIDs[mId] = true;
    glActiveTexture(GL_TEXTURE0 + mId);

    QImage img;
    QString s(_image);
    lenAssert(img.load(s));
    QImage conv = QGLWidget::convertToGLFormat(img);
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    GLfloat fLargest;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    if(mode == 0)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else if(mode == 1)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    }

    glTexImage2D(GL_TEXTURE_2D, 0 /* MipMap level */, GL_RGBA, conv.width(),conv.height(), 0 /* no border */, GL_RGBA, GL_UNSIGNED_BYTE, conv.bits() );

    glGenerateMipmap(GL_TEXTURE_2D);

    iType = 0;
}

myTexture::myTexture(const char *_negx, const char *_negy, const char *_negz, const char *_posx, const char *_posy, const char *_posz)
{
    mId = getFreeID();
    myTexture::usedIDs[mId] = true;
    glActiveTexture(GL_TEXTURE0 + mId);
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

    QImage tex, texture;

    lenAssert(tex.load(_negz));
    texture = QGLWidget::convertToGLFormat(tex);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, texture.width(), texture.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

    lenAssert(tex.load(_posz));
    texture = QGLWidget::convertToGLFormat(tex);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, texture.width(), texture.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

    lenAssert(tex.load(_negy));
    texture = QGLWidget::convertToGLFormat(tex);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, texture.width(), texture.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

    lenAssert(tex.load(_posy));
    texture = QGLWidget::convertToGLFormat(tex);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, texture.width(), texture.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

    lenAssert(tex.load(_posx));
    texture = QGLWidget::convertToGLFormat(tex);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, texture.width(), texture.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

    lenAssert(tex.load(_negx));
    texture = QGLWidget::convertToGLFormat(tex);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, texture.width(), texture.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    iType = 2;
}

myTexture::myTexture(int _width, int _height, GLuint _format, GLuint _intFormat)
{
    mId = getFreeID();
    myTexture::usedIDs[mId] = true;
    glActiveTexture(GL_TEXTURE0 + mId);
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0 /* MipMap level */, _format, _width, _height, 0 /* no border */, _intFormat, GL_FLOAT, 0 );

    glGenerateMipmap(GL_TEXTURE_2D);

    iType = 1;
}

myTexture::~myTexture()
{
    myTexture::usedIDs[mId] = false;
    glDeleteTextures(1, &handle);
}

void myTexture::resize(int _width, int _height, GLuint _format, GLuint _intFormat)
{
    glActiveTexture(GL_TEXTURE0 + mId);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0 /* MipMap level */, _format, _width, _height, 0 /* no border */, _intFormat, GL_FLOAT, 0 );
    glGenerateMipmap(GL_TEXTURE_2D);
}

void myTexture::changeTexture(QImage &_image)
{
    glActiveTexture(GL_TEXTURE0 + mId);
    glBindTexture(GL_TEXTURE_2D, handle);
    QImage conv = QGLWidget::convertToGLFormat(_image);
    glTexImage2D(GL_TEXTURE_2D, 0 /* MipMap level */, GL_RGBA, conv.width(),conv.height(), 0 /* no border */, GL_RGBA, GL_UNSIGNED_BYTE, conv.bits() );
    glGenerateMipmap(GL_TEXTURE_2D);
}

GLuint myTexture::getId()
{
    return (GLuint)mId;
}

GLuint myTexture::getHandle()
{
    return handle;
}

int myTexture::getFreeID()
{
    for(int i = 0; i < myTexture::usedIDs.size(); ++i)
    {
        if(!myTexture::usedIDs[i]) return i;
    }
    return -1;
}

void myTexture::initialize()
{
    int maxIDs;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxIDs);

    for(int i = 0; i < maxIDs; ++i)
    {
        myTexture::usedIDs.append(false);
    }
}
