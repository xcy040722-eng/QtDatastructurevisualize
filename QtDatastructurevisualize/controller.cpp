#include "controller.h"
#include "basescene.h"
#include "arraylistmodel.h"
#include "stackmodel.h"
#include "linkedlistmodel.h"
#include <QDebug>

Controller::Controller(BaseScene* scene, QObject* parent)
    : QObject(parent), m_scene(scene) {
    ensureModels();
    // bind models -> scene
    if (m_linkedModel) {
        connect(m_linkedModel, &LinkedListModel::nodeInserted, m_scene, &BaseScene::onNodeInserted);
        connect(m_linkedModel, &LinkedListModel::nodeDeleted, m_scene, &BaseScene::onNodeDeleted);
        connect(m_linkedModel, &LinkedListModel::listCleared, m_scene, &BaseScene::onListCleared);
    }
    if (m_arrayModel) {
        connect(m_arrayModel, &ArrayListModel::nodeInserted, m_scene, &BaseScene::onNodeInserted);
        connect(m_arrayModel, &ArrayListModel::nodeDeleted, m_scene, &BaseScene::onNodeDeleted);
        connect(m_arrayModel, &ArrayListModel::listCleared, m_scene, &BaseScene::onListCleared);
    }
    if (m_stackModel) {
        connect(m_stackModel, &StackModel::nodePushed, m_scene, &BaseScene::onNodeInserted);
        connect(m_stackModel, &StackModel::nodePopped, m_scene, &BaseScene::onNodeDeleted);
        connect(m_stackModel, &StackModel::stackCleared, m_scene, &BaseScene::onListCleared);
    }
}

void Controller::ensureModels() {
    if (!m_arrayModel) m_arrayModel = new ArrayListModel(this);
    if (!m_linkedModel) m_linkedModel = new LinkedListModel(this);
    if (!m_stackModel) m_stackModel = new StackModel(this);
}

int Controller::toIntOrWarn(const QString& s) {
    bool ok = false;
    int v = s.toInt(&ok);
    if (!ok) { qDebug() << "请输入整数"; return INT_MIN; }
    return v;
}

void Controller::onInsertRequested(const QString& value) {
    int v = toIntOrWarn(value); if (v == INT_MIN) return;
    ensureModels();
    switch (m_struct) {
    case LINKED: m_linkedModel->insertNode(v); break;
    case ARRAY: m_arrayModel->insertNode(v); break;
    case STACK: m_stackModel->push(v); break;
    }
}

void Controller::onRemoveRequested(const QString& value) {
    int v = toIntOrWarn(value); if (v == INT_MIN) return;
    switch (m_struct) {
    case LINKED: m_linkedModel->deleteNode(v); break;
    case ARRAY: m_arrayModel->deleteNode(v); break;
    case STACK: m_stackModel->popValue(v); break;
    }
}

void Controller::onFindRequested(const QString& value) {
    int v = toIntOrWarn(value); if (v == INT_MIN) return;
    bool found = false;
    switch (m_struct) {
    case LINKED: found = m_linkedModel->contains(v); break;
    case ARRAY: found = m_arrayModel->contains(v); break;
    case STACK: found = m_stackModel->contains(v); break;
    }
    qDebug() << (found ? "Found:" : "Not found:") << v;
}

void Controller::onResetRequested() {
    if (m_linkedModel) m_linkedModel->clearList();
    if (m_arrayModel) m_arrayModel->clearList();
    if (m_stackModel) m_stackModel->clearStack();
}

void Controller::onStructureChanged(int idx) {
    if (idx < 0 || idx>2) return;
    m_struct = static_cast<StructType>(idx);
    // let scene know to render different layout style
    if (m_scene) m_scene->setStructureType(idx);
}
