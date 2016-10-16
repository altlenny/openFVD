#include "bezrsegment.h"
#include <QDebug>
#include "bezrvertex.h"
#include <core/core.h>
#include <math.h>

BezrSegment::BezrSegment() : IDLRLComponent() {
    nodes.clear();
    segmentLength = 0.0f;
    segmentLengthNormalized = 0.0f;
}

void BezrSegment::updateMatrices(BezrVertex *v0, BezrVertex *v1) {
    Node *firstNode = nodes.first();

    glm::vec3 firstNodePosition(firstNode->matrix[0][3], firstNode->matrix[1][3], firstNode->matrix[2][3]);
    glm::vec3 v0normal = v0->getCP2() - v0->getCP1();

    firstNode->xAngle = 0.0f;
    firstNode->yAngle = 0.0f;
    firstNode->zAngle = 0.0f;
    firstNode->matrix = v0->computeMatrix();
    firstNode->roll = sin(atan2(v0normal.y, glm::length(glm::vec2(v0normal.xz()))));

    if(prev) {
        if(prev->getPrev()) {
            BezrSegment *prevSegment = dynamic_cast<BezrSegment*>(prev->getPrev()); // var_114;
            if(prevSegment && v0->isRelativeRoll()) {
                Node *lastNode = prevSegment->getNodeAt(prevSegment->getNodesSize() - 1);
                glm::mat4 lastNodeMatrix = lastNode->matrix;
                NL::AzRotR(lastNodeMatrix, prevSegment->getRollOffset());
                firstNode->matrix = lastNodeMatrix;
            }
        }
    }

    glm::mat4 prevNodeMatrix = firstNode->matrix;
    prevNodeMatrix[0][3] = 0.0f;
    prevNodeMatrix[1][3] = 0.0f;
    prevNodeMatrix[2][3] = 0.0f;

    glm::vec3 prevNodePosition = firstNodePosition;

    if(nodes.size() > 1) {
        Node *prevNode = nodes.at(1);
        Node *node;

        int i = 1;
        do {
            glm::vec3 nodePosition;

            if(i == nodes.size() - 1) {
                prevNodePosition = v1->getCP1();
                nodePosition = v1->getCP2();
                i++;
            } else {
                i++;
                node = nodes.at(i);
                nodePosition = glm::vec3(node->matrix[0][3], node->matrix[1][3], node->matrix[2][3]);
            }

            glm::vec3 nodesNormal = nodePosition - prevNodePosition;
            prevNode->roll = sin(atan2(nodesNormal.y, glm::length(glm::vec2(nodesNormal.xz()))));

            glm::mat4 newMatrix = prevNodeMatrix;
            newMatrix[3][3] = 1.0f;

            std::swap(newMatrix[0][1], newMatrix[1][0]);
            std::swap(newMatrix[0][2], newMatrix[2][0]);
            std::swap(newMatrix[1][2], newMatrix[2][1]);

            nodesNormal = glm::vec3(glm::vec4(nodesNormal, 1) * newMatrix);

            prevNode->xAngle = atan2(nodesNormal.y, glm::length(glm::vec2(nodesNormal.xz())));
            prevNode->yAngle = atan2(nodesNormal.x * -1, nodesNormal.z * -1);
            prevNode->zAngle = 0.0f;

            NL::AyRotR(prevNodeMatrix, prevNode->yAngle);
            NL::AxRotR(prevNodeMatrix, prevNode->xAngle);

            prevNodePosition = glm::vec3(prevNode->matrix[0][3], prevNode->matrix[1][3], prevNode->matrix[2][3]);
            prevNode->matrix = prevNodeMatrix;
            prevNode->matrix[0][3] = prevNodePosition.x;
            prevNode->matrix[1][3] = prevNodePosition.y;
            prevNode->matrix[2][3] = prevNodePosition.z;

            if(nodes.size() <= i)
                break;

            prevNode = node;
        } while(true);
    }

    if(v1->isRelativeRoll() && !v1->isFirstBezier()) {
        rollOffset = v1->getRoll();
    } else {
        rollOffset = v1->getRoll() - atan2(nodes.last()->matrix[1][0], nodes.last()->matrix[1][1]);

        if (fabsf(rollOffset) > glm::radians(270.0f)) {
            if (rollOffset > 0.0) rollOffset -= glm::radians(360.0f);
            else rollOffset += glm::radians(360.0f);
        }
    }
}

