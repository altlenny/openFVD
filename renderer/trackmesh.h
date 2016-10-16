#ifndef TRACKMESH_H
#define TRACKMESH_H

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

//#include "mypanelopengl.h"
#include "glviewwidget.h"

typedef struct tracknode_s{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    float vel;
    float rollSpeed;
    float yForce;
    float xForce;
    float flexion;
    float selected;
    int node;
} tracknode_t;

typedef struct meshnode_s{
    glm::vec3 pos;
    int node;
} meshnode_t;

typedef struct pipeoption_s{
    int edges;
    glm::vec2 radius;
    glm::vec2 offset;
    bool smooth;
} pipeoption_t;

class trackMesh
{
public:
    trackMesh(track* parent = NULL);
    ~trackMesh();

    bool isInit;

    int createPipes(QVector<tracknode_t> &list, QList<pipeoption_t> &options);
    int create3dsPipes(QVector<float> *_vertices, QList<pipeoption_t> &options);
    void createIndices();

    int createPipe(QVector<tracknode_t> &list, int edges, float radiusy, float radiusx, float y, float x, bool smooth = true);
    void createBox(QVector<tracknode_t> &list, glm::vec3 P1l, glm::vec3 P2l, glm::vec3 P3l, glm::vec3 P4l, glm::vec3 P1r, glm::vec3 P2r, glm::vec3 P3r, glm::vec3 P4r);
    void createBox(QVector<meshnode_t> &list, glm::vec3 P1l, glm::vec3 P2l, glm::vec3 P3l, glm::vec3 P4l, glm::vec3 P1r, glm::vec3 P2r, glm::vec3 P3r, glm::vec3 P4r);
    int createShadowBox(QVector<meshnode_t> &list, glm::vec3 P1l, glm::vec3 P2l, glm::vec3 P3l, glm::vec3 P4l, glm::vec3 P1r, glm::vec3 P2r, glm::vec3 P3r, glm::vec3 P4r);
    void create3dsBox(QVector<float> *_vertices, QVector<unsigned int> *_indices, glm::vec3 P1l, glm::vec3 P2l, glm::vec3 P3l, glm::vec3 P4l, glm::vec3 P1r, glm::vec3 P2r, glm::vec3 P3r, glm::vec3 P4r);

    void createQuad(QVector<tracknode_t> &list, glm::vec3 P1, glm::vec3 P2, glm::vec3 P3, glm::vec3 P4);
    void createQuad(QVector<meshnode_t> &list, glm::vec3 P1, glm::vec3 P2, glm::vec3 P3, glm::vec3 P4);
    void create3dsQuad(QVector<float> *_vertices, QVector<unsigned int> *_indices, glm::vec3 P1, glm::vec3 P2, glm::vec3 P3, glm::vec3 P4);
    int createShadowTriangle(QVector<meshnode_t> &list, glm::vec3 P1, glm::vec3 P2, glm::vec3 P3);

    void createSupport(QVector<tracknode_t> &list, int edges, float radiusy, float radiusx, glm::vec3 P1, glm::vec3 P2, bool smooth);

    void buildMeshes(int fromNode);
    void build3ds(const int _sec, QVector<float> * _vertices, QVector<unsigned int> *_indices, QVector<unsigned int> *_borders);
    void updateVertexArrays();

    void appendTrackNode(QVector<tracknode_t> &list, float _u = 0.f, float _v = 0.f);
    void append3dsNode(QVector<float> *_vertices);
    void appendSupportNode(QVector<tracknode_t> &list, float _u = 0.f, float _v = 0.f);
    void appendMeshNode(QVector<meshnode_t> &list);

    void recolorTrack(void);

    QVector<tracknode_t> rails;
    QList<int> nodeList;
    QVector<int> pipeIndices, shadowIndices;
    QList<int> pipeBorders;
    QVector<tracknode_t> crossties;
    QVector<tracknode_t> rendersupports;

    QVector<meshnode_t> supports;
    QVector<meshnode_t> railshadows;
    QVector<meshnode_t> crosstieshadows;
    QVector<meshnode_t> supportshadows;
    QVector<meshnode_t> heartline;

    QList<pipeoption_t> options;

    QList<int> posList;
    QList<int> secList;

    GLuint TrackBuffer[7], TrackObject[5], TrackIndices[5];
    GLuint HeartBuffer[5], HeartObject[5], HeartIndices[5];
    GLuint ShadowBuffer[1], ShadowObject[1];

    track* trackData;

    int trackVertexSize, supportsSize, numRails, heartlineSize, railShadowSize;

    bool isWireframe;

    void init();
private:
    int j;
    int nextNode;
    glm::vec3 nextPos;
    glm::vec3 nextNorm;
    mnode* curNode;
    section* curSection;
};

#endif // TRACKMESH_H
