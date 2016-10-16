#ifndef SECNLCSV_H
#define SECNLCSV_H

#include <QMap>
#include "track.h"
#include "section.h"

class secnlcsv : public section
{
public:
    secnlcsv(track* getParent, mnode* first);
    virtual int updateSection(int node = 0);
    virtual void saveSection(std::fstream& file);
    virtual void loadSection(std::fstream& file);
    virtual void legacyLoadSection(std::fstream& file);
    virtual void saveSection(std::stringstream& file);
    virtual void loadSection(std::stringstream& file);
    virtual float getMaxArgument();
    virtual bool isLockable(func* _func);
    virtual bool isInFunction(int index, subfunc* func);
    void loadTrack(QString filename);
    mnode getNodeAtDistance(float distance, float totalLength);
private:
    QList<float> distanceList;
    QMap<int, int> distanceMapping;
    QList<mnode> csvNodes;

    void initDistanceMapping();
    mnode lerpNode(mnode *n1, mnode *n2, float t);
};

#endif // SECNLCSV_H
