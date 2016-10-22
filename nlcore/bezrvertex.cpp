#include "bezrvertex.h"

#include <QDebug>
#include "nl.h"
#include <limits>

BezrVertex::BezrVertex(glm::vec3 _cp1, glm::vec3 _pos, glm::vec3 _cp2, double _roll) {
    cp1 = _cp1;
    pos = _pos;
    cp2 = _cp2;
    roll = _roll;

    firstBezier = false;
    lockedHandles = false;
    continuousRoll = false;
    relativeRoll = false;

    prev = 0;
    next = 0;

    checkNullVectors(0);
    limitAndClip();
}

void BezrVertex::setCP1(glm::vec3 _cp1) {
    cp1 = _cp1;
    checkNullVectors(1);

    glm::vec3 ncp2 = cp2 - pos;
    glm::vec3 ncp1 = cp1 - pos;
    float ncp2Len = glm::length(ncp2);

    if(!lockedHandles) {
        float ncp1Len = glm::length(ncp1);
        ncp1 = (ncp1 * ncp2Len) / ncp1Len;
    }

    cp2 = pos - ncp1;
    limitAndClip();

    if(prev != 0)
        prev->notify();

    if(next != 0)
        next->notify();
}

void BezrVertex::setCP2(glm::vec3 _cp2) {
    cp2 = _cp2;
    checkNullVectors(2);

    glm::vec3 ncp1 = cp1 - pos;
    glm::vec3 ncp2 = cp2 - pos;
    float ncp1Len = glm::length(ncp1);

    if(!lockedHandles) {
        float ncp2Len = glm::length(ncp2);
        ncp2 = (ncp2 * ncp1Len) / ncp2Len;
    }

    cp1 = pos - ncp2;
    limitAndClip();

    if(prev != 0)
        prev->notify();

    if(next != 0)
        next->notify();
}

void BezrVertex::setPos(glm::vec3 _pos) {
    cp1 = cp1 - pos + _pos;
    cp2 = cp2 - pos + _pos;
    pos = _pos;

    limitAndClip();

    if(prev != 0)
        prev->notify();

    if(next != 0)
        next->notify();
}

void BezrVertex::setBezier(glm::vec3 _cp1, glm::vec3 _pos, glm::vec3 _cp2) {
    cp1 = _cp1;
    cp2 = _cp2;
    pos = _pos;

    limitAndClip();

    if(prev != 0)
        prev->notify();

    if(next != 0)
        next->notify();
}

