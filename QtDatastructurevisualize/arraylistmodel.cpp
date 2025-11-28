#include "arraylistmodel.h"
#include <QDebug>

ArrayListModel::ArrayListModel(QObject* parent) : QObject(parent) {}

void ArrayListModel::insertNode(int value) {
    qDebug() << "[ArrayModel] insert" << value;
    m_data.push_back(value);
    emit nodeInserted(value);
}

void ArrayListModel::deleteNode(int value) {
    qDebug() << "[ArrayModel] delete" << value;
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        if (*it == value) {
            m_data.erase(it);
            emit nodeDeleted(value);
            return;
        }
    }
    qDebug() << "[ArrayModel] not found" << value;
}

void ArrayListModel::clearList() {
    m_data.clear();
    emit listCleared();
}

bool ArrayListModel::contains(int value) const {
    for (int v : m_data) if (v == value) return true;
    return false;
}
