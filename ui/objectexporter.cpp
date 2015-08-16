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

#include "objectexporter.h"
#include "ui_objectexporter.h"

#include <lib3ds.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <QFileDialog>

#include "mainwindow.h"
#include "trackmesh.h"

extern MainWindow* gloParent;

objectExporter::objectExporter(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::objectExporter)
{
    ui->setupUi(this);
    curTrackIndex = -1;
}

objectExporter::~objectExporter()
{
    delete ui;
}

bool objectExporter::update()
{
    ui->trackBox->clear();
    QList<trackHandler*> list = gloParent->getTrackList();
    for(int i = 0; i < list.size(); ++i) {
        ui->trackBox->addItem(list[i]->trackData->name);
    }
    if(curTrackIndex > list.size()-1) {
        if(list.size() > 0) {
            curTrackIndex = 0;
            ui->trackBox->setCurrentIndex(curTrackIndex);
        } else {
            curTrackIndex = -1;
        }
        return false;
    } else {
        if(curTrackIndex == -1 && list.size() > 0) {
            curTrackIndex = 0;
        }
        ui->trackBox->setCurrentIndex(curTrackIndex);
        return true;
    }
}

// TODO: Build own exporter class
void objectExporter::on_buttonBox_accepted()
{
    QString fileName = QFileDialog::getSaveFileName(gloParent, "Save 3ds Object", ".", "3D Object (*.3ds)", 0, 0);

    QList<trackHandler*> trackList = gloParent->getTrackList();
    trackHandler* curTrack = trackList[ui->trackBox->currentIndex()];

    trackMesh* mesh = curTrack->mMesh;

    QVector<float> *vertices = new QVector<float>();
    QVector<unsigned int> *indices = new QVector<unsigned int>();
    QVector<unsigned int> *borders = new QVector<unsigned int>();

    Lib3dsFile *file = lib3ds_file_new();
    file->frames = 360;

    {
        Lib3dsMaterial* mat = lib3ds_material_new("coaster");
        lib3ds_file_insert_material(file, mat, -1);
        mat->diffuse[0] = curTrack->trackColors[0].red()/255.f;
        mat->diffuse[1] = curTrack->trackColors[0].green()/255.f;
        mat->diffuse[2] = curTrack->trackColors[0].blue()/255.f;
    }

    {
        for(int section = 0; section < curTrack->trackData->lSections.size(); ++section) {
            vertices->clear();
            indices->clear();
            borders->clear();
            mesh->build3ds(section, vertices, indices, borders);

            float* fvertices = new float[vertices->size()];
            for(int i = 0; i < vertices->size()/3; ++i) {
                fvertices[3*i+0] = vertices->at(3*i+0);
                fvertices[3*i+1] = -vertices->at(3*i+2);
                fvertices[3*i+2] = vertices->at(3*i+1);

                //exportScreen->doFastExport();
            }
            for(int subIndex = 0; subIndex < borders->size()-2; subIndex+= 2) {
                int fromVIndex = borders->at(subIndex)/3;
                int toVIndex = borders->at(subIndex+2)/3;
                int fromIIndex = borders->at(subIndex+1)/3;
                int toIIndex = borders->at(subIndex+3)/3;

                int i, j;
                QString name = QString::number(section).append(QString("_").append(QString::number(subIndex/2)));
                Lib3dsMesh *mesh = lib3ds_mesh_new(name.toLocal8Bit().data());
                Lib3dsMeshInstanceNode *inst;
                lib3ds_file_insert_mesh(file, mesh, -1);

                lib3ds_mesh_resize_vertices(mesh, toVIndex-fromVIndex, 1, 0);
                for (i = 0; i < toVIndex-fromVIndex; ++i) {
                    lib3ds_vector_copy(mesh->vertices[i], &fvertices[(i+fromVIndex)*3]);
                    mesh->texcos[i][0] = 0.f;
                    mesh->texcos[i][1] = 0.f;
                }

                lib3ds_mesh_resize_faces(mesh, toIIndex-fromIIndex);
                for (i = 0; i < toIIndex-fromIIndex; ++i) {
                    for (j = 0; j < 3; ++j) {
                        mesh->faces[i].index[j] = indices->at(3*(i+fromIIndex)+j)-fromVIndex;
                        mesh->faces[i].material = 0;
                    }
                }
                inst = lib3ds_node_new_mesh_instance(mesh, name.toLocal8Bit().data(), NULL, NULL, NULL);
                lib3ds_file_append_node(file, (Lib3dsNode*)inst, NULL);
            }
            delete[] fvertices;
        }
    }

    {
        Lib3dsCamera *camera;
        Lib3dsCameraNode *n;
        Lib3dsTargetNode *t;
        int i;

        camera = lib3ds_camera_new("camera01");
        lib3ds_file_insert_camera(file, camera, -1);
        lib3ds_vector_make(camera->position, 0.0, -100, 0.0);
        lib3ds_vector_make(camera->target, 0.0, 0.0, 0.0);

        n = lib3ds_node_new_camera(camera);
        t = lib3ds_node_new_camera_target(camera);
        lib3ds_file_append_node(file, (Lib3dsNode*)n, NULL);
        lib3ds_file_append_node(file, (Lib3dsNode*)t, NULL);

        lib3ds_track_resize(&n->pos_track, 37);
        for (i = 0; i <= 36; i++) {
            n->pos_track.keys[i].frame = 10 * i;
            lib3ds_vector_make(n->pos_track.keys[i].value,
                (float)(100.0 * cos(2 * F_PI * i / 36.0)),
                (float)(100.0 * sin(2 * F_PI * i / 36.0)),
                50.0
            );
        }
    }

    if (!lib3ds_file_save(file, fileName.toLocal8Bit().data())) {
         qDebug("ERROR: Saving 3ds file failed!\n");
    }
    lib3ds_file_free(file);

    delete indices;
    delete vertices;
}
