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

    void insertAt(IDLRLComponent *component, int i, QString debugName);
    void insertAt(IDLRLComponent *component, int i);

    IDLRLComponent *at(int i);
private:
    bool connected;

    QVector<IDLRLComponent *> list;
};

#endif // DOUBLERINGLIST_H
