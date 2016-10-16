#ifndef BEZRVERTEX_H
#define BEZRVERTEX_H

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <track/pickobject.h>
#include <track/bezrsegment.h>
#include <track/idlrlcomponent.h>
#include <QDebug>

/*
 * Base fully implemented!
 * Missing: PickObject methods, but first, implement PickObject
 */
class BezrVertex : public PickObject, public IDLRLComponent
{
public:
    BezrVertex(glm::vec3 _cp1, glm::vec3 _pos, glm::vec3 _cp2, double _roll);

    QString getPickClassName() { return "BezrVertex"; }

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

    void debug() {
        qDebug() << "first " << firstBezier;
        qDebug() << "pos.x" << pos.x << "pos.y" << pos.y << "pos.z" << pos.z;
        qDebug() << "cp1.x" << cp1.x << "cp1.y" << cp1.y << "cp1.z" << cp1.z;
        qDebug() << "cp2.x" << cp2.x << "cp2.y" << cp2.y << "cp2.z" << cp2.z;
        qDebug() << "roll " << roll;
        qDebug() << "------------------------------";
    }

private:
    // see class IDLRLComponent *prev; // 0x4 (ebx + 0x4)
    // see class IDLRLComponent *next; // 0x8 (ebx + 0x8)

    double roll; // 0x18 (ebx + 0x18)

    glm::vec3 cp1; // 0x20 (ebx + 0x20, ebx + 0x24, ebx + 0x28)
    glm::vec3 pos; // 0x2c (ebx + 0x2c, ebx + 0x30, ebx + 0x34)
    glm::vec3 cp2; // 0x38 (ebx + 0x38, ebx + 0x3C, ebx + 0x40)

    bool firstBezier; // 0x44 (ebx + 0x44)
    bool lockedHandles; // 0x45 (ebx + 0x45)
    bool continuousRoll; // 0x46 (ebx + 0x46)
    bool relativeRoll;  // 0x47 (ebx + 0x47)
};

#endif // BEZRVERTEX_H
