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

#include "myshader.h"
#include "shaders.h"
#include <QFile>

myShader::myShader(const char* _vertex, const char* _fragment)
{
    QFile vertex(_vertex);
    lenAssert(vertex.open(QIODevice::ReadOnly));
    QString v = vertex.readAll();
    QFile fragment(_fragment);
    lenAssert(fragment.open(QIODevice::ReadOnly));
    QString f = fragment.readAll();
    sources[0] = glCreateShader(GL_VERTEX_SHADER);
    sources[1] = glCreateShader(GL_FRAGMENT_SHADER);
    program = glCreateProgram();

#ifdef Q_OS_MAC
    v.replace("#version 130", "#version 150 core");
    f.replace("#version 130", "#version 150 core");
#endif

    char* v1 = new char[v.size()+1];
    strcpy(v1, v.toLocal8Bit().data());
    char* v2 = new char[f.size()+1];
    strcpy(v2, f.toLocal8Bit().data());

    const char* strings[2] = {v1, v2};

    glShaderSource(sources[0], 1, &strings[0], 0);
    glShaderSource(sources[1], 1, &strings[1], 0);

    delete v1;
    delete v2;

    glCompileShader(sources[0]);
    printGLSLCompileLog(sources[0]);
    glCompileShader(sources[1]);
    printGLSLCompileLog(sources[1]);
}

myShader::~myShader()
{

}

void myShader::useAttribute(GLuint _index, const GLchar* _name)
{
    glBindAttribLocation(program, _index, _name);
}

void myShader::setOutput(GLuint _index, const GLchar* _name)
{
    glBindFragDataLocation(program, _index, _name);
}

void myShader::useUniform(const GLchar* _name, glm::mat4* _mat4)
{
    glUniformMatrix4fv(glGetUniformLocation(program, _name), 1, GL_FALSE, glm::value_ptr(*_mat4));
}

void myShader::useUniform(const GLchar* _name, glm::vec4* _vec4)
{
    glUniform4f(glGetUniformLocation(program, _name), _vec4->x, _vec4->y, _vec4->z, _vec4->w);
}

void myShader::useUniform(const GLchar* _name, glm::vec3* _vec3)
{
    glUniform3f(glGetUniformLocation(program, _name), _vec3->x, _vec3->y, _vec3->z);
}

void myShader::useUniform(const GLchar* _name, float f1, float f2, float f3)
{
    glUniform3f(glGetUniformLocation(program, _name), f1, f2, f3);
}

void myShader::useUniform(const GLchar* _name, GLuint _int)
{
    glUniform1i(glGetUniformLocation(program, _name), _int);
}

void myShader::useUniform(const GLchar* _name, float _float)
{
    glUniform1f(glGetUniformLocation(program, _name), _float);
}

void myShader::linkProgram()
{
    glAttachShader(program, sources[0]);
    glAttachShader(program, sources[1]);

    glLinkProgram(program);
    printGLSLLinkLog(program);
}

void myShader::bind()
{
    glUseProgram(program);
}
