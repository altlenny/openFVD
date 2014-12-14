#-------------------------------------------------
#
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
#
#-------------------------------------------------


# DEPENDENCIES
# 
# QT (tested with 5.2.1)
# glew (for Windows and Linux builds only)
# glm (tested with 0.9.5.1-1)
# lib3ds

CONFIG	+= qt
QT       += core gui widgets printsupport opengl

#CONFIG += exceptions \
#          rtti

TARGET = FVD
TEMPLATE = app

SOURCES += main.cpp\
    core/undohandler.cpp \
    core/undoaction.cpp \
    core/trackhandler.cpp \
    core/track.cpp \
    core/subfunction.cpp \
    core/smoothhandler.cpp \
    core/sectionhandler.cpp \
    core/section.cpp \
    core/secstraight.cpp \
    core/secgeometric.cpp \
    core/secforced.cpp \
    core/seccurved.cpp \
    core/secbezier.cpp \
    core/saver.cpp \
    core/nolimitsimporter.cpp \
    core/mnode.cpp \
    core/function.cpp \
    core/exportfuncs.cpp \
    osx/common.cpp \
    renderer/trackmesh.cpp \
    renderer/mytexture.cpp \
    renderer/myshader.cpp \
    renderer/myframebuffer.cpp \
    renderer/glviewwidget.cpp \
    ui/transitionwidget.cpp \
    ui/trackwidget.cpp \
    ui/trackproperties.cpp \
    ui/smoothui.cpp \
    ui/qcustomplot.cpp \
    ui/projectwidget.cpp \
    ui/optionsmenu.cpp \
    ui/objectexporter.cpp \
    ui/mytreewidget.cpp \
    ui/myqdoublespinbox.cpp \
    ui/mainwindow.cpp \
    ui/importui.cpp \
    ui/graphwidget.cpp \
    ui/graphhandler.cpp \
    ui/exportui.cpp \
    ui/draglabel.cpp \
    ui/conversionpanel.cpp

HEADERS  += core/undohandler.h \
    core/undoaction.h \
    core/trackhandler.h \
    core/track.h \
    core/subfunction.h \
    core/smoothhandler.h \
    core/sectionhandler.h \
    core/section.h \
    core/secstraight.h \
    core/secgeometric.h \
    core/secforced.h \
    core/seccurved.h \
    core/secbezier.h \
    core/saver.h \
    core/nolimitsimporter.h \
    core/mnode.h \
    core/function.h \
    core/exportfuncs.h \
    osx/common.h \
    renderer/trackmesh.h \
    renderer/mytexture.h \
    renderer/myshader.h \
    renderer/myframebuffer.h \
    renderer/glviewwidget.h \
    ui/transitionwidget.h \
    ui/trackwidget.h \
    ui/trackproperties.h \
    ui/smoothui.h \
    ui/qcustomplot.h \
    ui/projectwidget.h \
    ui/optionsmenu.h \
    ui/objectexporter.h \
    ui/mytreewidget.h \
    ui/myqdoublespinbox.h \
    ui/mainwindow.h \
    ui/importui.h \
    ui/graphwidget.h \
    ui/graphhandler.h \
    ui/exportui.h \
    ui/draglabel.h \
    ui/conversionpanel.h \
    lenassert.h

FORMS    += ui/transitionwidget.ui \
    ui/trackwidget.ui \
    ui/trackproperties.ui \
    ui/smoothui.ui \
    ui/projectwidget.ui \
    ui/optionsmenu.ui \
    ui/objectexporter.ui \
    ui/mainwindow.ui \
    ui/importui.ui \
    ui/graphwidget.ui \
    ui/exportui.ui \
    ui/conversionpanel.ui

win32 {
    INCLUDEPATH += "./ui/"
    INCLUDEPATH += "./renderer/"
    INCLUDEPATH += "./core/"

    INCLUDEPATH += "C:\Libraries\glew-1.11.0\include" # path-to-glew/include
    INCLUDEPATH += "C:\Libraries\glm" #path-to-glm"
    INCLUDEPATH += "C:\Libraries\lib3ds-20080909\src" #path-to-lib3ds

    RC_FILE = winicon.rc

    LIBS += -lOpenGL32
    LIBS += -lGlU32
    LIBS += "C:\Libraries\glew-1.11.0\lib\Release\Win32\glew32.lib" #path-to-glew\lib\Release\Win32\glew32.lib
    LIBS += "C:\Libraries\glew-1.11.0\bin\Release\Win32\glew32.dll" #path-to-glew\bin\Release\Win32\glew32.dll
}

unix:!macx {
    INCLUDEPATH += "./ui/"
    INCLUDEPATH += "./renderer/"
    INCLUDEPATH += "./core/"

    LIBS += -lGL
    LIBS += -lGLU
    LIBS += -lGLEW

    LIBS += -lX11
    LIBS += -L /usr/local/lib/
    LIBS += -l3ds
}

macx {
    ICON = fvd.icns
    QMAKE_INFO_PLIST = ./osx/resources/Info.plist

    INCLUDEPATH += "./ui/"
    INCLUDEPATH += "./renderer/"
    INCLUDEPATH += "./core/"
    INCLUDEPATH += "./glm/"
    INCLUDEPATH += "/usr/local/include/"

    LIBS += -framework Foundation -framework Cocoa
    LIBS += -L /usr/local/lib/
    LIBS += -l3ds

    QMAKE_CFLAGS_X86_64 += -mmacosx-version-min=10.6
    QMAKE_CXXFLAGS_X86_64 = $$QMAKE_CFLAGS_X86_64
    QMAKE_CXXFLAGS_RELEASE += -O2

    OBJECTIVE_SOURCES += \
        osx/Document.mm \
        osx/MainDelagate.mm \
        osx/NSApplicationMain.mm

    HEADERS += \
        osx/Document.h \
        osx/MainDelagate.h \
        osx/NSApplicationMain.h

    SOURCES +=  \
        osx/Init.cpp
}

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    winicon.rc \
    background.png \
    metal.png \
    shaders/normals.vert \
    shaders/track.vert \
    shaders/track.frag \
    shaders/sky.vert \
    shaders/sky.frag \
    shaders/simpleSM.vert \
    shaders/simpleSM.frag \
    shaders/shadowVolume.vert \
    shaders/shadowVolume.frag \
    shaders/oculus.vert \
    shaders/oculus.frag \
    shaders/occlusion.vert \
    shaders/occlusion.frag \
    shaders/normals.frag \
    shaders/metal.dat \
    shaders/floor.vert \
    shaders/floor.frag \
    shaders/debug.vert \
    shaders/debug.frag \
    metalnormals.png \
    readme.txt \
    sky/negx.jpg \
    sky/negy.jpg \
    sky/negz.jpg \
    sky/posx.jpg \
    sky/posy.jpg \
    sky/posz.jpg
