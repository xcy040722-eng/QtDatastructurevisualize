#include "controller.h"
#include "basescene.h"
#include "linearlistscene.h"
#include "controlpanel.h"
#include <QDebug>

Controller::Controller(BaseScene* scene, ControlPanel* panel, QObject* parent)
    : QObject(parent), m_scene(scene), m_panel(panel)
{
    connect(m_panel, &ControlPanel::insertRequested, this, &Controller::onInsertRequested);
    connect(m_panel, &ControlPanel::removeRequested, this, &Controller::onRemoveRequested);
    connect(m_panel, &ControlPanel::findRequested, this, &Controller::onFindRequested);
    connect(m_panel, &ControlPanel::resetRequested, this, &Controller::onResetRequested);
    connect(m_panel, &ControlPanel::structureChanged, this, &Controller::onStructureChanged);

    if (m_scene) {
        connect(m_scene, &BaseScene::animationFinished, this, &Controller::onAnimationFinished);
    }
    else {
        qDebug() << "CRITICAL ERROR: Scene is null in Controller constructor!";
    }
}

void Controller::lockUI() {
    m_isAnimating = true;
    m_panel->setButtonsEnabled(false);
    qDebug() << "UI Locked";
}

void Controller::unlockUI() {
    m_isAnimating = false;
    m_panel->setButtonsEnabled(true);
    qDebug() << "UI Unlocked";
}

void Controller::onAnimationFinished() {
    unlockUI();
}

void Controller::onStructureChanged(int idx) {
    if (idx < 0 || idx > 2) return;

    unlockUI();

    m_data.clear();
    m_currentType = static_cast<StructType>(idx);

    LinearListScene* ls = dynamic_cast<LinearListScene*>(m_scene);
    if (ls) {
        ls->setStructureType(static_cast<LinearListScene::StructureType>(idx));
        qDebug() << "Switched to structure type:" << idx << ". Data cleared.";
    }
    else {
        qDebug() << "ERROR: Scene is not LinearListScene!";
    }
}

void Controller::onResetRequested() {
    m_data.clear();
    m_scene->reset();
    unlockUI();
    qDebug() << "Reset all.";
}

int Controller::findIndex(int value) {
    for (size_t i = 0; i < m_data.size(); ++i) {
        if (m_data[i] == value) return i;
    }
    return -1;
}

void Controller::onInsertRequested(const QString& valueStr) {
    qDebug() << "Insert Requested:" << valueStr;

    bool ok;
    int val = valueStr.toInt(&ok);
    if (!ok) {
        qDebug() << "Invalid integer input";
        return;
    }
    if (m_isAnimating) {
        qDebug() << "Ignored: Animation in progress";
        return;
    }

    if (findIndex(val) != -1) {
        qDebug() << "Value already exists";
        return;
    }

    lockUI();

    int index = 0;
    if (m_currentType == LINKED || m_currentType == ARRAY) {
        index = m_data.size(); // 尾插
        m_data.push_back(val);
    }
    else if (m_currentType == STACK) {
        index = m_data.size(); // 栈顶
        m_data.push_back(val);
    }

    qDebug() << "Invoking scene insert: val=" << val << " idx=" << index;
    m_scene->insertNodeAnimated(val, index);
}

void Controller::onRemoveRequested(const QString& valueStr) {
    qDebug() << "Remove Requested:" << valueStr;

    bool ok;
    int val = valueStr.toInt(&ok);
    if (!ok || m_isAnimating) return;

    int index = findIndex(val);
    if (index == -1) {
        qDebug() << "Value not found";
        return;
    }

    lockUI();
    m_data.erase(m_data.begin() + index);
    m_scene->removeNodeAnimated(val, index);
}

void Controller::onFindRequested(const QString& valueStr) {
    bool ok;
    int val = valueStr.toInt(&ok);
    if (!ok || m_isAnimating) return;

    int index = findIndex(val);
    if (index != -1) {
        lockUI();
        m_scene->searchNodeAnimated(val, index);
    }
    else {
        qDebug() << "Not found";
    }
}