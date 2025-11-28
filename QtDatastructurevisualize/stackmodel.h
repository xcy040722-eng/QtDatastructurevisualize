#pragma once
#include <QObject>
#include <vector>

class StackModel : public QObject {
    Q_OBJECT
public:
    explicit StackModel(QObject* parent = nullptr);
    void push(int v);
    int pop(); // returns popped value or INT_MIN if empty
    void popValue(int value); // remove specific value if exists (for compatibility)
    void clearStack();
    bool contains(int v) const;
signals:
    void nodePushed(int value);
    void nodePopped(int value);
    void stackCleared();
private:
    std::vector<int> m_stack;
};
