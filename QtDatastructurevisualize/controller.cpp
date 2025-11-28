#include "controller.h"
#include "basescene.h"
#include "linearlistscene.h"
#include "controlpanel.h"
#include <QDebug>
#include <QMessageBox>

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
}

void Controller::lockUI() {
    m_isAnimating = true;
    m_panel->setButtonsEnabled(false);
}

void Controller::unlockUI() {
    m_isAnimating = false;
    m_panel->setButtonsEnabled(true);
}

void Controller::onAnimationFinished() {
    unlockUI();
}

void Controller::showError(const QString& msg) {
    QMessageBox::warning(m_panel, QStringLiteral("提示"), msg);
}

void Controller::onStructureChanged(int idx) {
    if (idx < 0 || idx > 2) return;
    unlockUI();
    m_data.clear();
    m_currentType = static_cast<StructType>(idx);

    LinearListScene* ls = dynamic_cast<LinearListScene*>(m_scene);
    if (ls) ls->setStructureType(static_cast<LinearListScene::StructureType>(idx));
}

void Controller::onResetRequested() {
    m_data.clear();
    m_scene->reset();
    unlockUI();
}

int Controller::findIndex(int value) {
    for (size_t i = 0; i < m_data.size(); ++i) {
        if (m_data[i] == value) return i;
    }
    return -1;
}

void Controller::onInsertRequested(const QString& valueStr) {
    if (m_isAnimating) return;

    bool ok;
    int val = valueStr.toInt(&ok);
    if (!ok) {
        showError(QStringLiteral("请输入有效的整数！"));
        return;
    }

    if (m_currentType != STACK && findIndex(val) != -1) {
        showError(QStringLiteral("该数值已存在！"));
        return;
    }

    lockUI();
    int index = m_data.size(); // 尾插 / 栈顶
    m_data.push_back(val);
    m_scene->insertNodeAnimated(val, index);
}

void Controller::onRemoveRequested(const QString& valueStr) {
    if (m_isAnimating) return;

    // === 栈的特殊处理：Pop ===
    if (m_currentType == STACK) {
        if (m_data.empty()) {
            showError(QStringLiteral("栈已经空了，无法出栈！"));
            return;
        }
        lockUI();
        // 只能移除栈顶
        int val = m_data.back();
        int index = m_data.size() - 1;
        m_data.pop_back();

        m_scene->removeNodeAnimated(val, index);
        return;
    }

    // === 链表/顺序表的处理：按值删除 ===
    bool ok;
    int val = valueStr.toInt(&ok);
    if (!ok) {
        showError(QStringLiteral("请输入要删除的节点数值！"));
        return;
    }

    int index = findIndex(val);
    if (index == -1) {
        showError(QStringLiteral("未找到该数值！"));
        return;
    }

    lockUI();
    m_data.erase(m_data.begin() + index);
    m_scene->removeNodeAnimated(val, index);
}

void Controller::onFindRequested(const QString& valueStr) {
    if (m_isAnimating) return;

    if (m_currentType == STACK) return;

    bool ok;
    int val = valueStr.toInt(&ok);
    if (!ok) {
        showError(QStringLiteral("请输入查找数值！"));
        return;
    }

    int index = findIndex(val);
    if (index != -1) {
        lockUI();
        m_scene->searchNodeAnimated(val, index);
    }
    else {
        showError(QStringLiteral("未找到该数值！"));
    }
}