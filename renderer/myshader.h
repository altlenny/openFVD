#ifndef MYSHADER_H
#define MYSHADER_H

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

class myShader
{
public:
    myShader(const char* _vertex, const char* _fragment);
    ~myShader();
    void useAttribute(GLuint _index, const GLchar* _name);
    void setOutput(GLuint _index, const GLchar* _name);

    void useUniform(const GLchar* _name, glm::mat4* _mat4);
    void useUniform(const GLchar* _name, glm::vec4* _vec4);
    void useUniform(const GLchar* _name, glm::vec3* _vec3);
    void useUniform(const GLchar* _name, float f1, float f2, float f3);
    void useUniform(const GLchar* _name, GLuint _int);
    void useUniform(const GLchar* _name, float _float);

    void linkProgram();
    void bind();
private:
    GLuint sources[2];
    GLuint program;
};

#endif // MYSHADER_H