bool BezrVertex::checkNullVectors(int i) {
    if(i == 2) {
        float x;

        glm::vec3 ncp2 = cp2 - pos;
        float ncp2len = glm::length(ncp2);

        if(NL_SPACE_MIN_VECTOR_LENGTH > ncp2len) {
            if(ncp2len < std::numeric_limits<double>::epsilon()) {
                ncp2.x += 0.001f;
                ncp2len = glm::length(ncp2);
            }

            ncp2 = (ncp2 / ncp2len) * NL_SPACE_MIN_VECTOR_LENGTH;
            x = cp2.x - ncp2.x;

            glm::vec3 newpos = cp2 - ncp2;
            pos = newpos;

            glm::vec3 ncp1 = cp1 - glm::vec3(x, pos.y, pos.z);
            float ncp1len = glm::length(ncp1);

            glm::vec3 newcp2 = glm::vec3(
                cp2.x - x,
                cp2.y - pos.y,
                cp2.z - pos.z
            );

            if(!lockedHandles) {
                float len = glm::length(newcp2);
                newcp2 = (newcp2 * ncp1len) / len;
            }

            cp1 = glm::vec3(
                x - newcp2.x,
                pos.y - newcp2.y,
                pos.z - newcp2.z
            );
        }

        glm::vec3 ncp1 = cp1 - pos;
        float ncp1len = glm::length(ncp1);

        if(NL_SPACE_MIN_VECTOR_LENGTH > ncp1len) {
            if(ncp1len < std::numeric_limits<double>::epsilon()) {
                ncp1.x += 0.001f;
                ncp1len = glm::length(ncp1);
            }

            glm::vec3 cp1backup = cp1;
            cp1 = (ncp1 / ncp1len) * NL_SPACE_MIN_VECTOR_LENGTH;

            glm::vec3 ncp1old = cp1backup - pos;
            float ncp1oldlen = glm::length(ncp1old);
            glm::vec3 ncp2 = glm::vec3(x, cp2.y, cp2.z) - pos;

            if(!lockedHandles) {
                float len = glm::length(ncp2);
                ncp2 = (ncp2 * ncp1oldlen) / len;
            }

            cp1 = pos - ncp2;
        }
    }
    else if(i == 1) {
        float x;

        glm::vec3 ncp1 = cp1 - pos;
        float ncp1len = glm::length(ncp1);

        if(NL_SPACE_MIN_VECTOR_LENGTH > ncp1len) {
            if(ncp1len < std::numeric_limits<double>::epsilon()) {
                ncp1.x += 0.001f;
                ncp1len = glm::length(ncp1);
            }

            ncp1 = (ncp1 / ncp1len) * NL_SPACE_MIN_VECTOR_LENGTH;
            x = cp1.x - ncp1.x;

            glm::vec3 newpos = cp1 - ncp1;
            pos = newpos;

            glm::vec3 ncp2 = cp2 - glm::vec3(x, pos.y, pos.z);
            float ncp2len = glm::length(ncp2);

            glm::vec3 newcp1 = glm::vec3(
                cp1.x - x,
                cp1.y - pos.y,
                cp1.z - pos.z
            );

            if(!lockedHandles) {
                float len = glm::length(newcp1);
                newcp1 = (newcp1 * ncp2len) / len;
            }

            cp2 = glm::vec3(
                x - newcp1.x,
                pos.y - newcp1.y,
                pos.z - newcp1.z
            );
        }

        glm::vec3 ncp2 = cp2 - pos;
        float ncp2len = glm::length(ncp2);

        if(NL_SPACE_MIN_VECTOR_LENGTH > ncp2len) {
            if(ncp2len < std::numeric_limits<double>::epsilon()) {
                ncp2.x += 0.001f;
                ncp2len = glm::length(ncp2);
            }

            glm::vec3 cp2backup = cp2;
            cp2 = (ncp2 / ncp2len) * NL_SPACE_MIN_VECTOR_LENGTH;

            glm::vec3 ncp2old = cp2backup - pos;
            float ncp2oldlen = glm::length(ncp2old);
            glm::vec3 ncp1 = glm::vec3(x, cp1.y, cp1.z) - pos;

            if(!lockedHandles) {
                float len = glm::length(ncp1);
                ncp1 = (ncp1 * ncp2oldlen) / len;
            }

            cp2 = pos - ncp1;
        }
    } else {
        glm::vec3 ncp1 = cp1 - pos;
        float ncp1len = glm::length(ncp1);

        if (NL_SPACE_MIN_VECTOR_LENGTH > ncp1len) {
            if(ncp1len <= std::numeric_limits<double>::epsilon()) {
                ncp1.x += 0.001f;
                ncp1len = glm::length(ncp1);
            }

            ncp1 = ((ncp1 / ncp1len) * NL_SPACE_MIN_VECTOR_LENGTH) + pos;
            cp1 = ncp1;

            glm::vec3 ncp2 = cp2 - pos;
            float ncp2len = glm::length(ncp2);

            ncp1 = ncp1 - pos;

            if(!lockedHandles) {
                float len = glm::length(ncp1);
                ncp1 = (ncp1 * ncp2len) / len;
            }

            cp2 = pos - ncp1;
        }

        glm::vec3 ncp2 = cp2 - pos;
        float ncp2len = glm::length(ncp2);

        if (NL_SPACE_MIN_VECTOR_LENGTH > ncp2len) {
            if(ncp2len <= std::numeric_limits<double>::epsilon()) {
                ncp2.x += 0.001f;
                ncp2len = glm::length(ncp2);
            }

            ncp2 = ((ncp2 / ncp2len) * NL_SPACE_MIN_VECTOR_LENGTH) + pos;
            cp2 = ncp2;

            glm::vec3 ncp1 = cp1 - pos;
            float ncp1len = glm::length(ncp1);

            ncp2 = ncp2 - pos;

            if(!lockedHandles) {
                float len = glm::length(ncp2);
                ncp2 = (ncp2 * ncp1len) / len;
            }

            cp1 = pos - ncp2;
        }
    }
}

