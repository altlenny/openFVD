#ifndef DOUBLERINGLIST_H
#define DOUBLERINGLIST_H

#include <QVector>

class IDLRLComponent;

class DoubleRingList
{
public:
    DoubleRingList();
    bool isConnected();
    void connect();
    void disconnect();
    void removeAt(int i, bool notify);
    void removeAll(bool notify);
    void setAnchor(int i);
    int size();

    void debug();
    void insertAt(IDLRLComponent *component, int i, QString debugName);
    void insertAt(IDLRLComponent *component, int i);

    IDLRLComponent *at(int i);
private:
    bool connected; // 0x4

    //int size 0x8
    QVector<IDLRLComponent *> list; // 0x10
};

#endif // DOUBLERINGLIST_H
