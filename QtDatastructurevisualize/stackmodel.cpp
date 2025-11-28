#include "stackmodel.h"
#include <QDebug>

StackModel::StackModel(QObject* parent) : QObject(parent) {}

void StackModel::push(int v) {
    m_stack.push_back(v);
    emit nodePushed(v);
    qDebug() << "[Stack] push" << v;
}

int StackModel::pop() {
    if (m_stack.empty()) return INT_MIN;
    int v = m_stack.back();
    m_stack.pop_back();
    emit nodePopped(v);
    qDebug() << "[Stack] pop" << v;
    return v;
}

void StackModel::popValue(int value) {
    // find from top; remove first encountered
    for (int i = (int)m_stack.size() - 1; i >= 0; --i) {
        if (m_stack[i] == value) {
            m_stack.erase(m_stack.begin() + i);
            emit nodePopped(value);
            qDebug() << "[Stack] popValue" << value;
            return;
        }
    }
    qDebug() << "[Stack] popValue not found" << value;
}

void StackModel::clearStack() {
    m_stack.clear();
    emit stackCleared();
}

bool StackModel::contains(int v) const {
    for (int x : m_stack) if (x == v) return true;
    return false;
}
