#include "secnlcsv.h"
#include "exportfuncs.h"
#include <fstream>
#include <QFile>
#include <QDebug>

using namespace std;

secnlcsv::secnlcsv(track* getParent, mnode* first) : section(getParent, nolimitscsv, first) {}

int secnlcsv::updateSection(int node)
{
    Q_UNUSED(node);

    initDistanceMapping();

    while(lNodes.size() > 1) {
        lNodes.removeLast();
    }

    lNodes[0].updateNorm();

    if(!csvNodes.size())
        return 0;

    float velocity = parent->anchorNode->fVel;
    float nodeDist = velocity / F_HZ;

    int numNode = 0;

    float trackLength = length;
    length = 0.0f;

    int totalNumOfNodes = floor(trackLength / nodeDist);
    nodeDist = trackLength / totalNumOfNodes;

    for(int i=0; i <= totalNumOfNodes; i++) {
        mnode node = getNodeAtDistance(i * nodeDist, trackLength);

        if(numNode) {
            lNodes.append(lNodes.back());
        }

        mnode *currentNode = &lNodes[numNode];
        currentNode->vPos = node.vPos;
        currentNode->vDir = node.vDir;
        currentNode->vLat = node.vLat;
        currentNode->fVel = velocity;
        currentNode->fDistFromLast = 0.0f;

        currentNode->updateNorm();
        currentNode->fRoll = glm::degrees(atan2(currentNode->vLat.y, -currentNode->vNorm.y));
        currentNode->fRollSpeed = 0.0f;

        if(numNode) {
            mnode *lastNode = &lNodes[numNode - 1];

            currentNode->fRollSpeed = (currentNode->fRoll - lastNode->fRoll) * F_HZ;

            currentNode->fHeartDistFromLast = glm::distance(currentNode->vPos, lastNode->vPos);
            currentNode->fTotalHeartLength += currentNode->fHeartDistFromLast;

            currentNode->fDistFromLast = glm::distance(currentNode->vPosHeart(parent->fHeart), lastNode->vPosHeart(parent->fHeart));
            currentNode->fTotalLength += currentNode->fDistFromLast;

            calcDirFromLast(numNode);
        }

        this->length += currentNode->fDistFromLast;

        numNode++;
    }
}

void secnlcsv::initDistanceMapping() {
    length = 0.0f;

    distanceMapping.clear();
    distanceList.clear();

    glm::vec3 lastPos;

    for(int i=0; i < csvNodes.size(); i++) {
        glm::vec3 pos = csvNodes[i].vPos;

        if(i) {
            length += glm::distance(lastPos, pos);
        }

        distanceList.append(length);

        int key = length;
        if(!distanceMapping.contains(length)) {
            distanceMapping[key] = i;
        }

        lastPos = pos;
    }
}

mnode secnlcsv::lerpNode(mnode *n1, mnode *n2, float t) {
    mnode n3;

    n3.vPos = (1.0f - t) * n1->vPos + t * n2->vPos;
    n3.vDir = (1.0f - t) * n1->vDir + t * n2->vDir;
    n3.vLat = (1.0f - t) * n1->vLat + t * n2->vLat;

    return n3;
}

mnode secnlcsv::getNodeAtDistance(float distance, float totalLength) {
    if(distance >= totalLength)
        return csvNodes.last();

    if(distance < 0.0f)
        return csvNodes.first();

    int startIndex = 0, endIndex = 0;

    if(distanceMapping.contains((int) distance)) {
        int index = distanceMapping[(int) distance];
        if(distance >= distanceList[index]) {
            for(int i=index; i < csvNodes.size(); i++) {
                if(distanceList[i] > distance) {
                    startIndex = i - 1;
                    endIndex = i;
                    break; // found one index before
                }
            }
        } else {
            for(int i=index; i >= 0; i--) {
                if(distanceList[i] < distance) {
                    startIndex = i;
                    endIndex = i + 1;
                    break; // found one index
                }
            }
        }

        float t = (1.0f * (distance - distanceList[startIndex])) / (distanceList[endIndex] - distanceList[startIndex]);
        return lerpNode(&csvNodes[startIndex], &csvNodes[endIndex], t);
    } else {
        return mnode();
    }
}

