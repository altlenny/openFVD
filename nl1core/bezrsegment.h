#ifndef BEZRSEGMENT_H
#define BEZRSEGMENT_H

#include <core/nl.h>
#include <QVector>

#include "idlrlcomponent.h"

class BezrVertex;

class Node {
public:
    // angles are maybe a vector3f
    float xAngle;           // 0x0
    float yAngle;           // 0x4
    float zAngle;           // 0x8

    float distance;         // 0x10

    // figure out which value is this, its an angle, but which one
    float roll;             // 0xc

    glm::mat4 matrix;       // 0x14 (pos: 0x14 + (0x30 (x: 0x30, y: 0x34, z: 0x38)))
};

class BezrSegment : public IDLRLComponent
{
public:
    BezrSegment();

    void notify();
    glm::vec3 bezierFast(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t);
    double estimateLength();
    void updateMatrices(BezrVertex *v0, BezrVertex *v1);

    float getSegmentLength() const;
    float getSegmentLengthNormalized() const;

    int getNodesSize();
    Node *getNodeAt(int i);

    void interpolateRoll(bool bPrevAvail, float fPrevLength, double fPrevOffset, bool bNextAvail, float fNextLength, double fNextOffset);
    glm::mat4 getMatrixAtDist(float distance, float &roll, float &distanceInPercent);

    double getRollOffset() const;

private:
    double rollOffset; // 0x18
    float segmentLength; // 0x20
    float segmentLengthNormalized; // 0x24

    // size: 0xc (not needed, we are using a vector)
    // buffersize: 0x10 (0xc + 0x4) (not needed, we are using qvector)
    QVector <Node*> nodes; // 0x14 (0xc + 0x8)
};

#endif // BEZRSEGMENT_H
