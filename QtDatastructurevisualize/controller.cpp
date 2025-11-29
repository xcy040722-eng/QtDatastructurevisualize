#include "controller.h"
#include "basescene.h"
#include "linearlistscene.h"
#include "treescene.h" // === 包含树场景 ===
#include "controlpanel.h"
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsView>

Controller::Controller(BaseScene* scene, ControlPanel* panel, QObject* parent)
    : QObject(parent), m_panel(panel)
{
    // 初始场景是 LinearListScene
    m_linearScene = scene;
    m_scene = m_linearScene;

    // 预创建 TreeScene
    m_treeScene = new TreeScene(this);

    connect(m_panel, &ControlPanel::insertRequested, this, &Controller::onInsertRequested);
    connect(m_panel, &ControlPanel::removeRequested, this, &Controller::onRemoveRequested);
    connect(m_panel, &ControlPanel::findRequested, this, &Controller::onFindRequested);
    connect(m_panel, &ControlPanel::resetRequested, this, &Controller::onResetRequested);
    connect(m_panel, &ControlPanel::structureChanged, this, &Controller::onStructureChanged);

    // 连接信号
    connect(m_scene, &BaseScene::animationFinished, this, &Controller::onAnimationFinished);
    connect(m_treeScene, &BaseScene::animationFinished, this, &Controller::onAnimationFinished);
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

// 核心：切换视图显示的场景
void Controller::switchScene(BaseScene* newScene) {
    if (m_scene == newScene) return;

    m_scene = newScene;

    // 查找 MainWindow 中的 QGraphicsView
    QGraphicsView* view = parent()->findChild<QGraphicsView*>();
    if (view) {
        view->setScene(m_scene);
        view->update();
    }
}

void Controller::onStructureChanged(int idx) {
    if (idx < 0 || idx > 3) return;
    unlockUI();
    m_data.clear();
    m_currentType = static_cast<StructType>(idx);

    if (idx == TREE) {
        m_treeScene->reset();
        switchScene(m_treeScene);
        qDebug() << "Switched to Tree Scene";
    }
    else {
        LinearListScene* ls = dynamic_cast<LinearListScene*>(m_linearScene);
        if (ls) {
            ls->setStructureType(static_cast<LinearListScene::StructureType>(idx));
        }
        m_linearScene->reset();
        switchScene(m_linearScene);
        qDebug() << "Switched to Linear Scene: " << idx;
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

    if (m_currentType != STACK && findIndex(val) != -1) {
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
    if (m_currentType == STACK) return;

    bool ok;
    int val = valueStr.toInt(&ok);
    if (!ok) {
        showError(QStringLiteral("请输入数值！"));
        return;
    }

    int index = findIndex(val);
    // Tree 模式下即使 index 为 -1 (未在 m_data 中找到) 也允许调用，因为 TreeScene 内部可能有更复杂的逻辑
    // 但为了安全，这里还是校验了 index。
    if (index != -1) {
        lockUI();
        m_scene->searchNodeAnimated(val, index);
    }
    else {
        showError(QStringLiteral("未找到该数值！"));
    }
}