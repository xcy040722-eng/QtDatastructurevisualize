#include "linkedlistmodel.h"
#include <QDebug>

LinkedListModel::LinkedListModel(QObject* parent) : QObject(parent), head(nullptr) {}
LinkedListModel::~LinkedListModel() { clearList(); }

void LinkedListModel::insertNode(int value) {
    qDebug() << "[LinkedModel] insert" << value;
    ListNode* node = new ListNode(value);
    if (!head) head = node;
    else {
        ListNode* p = head;
        while (p->next) p = p->next;
        p->next = node;
    }
    emit nodeInserted(value);
}

void LinkedListModel::deleteNode(int value) {
    qDebug() << "[LinkedModel] delete" << value;
    if (!head) return;
    if (head->value == value) {
        ListNode* tmp = head;
        head = head->next;
        delete tmp;
        emit nodeDeleted(value);
        return;
    }
    ListNode* p = head;
    while (p->next && p->next->value != value) p = p->next;
    if (p->next) {
        ListNode* tmp = p->next;
        p->next = p->next->next;
        delete tmp;
        emit nodeDeleted(value);
    }
    else {
        qDebug() << "[LinkedModel] not found" << value;
    }
}

void LinkedListModel::clearList() {
    qDebug() << "[LinkedModel] clear";
    while (head) {
        ListNode* tmp = head;
        head = head->next;
        delete tmp;
    }
    emit listCleared();
}

bool LinkedListModel::contains(int value) const {
    ListNode* p = head;
    while (p) { if (p->value == value) return true; p = p->next; }
    return false;
}
