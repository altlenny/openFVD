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

#include "importui.h"
#include "ui_importui.h"
#include "fstream"
#include "exportfuncs.h"
#include "trackhandler.h"
#include "mainwindow.h"
#include "lenassert.h"

using namespace std;

extern MainWindow* gloParent;

importUi::importUi(QWidget *parent, QString _fileName) :
    QDialog(parent),
    ui(new Ui::importUi)
{
    ui->setupUi(this);
    fileName = _fileName;

#ifdef Q_OS_MAC
    setWindowModality(Qt::WindowModal);
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setFixedSize(430, 200);
#endif

    fstream fin(fileName.toLocal8Bit().data(), ios::in | ios::binary);
    //lenAssert(fin != NULL && "input stream NULL");
    if(!fin) {
        return;
    }

    fin.seekg (0, ios::end);
    int length = fin.tellg();
    int id = 0;

    legacymode = false;

    for(int i = 0; i < length; ++i) {
        fin.seekg(i);
        string check = readString(&fin, 3);
        if(check == "TRC") {
            posList.append(fin.tellg());
            int namelength = readInt(&fin);
            QString name = QString(readString(&fin, namelength).c_str());
            trackList.append(new trackHandler(name, id));
            ui->treeWidget->addTopLevelItem(trackList[id]->listItem);
            id++;
            i += 3+namelength;
        }

        if(check == "v0.") {
            string check = readString(&fin, 2);
            if(check == "30") {
                legacymode = true;
            }
        }
    }

    fin.close();

    ui->treeWidget->setColumnCount(3);
    ui->treeWidget->setColumnWidth(0, 30);
    ui->treeWidget->setColumnWidth(1, 200);
    ui->treeWidget->setColumnWidth(2, 30);
}

importUi::~importUi()
{
    delete ui;
}

void importUi::on_buttonBox_accepted()
{
    fstream fin(fileName.toLocal8Bit().data(), ios::in | ios::binary);
    //lenAssert(fin != NULL && "input stream NULL");
    if(!fin) {
        return;
    }

    for(int i = 0; i < trackList.size(); ++i) {
        if(ui->treeWidget->topLevelItem(0)->checkState(2) == Qt::Unchecked) {
            delete trackList[i];
            trackList.removeAt(i);
            if(posList.size() > i) {
                posList.removeAt(i);
            }
            --i;
        } else if(posList.size()) {
            fin.seekg(posList[i]);
            if(legacymode) {
                trackList[i]->trackData->legacyLoadTrack(fin, trackList[i]->trackWidgetItem);

            } else {
                trackList[i]->trackData->loadTrack(fin, trackList[i]->trackWidgetItem);
            }
            ui->treeWidget->takeTopLevelItem(0);
        }
    }

    fin.close();

    ((projectWidget*)parent())->appendTracks(trackList);
    delete this;
}

void importUi::on_buttonBox_rejected()
{
    for(int i = 0; i < trackList.size(); ++i) {
        delete trackList[i];
    }
    delete this;
}
