#ifndef BEZRSEGMENT_H
#define BEZRSEGMENT_H

#include "nl.h"
#include <QVector>

#include "idlrlcomponent.h"

class BezrVertex;

class Node {
public:
    float xAngle;
    float yAngle;
    float zAngle;

    float distance;
    float roll;

    glm::mat4 matrix;
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
    double rollOffset;
    float segmentLength;
    float segmentLengthNormalized;

    QVector <Node*> nodes;
};

#endif // BEZRSEGMENT_H
