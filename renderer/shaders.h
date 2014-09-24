#ifndef SHADERS_H
#define SHADERS_H

#include "ui/mainwindow.h"

void printGLSLCompileLog( GLuint shaderHandle ) {
    GLint shaderError;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &shaderError);
    if (shaderError != GL_TRUE)
    {
        // yes, errors
        qCritical() << "Shader compile error: ";
    }

    // a log gets always printed (could be warnings)
    GLsizei length = 0;
    glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &length);
    if (length > 1)
    {
        // a compile log can get produced even if there were no errors, but warnings!
        GLchar* pInfo = new char[length + 1];
        glGetShaderInfoLog(shaderHandle,  length, &length, pInfo);
        qDebug() << "Compile log: " << pInfo;
        delete[] pInfo;
    }
}

void printGLSLLinkLog( GLuint progHandle ) {
    // check for program link errors:
    GLint programError;
    glGetProgramiv(progHandle, GL_LINK_STATUS, &programError);

    if (programError != GL_TRUE)
    {
        // yes, errors :-(
        qCritical() << "Program could not get linked:";
    }

    GLsizei length = 0;
    glGetProgramiv(progHandle, GL_INFO_LOG_LENGTH, &length);
    if (length > 1)
    {
        // error log or warnings:
        GLchar* pInfo = new char[length + 1];
        glGetProgramInfoLog(progHandle,  length, &length, pInfo);
        qDebug() << "Linker log: " << pInfo;
        delete[] pInfo;
    }
}

#endif // SHADERS_H