void secnlcsv::saveSection(fstream &file)
{
    int size = csvNodes.size();

    file << "CSV";

    writeBytes(&file, (const char*)&size, sizeof(int));

    for(int i=0; i < csvNodes.size(); i++) {
        writeBytes(&file, (const char*)&csvNodes[i].vPos.x, sizeof(float));
        writeBytes(&file, (const char*)&csvNodes[i].vPos.y, sizeof(float));
        writeBytes(&file, (const char*)&csvNodes[i].vPos.z, sizeof(float));

        writeBytes(&file, (const char*)&csvNodes[i].vDir.x, sizeof(float));
        writeBytes(&file, (const char*)&csvNodes[i].vDir.y, sizeof(float));
        writeBytes(&file, (const char*)&csvNodes[i].vDir.z, sizeof(float));

        writeBytes(&file, (const char*)&csvNodes[i].vLat.x, sizeof(float));
        writeBytes(&file, (const char*)&csvNodes[i].vLat.y, sizeof(float));
        writeBytes(&file, (const char*)&csvNodes[i].vLat.z, sizeof(float));
    }
}

void secnlcsv::loadSection(fstream &file)
{
    csvNodes.clear();

    int size = readInt(&file);

    for(int i=0; i < size; i++) {
        mnode node;
        node.vPos = readVec3(&file);
        node.vDir = readVec3(&file);
        node.vLat = readVec3(&file);

        csvNodes.append(node);
    }
}

void secnlcsv::legacyLoadSection(fstream &file)
{
    csvNodes.clear();

    int size = readInt(&file);

    for(int i=0; i < size; i++) {
        mnode node;
        node.vPos = readVec3(&file);
        node.vDir = readVec3(&file);
        node.vLat = readVec3(&file);

        csvNodes.append(node);
    }
}

void secnlcsv::saveSection(stringstream &file)
{
    int size = csvNodes.size();

    file << "CSV";

    writeBytes(&file, (const char*)&size, sizeof(int));

    for(int i=0; i < csvNodes.size(); i++) {
        writeBytes(&file, (const char*)&csvNodes[i].vPos.x, sizeof(float));
        writeBytes(&file, (const char*)&csvNodes[i].vPos.y, sizeof(float));
        writeBytes(&file, (const char*)&csvNodes[i].vPos.z, sizeof(float));

        writeBytes(&file, (const char*)&csvNodes[i].vDir.x, sizeof(float));
        writeBytes(&file, (const char*)&csvNodes[i].vDir.y, sizeof(float));
        writeBytes(&file, (const char*)&csvNodes[i].vDir.z, sizeof(float));

        writeBytes(&file, (const char*)&csvNodes[i].vLat.x, sizeof(float));
        writeBytes(&file, (const char*)&csvNodes[i].vLat.y, sizeof(float));
        writeBytes(&file, (const char*)&csvNodes[i].vLat.z, sizeof(float));
    }
}

void secnlcsv::loadSection(stringstream &file)
{
    csvNodes.clear();

    int size = readInt(&file);

    for(int i=0; i < size; i++) {
        mnode node;
        node.vPos = readVec3(&file);
        node.vDir = readVec3(&file);
        node.vLat = readVec3(&file);

        csvNodes.append(node);
    }
}

float secnlcsv::getMaxArgument()
{
    return 0.f;
}

bool secnlcsv::isLockable(func* _func)
{
    Q_UNUSED(_func);
    return false;
}

bool secnlcsv::isInFunction(int index, subfunc* func)
{
    Q_UNUSED(index);
    Q_UNUSED(func);
    return false;
}

void secnlcsv::loadTrack(QString filename)
{
    QFile file(filename);

    csvNodes.clear();

    if (!file.open(QIODevice::ReadOnly)) {
        return ;
    }

    int lineCount=0;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();

        if(lineCount) {
            QList<QByteArray> lineSplitted = line.split('\t');

            glm::vec3 pos = glm::vec3(lineSplitted[1].toFloat(), lineSplitted[2].toFloat(), lineSplitted[3].toFloat());
            glm::vec3 front = glm::vec3(lineSplitted[4].toFloat(), lineSplitted[5].toFloat(), lineSplitted[6].toFloat());
            glm::vec3 left = -glm::vec3(lineSplitted[7].toFloat(), lineSplitted[8].toFloat(), lineSplitted[9].toFloat());

            mnode node;
            node.vPos = pos;
            node.vDir = front;
            node.vLat = left;
            csvNodes.append(node);
        }

        lineCount++;
    }

    parent->updateTrack(0, 0);
}
