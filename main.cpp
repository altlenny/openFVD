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

#include <QApplication>
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include "mainwindow.h"
#include "lenassert.h"

QApplication* application;

#ifdef Q_OS_MAC
#include "osx/NSApplicationMain.h"
#endif

void myMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    FILE* log = fopen("fvd.log", "a");

    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        printf("%s\n", localMsg.constData());
        fprintf(log, "%s\n", localMsg.constData());
        //printf("Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        //fprintf(log, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        printf("%s\n", localMsg.constData());
        fprintf(log, "%s\n", localMsg.constData());
        //printf("Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        //fprintf(log, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        printf("%s\n", localMsg.constData());
        fprintf(log, "%s\n", localMsg.constData());
        //printf("Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        //fprintf(log, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        printf("%s\n", localMsg.constData());
        fprintf(log, "%s\n", localMsg.constData());
        //printf("Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        //fprintf(log, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
    fflush(stdout);
    fclose(log);
}

int main(int argc, char *argv[])
{
    application = new QApplication(argc, argv);
#ifndef Q_OS_MAC
    qInstallMessageHandler(myMessageHandler);

    FILE* log = fopen("fvd.log", "w");
    fprintf(log, "FVD++ v0.77 Logfile\n");
    fclose(log);
#endif

#ifdef Q_OS_MAC
    QGLFormat fmt;
    fmt.setProfile(QGLFormat::CoreProfile);
    fmt.setVersion(3,2);
    fmt.setSampleBuffers(true);
    fmt.setSamples(4);
    QGLFormat::setDefaultFormat(fmt);
#endif

    MainWindow w;
    w.show();
    if(argc == 2) {
        QString fileName(argv[1]);
        if(fileName.endsWith(".fvd")) {
            qDebug("starting FVD++ with project %s", argv[1]);
            w.loadProject(argv[1]);
        }
    }


#ifdef Q_OS_MAC
    return OwnNSApplicationMain(argc, (const char **)argv);
#else
    return application->exec();
#endif
}