void BezrSegment::notify() {
    BezrVertex *prevVertex = 0, *nextVertex = 0;

    if(prev) prevVertex = dynamic_cast<BezrVertex*>(prev);
    if(next) nextVertex = dynamic_cast<BezrVertex*>(next);

    segmentLength = 0.0f;
    segmentLengthNormalized = 0.0f;

    if(prevVertex && nextVertex) {
        float length = estimateLength();
        int flatLengthEstimated = length * 20.0f;

        if(flatLengthEstimated > 256)
            flatLengthEstimated = 256;

        nodes.clear();
        glm::vec3 lastPos = prevVertex->getPos();

        if(flatLengthEstimated > 0) {
            for(int i=0; i < flatLengthEstimated; i++) {
                float t = (float) i / (float) (flatLengthEstimated - 1);

                glm::vec3 pos = bezierFast(prevVertex->getPos(), prevVertex->getCP2(), nextVertex->getCP1(), nextVertex->getPos(), t);

                float distance = glm::distance(pos, lastPos);
                segmentLength += distance;
                lastPos = pos;

                Node *item = new Node();
                item->distance = segmentLength;

                // implement: use matrix::translate()
                item->matrix[0][3] = pos.x;
                item->matrix[1][3] = pos.y;
                item->matrix[2][3] = pos.z;

                nodes.append(item);
            }
        }

        if(segmentLength > 0.0f)
            segmentLengthNormalized = 1 / segmentLength;

        // implement: use matrix::translate()
        nodes.first()->matrix[0][3] = prevVertex->getPos().x;
        nodes.first()->matrix[1][3] = prevVertex->getPos().y;
        nodes.first()->matrix[2][3] = prevVertex->getPos().z;

        nodes.last()->matrix[0][3] = nextVertex->getPos().x;
        nodes.last()->matrix[1][3] = nextVertex->getPos().y;
        nodes.last()->matrix[2][3] = nextVertex->getPos().z;

        updateMatrices(prevVertex, nextVertex);

        if(nextVertex->isRelativeRoll() && !nextVertex->isFirstBezier()) {
            if(nextVertex->getNext()) {
                BezrSegment *nextSegment = dynamic_cast<BezrSegment*>(nextVertex->getNext());
                if(nextSegment && nextSegment != this) {
                    nextSegment->notify();
                }
            }
        }
    }
}

glm::vec3 BezrSegment::bezierFast(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t) {
    float t1 = 1.0f-t;
    float b0 = t1 * t1 * t1;
    float b1 = 3 * t1 * t1 * t;
    float b2 = 3 * t1 * t * t;
    float b3 = t * t * t;

    return b0 * p0  + b1 * p1 + b2 * p2 + b3 * p3;
}

double BezrSegment::estimateLength() {
    BezrVertex *prevVertex = 0, *nextVertex = 0;

    if(prev) prevVertex = dynamic_cast<BezrVertex*>(prev);
    if(next) nextVertex = dynamic_cast<BezrVertex*>(next);

    if(!prev || !next) return 0.0;

    glm::vec3 lastPos = prevVertex->getPos();
    float length = 0.0f;

    for(int i=1; i < 15; i+= 2) {
        float t1 = (float) (i) / 14.0f;
        float t2 = (float) (i+1) / 14.0f;

        glm::vec3 pos1 = BezrSegment::bezierFast(prevVertex->getPos(), prevVertex->getCP2(), nextVertex->getCP1(), nextVertex->getPos(), t1);
        glm::vec3 pos2 = BezrSegment::bezierFast(prevVertex->getPos(), prevVertex->getCP2(), nextVertex->getCP1(), nextVertex->getPos(), t2);

        length += glm::distance(pos1, lastPos) + glm::distance(pos1, pos2);

        lastPos = pos2;
    };

    return length;
}

