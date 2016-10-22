#ifndef BEZRVERTEX_H
#define BEZRVERTEX_H

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "bezrsegment.h"
#include "idlrlcomponent.h"

#include <QDebug>

class BezrVertex : public IDLRLComponent
{
public:
    BezrVertex(glm::vec3 _cp1, glm::vec3 _pos, glm::vec3 _cp2, double _roll);

    void setCP1(glm::vec3 _cp1);
    void setCP2(glm::vec3 _cp2);
    void setPos(glm::vec3 _pos);
    void setRoll(double _roll);
    void setBezier(glm::vec3 _cp1, glm::vec3 _pos, glm::vec3 _cp2);
    bool checkNullVectors(int i);

    void limitAndClip();

    glm::vec3 clipBezier(glm::vec3 vec1, glm::vec3 vec2, float value1, float value2, float minValue, float maxValue);

    glm::mat4 computeMatrix();

    bool isLockedHandles() const;
    void setLockedHandles(bool value);

    bool isFirstBezier() const;
    void setFirstBezier(bool value);

    bool isContinuousRoll() const;
    void setContinuousRoll(bool value);

    bool isRelativeRoll() const;
    void setRelativeRoll(bool value);

    glm::vec3 getCP1() const;
    glm::vec3 getCP2() const;
    glm::vec3 getPos() const;

    double getRoll() const;

private:
    double roll;

    glm::vec3 cp1;
    glm::vec3 pos;
    glm::vec3 cp2;

    bool firstBezier;
    bool lockedHandles;
    bool continuousRoll;
    bool relativeRoll;
};

#endif // BEZRVERTEX_H