glm::mat4 BezrVertex::computeMatrix() {
    glm::vec3 cpDiff = cp2 - cp1;

    glm::mat4 mat;

    NL::computeInitialMatrix(mat, cpDiff);
    NL::AzRotR(mat, roll);

    mat[0][3] = pos.x;
    mat[1][3] = pos.y;
    mat[2][3] = pos.z;

    return mat;
}

void BezrVertex::limitAndClip() {
    glm::vec3 ncp1 = cp1 - pos;
    glm::vec3 ncp2 = cp2 - pos;

    NL::limitInSpace(pos, NL_SPACE_MIN_VECTOR_LENGTH);

    cp1 = ncp1 + pos;
    cp2 = ncp2 + pos;

    cp1 = clipBezier(cp1, pos, cp1.x, pos.x, NL_SPACE_MIN_XZ, NL_SPACE_MAX_XZ);
    cp2 = clipBezier(cp2, pos, cp2.x, pos.x, NL_SPACE_MIN_XZ, NL_SPACE_MAX_XZ);

    cp1 = clipBezier(cp1, pos, cp1.y, pos.y, NL_SPACE_MIN_Y, NL_SPACE_MAX_Y);
    cp2 = clipBezier(cp2, pos, cp2.y, pos.y, NL_SPACE_MIN_Y, NL_SPACE_MAX_Y);

    cp1 = clipBezier(cp1, pos, cp1.z, pos.z, NL_SPACE_MIN_XZ, NL_SPACE_MAX_XZ);
    cp2 = clipBezier(cp2, pos, cp2.z, pos.z, NL_SPACE_MIN_XZ, NL_SPACE_MAX_XZ);
}

glm::vec3 BezrVertex::clipBezier(glm::vec3 vec1, glm::vec3 vec2, float value1, float value2, float minValue, float maxValue) {
    if(minValue > value1) {
        float diff = minValue - value2;
        float invertedDiff = (diff * -1) / ((minValue - value1) - diff);
        vec1 = ((vec1 - vec2) * invertedDiff) + vec2;
    }

    if(value1 > maxValue) {
        float diff = maxValue - value2;
        float invertedDiff = (diff * -1) / ((maxValue - value1) - diff);
        vec1 = (invertedDiff * (vec1 - vec2)) + vec2;
    }

    return vec1;
}

void BezrVertex::setRoll(double _roll) {
    roll = _roll;

    if(prev != 0)
        prev->notify();

    if(next != 0)
        next->notify();
}

bool BezrVertex::isLockedHandles() const {
    return lockedHandles;
}

void BezrVertex::setLockedHandles(bool value) {
    lockedHandles = value;
}

bool BezrVertex::isFirstBezier() const {
    return firstBezier;
}

void BezrVertex::setFirstBezier(bool value) {
    firstBezier = value;
}

bool BezrVertex::isContinuousRoll() const {
    return continuousRoll;
}

void BezrVertex::setContinuousRoll(bool value) {
    continuousRoll = value;
}

bool BezrVertex::isRelativeRoll() const {
    return relativeRoll;
}

void BezrVertex::setRelativeRoll(bool value) {
    relativeRoll = value;
}

glm::vec3 BezrVertex::getCP1() const {
    return cp1;
}

glm::vec3 BezrVertex::getCP2() const {
    return cp2;
}

glm::vec3 BezrVertex::getPos() const {
    return pos;
}

double BezrVertex::getRoll() const {
    return roll;
}
