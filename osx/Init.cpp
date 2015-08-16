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

#include <QString>
#include <mainwindow.h>

#include <QMessageBox>
#include <importui.h>
#include <QApplication>

extern MainWindow* gloParent;
extern QApplication* application;

extern "C" {
void initApplication();
void receiveFileOpen(const char *filename);
}

void initApplication() {
    application->exec();
}

void receiveFileOpen(const char *filename) {
    QString currentFileName = gloParent->getCurrentFileName();

    if(!currentFileName.length()) gloParent->on_actionLoad_triggered(QString(filename));
    else {
        QMessageBox mb(gloParent);
        mb.setWindowTitle(gloParent->tr("Loading Project"));
        mb.setText("Another project is already loaded. Do you want to add the project in addition?");
        mb.setIcon(QMessageBox::Warning);
        mb.setStandardButtons(QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes);
        mb.setDefaultButton(QMessageBox::Yes);
#ifdef Q_OS_MAC
        mb.setWindowModality(Qt::WindowModal);
        mb.setWindowFlags((mb.windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#endif
        int result = mb.exec();

        if(result == QMessageBox::No) gloParent->on_actionLoad_triggered(QString(filename));
        else if(result == QMessageBox::Yes) {
            gloParent->addProject(QString(filename));
        }
    }
}
