#include "controller.h"
#include "basescene.h"
#include "linearlistscene.h"
#include "treescene.h" 
#include "huffmanscene.h" 
#include "controlpanel.h"
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsView>

Controller::Controller(BaseScene* scene, ControlPanel* panel, QObject* parent)
    : QObject(parent), m_panel(panel)
{
    m_linearScene = scene;
    m_scene = m_linearScene;
    m_treeScene = new TreeScene(this);
    m_huffmanScene = new HuffmanScene(this);

    connect(m_panel, &ControlPanel::insertRequested, this, &Controller::onInsertRequested);
    connect(m_panel, &ControlPanel::removeRequested, this, &Controller::onRemoveRequested);
    connect(m_panel, &ControlPanel::findRequested, this, &Controller::onFindRequested);
    connect(m_panel, &ControlPanel::traverseRequested, this, &Controller::onTraverseRequested);
    connect(m_panel, &ControlPanel::resetRequested, this, &Controller::onResetRequested);
    connect(m_panel, &ControlPanel::structureChanged, this, &Controller::onStructureChanged);

    connect(m_scene, &BaseScene::animationFinished, this, &Controller::onAnimationFinished);
    connect(m_treeScene, &BaseScene::animationFinished, this, &Controller::onAnimationFinished);
    connect(m_huffmanScene, &BaseScene::animationFinished, this, &Controller::onAnimationFinished);
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
    // === 新增：动画结束后检查是否有延迟消息 ===
    if (!m_endAnimationMsg.isEmpty()) {
        showError(m_endAnimationMsg);
        m_endAnimationMsg.clear();
    }
}

void Controller::showError(const QString& msg) {
    QMessageBox::warning(m_panel, QStringLiteral("提示"), msg);
}

void Controller::switchScene(BaseScene* newScene) {
    if (m_scene == newScene) return;
    m_scene = newScene;
    QGraphicsView* view = parent()->findChild<QGraphicsView*>();
    if (view) {
        view->setScene(m_scene);
        view->update();
    }
}

void Controller::onStructureChanged(int idx) {
    if (idx < 0 || idx > 4) return;
    unlockUI();
    m_data.clear();
    m_endAnimationMsg.clear(); // 清理残留消息
    m_currentType = static_cast<StructType>(idx);

    if (idx == TREE) {
        m_treeScene->reset();
        switchScene(m_treeScene);
    }
    else if (idx == HUFFMAN) {
        m_huffmanScene->reset();
        switchScene(m_huffmanScene);
    }
    else {
        LinearListScene* ls = dynamic_cast<LinearListScene*>(m_linearScene);
        if (ls) ls->setStructureType(static_cast<LinearListScene::StructureType>(idx));
        m_linearScene->reset();
        switchScene(m_linearScene);
    }
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

    if (m_currentType != STACK && m_currentType != HUFFMAN && findIndex(val) != -1) {
        showError(QStringLiteral("该数值已存在！"));
        return;
    }

    lockUI();
    int index = m_data.size();
    m_data.push_back(val);
    m_scene->insertNodeAnimated(val, index);
}

void Controller::onRemoveRequested(const QString& valueStr) {
    if (m_isAnimating) return;

    if (m_currentType == HUFFMAN) {
        lockUI();
        m_scene->removeNodeAnimated(0, 0);
        return;
    }

    if (m_currentType == STACK) {
        if (m_data.empty()) {
            showError(QStringLiteral("栈已经空了！"));
            return;
        }
        lockUI();
        int val = m_data.back();
        int index = m_data.size() - 1;
        m_data.pop_back();
        m_scene->removeNodeAnimated(val, index);
        return;
    }

    bool ok;
    int val = valueStr.toInt(&ok);
    if (!ok) {
        showError(QStringLiteral("请输入数值！"));
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
    if (m_currentType == STACK || m_currentType == HUFFMAN) return;

    bool ok;
    int val = valueStr.toInt(&ok);
    if (!ok) {
        showError(QStringLiteral("请输入数值！"));
        return;
    }

    int index = findIndex(val);

    // === 修复点：BST 查找未找到时的处理 ===
    if (index != -1) {
        // 找到了
        lockUI();
        m_scene->searchNodeAnimated(val, index);
    }
    else {
        // 没找到
        if (m_currentType == TREE) {
            // BST：先播动画（探针走到空），动画结束后再弹窗
            lockUI();
            m_endAnimationMsg = QStringLiteral("未找到该数值！");
            m_scene->searchNodeAnimated(val, index);
        }
        else {
            // 线性表：直接弹窗（因为线性表没找到通常不播动画，或者直接报错）
            showError(QStringLiteral("未找到该数值！"));
        }
    }
}

void Controller::onTraverseRequested(int type) {
    if (m_isAnimating) return;
    if (m_currentType != TREE) return;
    lockUI();
    m_scene->traverseAnimated(type);
}