void BezrSegment::interpolateRoll(bool bPrevAvail, float fPrevLength, double fPrevOffset, bool bNextAvail, float fNextLength, double fNextOffset) {
    BezrVertex *prevBez = 0;
    BezrVertex *nextBez = 0;

    if(prev) prevBez = dynamic_cast<BezrVertex*>(prev);
    if(next) nextBez = dynamic_cast<BezrVertex*>(next);

    double fOffset = getRollOffset();

    // Hermite derivatives
    float m0 = 0.0f, m1 = 0.0f;

    // Hermite end-points
    float p0 = 0.0f;
    float p1 = fOffset;

    if (bPrevAvail && prevBez->isContinuousRoll()) {
      m0 = (fPrevLength * fPrevOffset + getSegmentLength() * fOffset) / (fPrevLength + getSegmentLength());
    }

    if (bNextAvail && nextBez->isContinuousRoll()) {
      m1 = (fNextLength * fNextOffset + getSegmentLength() * fOffset) / (fNextLength + getSegmentLength());
    }

    // convert Hermite to Bezier
    double b00 = p0;
    double b01 = p0 + m0 / 3.0;
    double b02 = p1 - m1 / 3.0;
    double b03 = p1;

    if(nodes.size() - 1 > 1) {
        for(int i=1; i < nodes.size() - 1; i++) {
            float t =  (float)i / (nodes.size() - 1);
            float t1 = 1.0f - t;
            float b0 = t1 * t1 * t1;
            float b1 = 3 * t1 * t1 * t;
            float b2 = 3 * t1 * t * t;
            float b3 = t * t * t;

            nodes.at(i)->zAngle = b0 * b00  + b1 * b01 + b2 * b02 + b3 * b03;
        }
    }

    nodes.last()->zAngle = fOffset;
}

glm::mat4 BezrSegment::getMatrixAtDist(float distance, float &roll, float &distanceInPercent) {
    int left = 0;
    int right = nodes.size() - 1;
    float range;
    int pos;

    if(!nodes.size()) return glm::mat4();

    while(distance >= nodes.at(left)->distance && distance <= nodes.at(right)->distance) {
        range = nodes.at(right)->distance - nodes.at(left)->distance;

        pos = left + (int)(((double)right - left) * (distance - nodes.at(left)->distance) / range);

        if(distance > nodes.at(pos)->distance) left = pos + 1;
        else if(distance < nodes.at(pos)->distance) right = pos - 1;
        else break;
    }

    distanceInPercent = distance * segmentLengthNormalized;

    Node *currentNode = nodes.at(pos); // ebx
    Node *nextNode = nodes.at(pos + 1); // esi

    float t = 0;
    float nodesDistanceDiff = nextNode->distance - currentNode->distance;
    float distanceDiff = distance - currentNode->distance;

    if (nodesDistanceDiff > 0.001f) {
        float ratio = distanceDiff / nodesDistanceDiff;

        if (ratio < 0.0f) {
            if (ratio != ratio) t = 0.0f;
            else t = fmin(1.0f, ratio);
        }
        else t = fmin(1.0f, ratio);
    } else t = 0.5f;

    glm::vec3 newPos(
        (nextNode->matrix[0][3] - currentNode->matrix[0][3]) * t,
        (nextNode->matrix[1][3] - currentNode->matrix[1][3]) * t,
        (nextNode->matrix[2][3] - currentNode->matrix[2][3]) * t
    );

    glm::mat4 resMat = currentNode->matrix;

    resMat[0][3] += newPos.x;
    resMat[1][3] += newPos.y;
    resMat[2][3] += newPos.z;

    NL::AyRotR(resMat, nextNode->yAngle * t);
    NL::AxRotR(resMat, nextNode->xAngle * t);
    NL::AzRotR(resMat, currentNode->zAngle + ((nextNode->zAngle - currentNode->zAngle) * t));

    roll = currentNode->roll + ((nextNode->roll - currentNode->roll) * t);
    return resMat;
}


float BezrSegment::getSegmentLengthNormalized() const {
    return segmentLengthNormalized;
}

int BezrSegment::getNodesSize() {
    return nodes.size();
}

Node *BezrSegment::getNodeAt(int i) {
    return nodes.at(i);
}

float BezrSegment::getSegmentLength() const {
    return segmentLength;
}

double BezrSegment::getRollOffset() const {
    return rollOffset;
}
