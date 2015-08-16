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

#include "myframebuffer.h"
#include "mytexture.h"

myFramebuffer::myFramebuffer(int _width, int _height, GLuint _format, GLuint _intFormat, bool _useRenderbuffer)
{
    width = _width;
    height = _height;
    format = _format;
    intFormat = _intFormat;
    clearColor = glm::vec3(1, 1, 1);

    glGenFramebuffers(1, &handle);
    glBindFramebuffer(GL_FRAMEBUFFER, handle);

    target = new myTexture(_width, _height, format, intFormat);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target->getHandle(), 0);

    if(_useRenderbuffer)
    {
        glGenRenderbuffers(1, &renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
    }

    GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, attachments);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printFBStatus();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

myFramebuffer::~myFramebuffer()
{
    glDeleteFramebuffers(1, &handle);
    delete target;
    glDeleteRenderbuffers(1, &renderbuffer);
}

void myFramebuffer::resize(int _width, int _height)
{
    width = _width;
    height = _height;

    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    target->resize(width, height, format, intFormat);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target->getHandle(), 0);

    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

    GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, attachments);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printFBStatus();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint myFramebuffer::getHandle()
{
    return handle;
}

GLuint myFramebuffer::getTexture()
{
    return target->getId();
}

void myFramebuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

void myFramebuffer::clear()
{
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void myFramebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void myFramebuffer::printFBStatus()
{
    GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status)
    {
    case GL_FRAMEBUFFER_UNDEFINED:
        qWarning("Target is the default framebuffer, but the default framebuffer does not exist.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        qWarning("Any of the framebuffer attachment points are framebuffer incomplete.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        qWarning("The framebuffer does not have at least one image attached to it.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        qWarning("The value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment point(s) named by GL_DRAW_BUFFERi.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        qWarning("GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for the color attachment point named by GL_READ_BUFFER.");
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        qWarning("The combination of internal formats of the attached images violates an implementation-dependent set of restrictions.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        qWarning("The value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES is the not same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES does not match the value of GL_TEXTURE_SAMPLES.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        qWarning("is returned if any framebuffer attachment is layered, and any populated attachment is not layered, or if all populated color attachments are not from textures of the same target.");
        break;
    }
}

void myFramebuffer::setClearColor(float f1, float f2, float f3)
{
    clearColor = glm::vec3(f1, f2, f3);
}
