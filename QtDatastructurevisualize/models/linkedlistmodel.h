#pragma once
#include <QObject>

struct ListNode {
    int value;
    ListNode* next;
    explicit ListNode(int v) : value(v), next(nullptr) {}
};

class LinkedListModel : public QObject {
    Q_OBJECT
public:
    explicit LinkedListModel(QObject* parent = nullptr);
    ~LinkedListModel() override;
    void insertNode(int value); // tail insert
    void deleteNode(int value);
    void clearList();
    bool contains(int value) const;
signals:
    void nodeInserted(int value);
    void nodeDeleted(int value);
    void listCleared();
private:
    ListNode* head = nullptr;
};
