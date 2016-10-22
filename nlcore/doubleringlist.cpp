#include "doubleringlist.h"

#include "bezrvertex.h"

DoubleRingList::DoubleRingList() {
    connected = false;
}

void DoubleRingList::insertAt(IDLRLComponent *component, int i) {
    insertAt(component, i, QString::number(list.size()));
}

IDLRLComponent *DoubleRingList::at(int i) {
    return list.at(i);
}

void DoubleRingList::insertAt(IDLRLComponent *component, int i, QString debugName) {
    component->debugName = debugName;

    bool wasConnected = isConnected();

    if(isConnected())
        disconnect();

    if(i >= list.size()) {
        i = list.size();
        list.append(component);
    } else if(i < 0) {
        i = 0;
        list.insert(0, component);
    } else {
        list.insert(i, component);
    }

    IDLRLComponent *next = 0;
    IDLRLComponent *prev = 0;

    bool hasNext = true;
    bool hasPrev = true;

    if(i - 1 < 0) hasPrev = false;
    if(i + 1 >= list.size()) hasNext = false;

    if(hasNext) next = list.at(i + 1);
    if(hasPrev) prev = list.at(i - 1);

    if(next) {
        component->setNext(next);
        next->setPrev(component);
    } else component->setNext(0);

    if(prev) {
        component->setPrev(prev);
        prev->setNext(component);
    } else component->setPrev(0);

    if(component->getPrev())
        component->getPrev()->notify();

    component->notify();

    if(component->getNext())
        component->getNext()->notify();

    if(wasConnected)
        connect();
}

void DoubleRingList::removeAll(bool notify) {
    if (isConnected())
        disconnect();

    if (list.size() > 0) {
        do {
            removeAt(list.size() - 1, notify);
        } while (list.size() > 0);
    }
}

void DoubleRingList::removeAt(int i, bool notify) {
    if(i < 0) i = 0;
    if(i >= list.size()) i = list.size() - 1;

    IDLRLComponent *current = list.at(i);
    IDLRLComponent *prevOfCurrent = current->getPrev();
    IDLRLComponent *nextOfCurrent = current->getNext();

    if(prevOfCurrent) prevOfCurrent->setNext(nextOfCurrent);
    if(nextOfCurrent) nextOfCurrent->setPrev(prevOfCurrent);
    if(prevOfCurrent) prevOfCurrent->notify();
    if(nextOfCurrent) nextOfCurrent->notify();
    if(notify) current->notify();

    list.removeAt(i);
}

void DoubleRingList::setAnchor(int i) {
    if(!isConnected()) return;

    if(i < 0) i = 0;
    if(i >= list.size()) i = list.size() - 1;

    IDLRLComponent *anchor = list.at(i);
    list.clear();

    QVector<IDLRLComponent *> tmpList;
    tmpList.append(anchor);

    IDLRLComponent *current = anchor;
    IDLRLComponent *next = 0;

    while (next = current->getNext()) {
        if(next == anchor) break;
        tmpList.append(next);
        current = next;
    }

    connected = false;

    for(int i=0; i < tmpList.size(); i++)
        insertAt(tmpList.at(i), i, tmpList.at(i)->debugName);

    connect();
}

void DoubleRingList::connect() {
    if(!list.size()) return;

    IDLRLComponent *first = list.at(0);
    IDLRLComponent *last = list.at(list.size() - 1);

    last->setNext(first);
    first->setPrev(last);

    first->notify();
    last->notify();

    connected = true;
}

void DoubleRingList::disconnect() {
    if(!list.size()) return;

    IDLRLComponent *first = list.at(0);
    IDLRLComponent *last = list.at(list.size() - 1);

    first->setPrev(0);
    last->setNext(0);

    first->notify();
    last->notify();

    connected = false;
}

bool DoubleRingList::isConnected() {
    return connected;
}

int DoubleRingList::size() {
    return list.size();
}
