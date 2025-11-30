#include "controller.h"
#include "basescene.h"
#include "linearlistscene.h"
#include "treescene.h" 
#include "huffmanscene.h" 
#include "controlpanel.h"
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsView>
#include <QScrollBar>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QComboBox> 

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

    connect(m_panel, &ControlPanel::saveRequested, this, &Controller::onSaveRequested);
    connect(m_panel, &ControlPanel::loadRequested, this, &Controller::onLoadRequested);

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
        view->centerOn(0, 0);
        if (view->horizontalScrollBar()) view->horizontalScrollBar()->setValue(0);
        if (view->verticalScrollBar()) view->verticalScrollBar()->setValue(0);
        view->update();
    }
}

void Controller::onStructureChanged(int idx) {
    if (idx < 0 || idx > 4) return;
    unlockUI();
    m_data.clear();
    m_endAnimationMsg.clear();
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

// === 文件保存实现 ===
void Controller::onSaveRequested() {
    if (m_data.empty()) {
        showError("当前没有数据可保存！");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(m_panel, "保存数据结构", "", "JSON Files (*.json)");
    if (fileName.isEmpty()) return;

    QJsonObject rootObj;
    rootObj["type"] = static_cast<int>(m_currentType);

    QJsonArray dataArray;
    for (int val : m_data) {
        dataArray.append(val);
    }
    rootObj["data"] = dataArray;

    QJsonDocument doc(rootObj);
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        QMessageBox::information(m_panel, "成功", "文件保存成功！");
    }
    else {
        showError("无法写入文件！");
    }
}

// === 文件读取实现 ===
void Controller::onLoadRequested() {
    if (m_isAnimating) return;

    QString fileName = QFileDialog::getOpenFileName(m_panel, "打开数据结构", "", "JSON Files (*.json)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        showError("无法打开文件！");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull() || !doc.isObject()) {
        showError("文件格式错误！");
        return;
    }

    QJsonObject rootObj = doc.object();
    if (!rootObj.contains("type") || !rootObj.contains("data")) {
        showError("数据结构文件无效！");
        return;
    }

    int type = rootObj["type"].toInt();
    QJsonArray dataArray = rootObj["data"].toArray();

    QComboBox* combo = m_panel->findChild<QComboBox*>();
    if (combo) {
        if (combo->currentIndex() != type) {
            combo->setCurrentIndex(type);
        }
        else {
            onResetRequested();
        }
    }

    std::vector<int> newData;
    for (const auto& val : dataArray) {
        newData.push_back(val.toInt());
    }

    batchInsert(newData);
}

// === 核心修复位置 ===
void Controller::batchInsert(const std::vector<int>& data) {
    if (data.empty()) return;

    struct Context {
        std::vector<int> vals;
        int idx = 0;
        Controller* ctrl;
    };

    QTimer* timer = new QTimer(this);
    Context* ctx = new Context{ data, 0, this };

    connect(timer, &QTimer::timeout, this, [this, timer, ctx]() {
        if (ctx->idx >= ctx->vals.size()) {
            timer->stop();
            timer->deleteLater();
            delete ctx;
            return;
        }

        int val = ctx->vals[ctx->idx];

        lockUI();

        // === 修复开始 ===
        // 1. 先获取当前的 index (对于空列表，size是0，所以index是0)
        int index = m_data.size();

        // 2. 然后再存入 Controller 数据
        m_data.push_back(val);

        // 3. 最后通知 Scene (此时传入的 index 0 对于空列表是合法的)
        m_scene->insertNodeAnimated(val, index);
        // === 修复结束 ===

        ctx->idx++;
        });

    timer->start(1000);
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
    if (!ok) { showError("请输入有效的整数！"); return; }

    if (m_currentType != STACK && m_currentType != HUFFMAN && findIndex(val) != -1) {
        showError("该数值已存在！");
        return;
    }

    lockUI();
    // 手动插入逻辑：先获取位置，再 push
    int index = m_data.size();
    m_data.push_back(val);
    m_scene->insertNodeAnimated(val, index);
}

void Controller::onRemoveRequested(const QString& valueStr) {
    if (m_isAnimating) return;

    if (m_currentType == HUFFMAN) {
        lockUI(); m_scene->removeNodeAnimated(0, 0); return;
    }
    if (m_currentType == STACK) {
        if (m_data.empty()) { showError("栈已经空了！"); return; }
        lockUI();
        int val = m_data.back();
        int index = m_data.size() - 1;
        m_data.pop_back();
        m_scene->removeNodeAnimated(val, index);
        return;
    }
    bool ok; int val = valueStr.toInt(&ok);
    if (!ok) { showError("请输入数值！"); return; }
    int index = findIndex(val);
    if (index == -1) { showError("未找到该数值！"); return; }
    lockUI();
    m_data.erase(m_data.begin() + index);
    m_scene->removeNodeAnimated(val, index);
}

void Controller::onFindRequested(const QString& valueStr) {
    if (m_isAnimating) return;
    if (m_currentType == STACK || m_currentType == HUFFMAN) return;
    bool ok; int val = valueStr.toInt(&ok);
    if (!ok) { showError("请输入数值！"); return; }
    int index = findIndex(val);
    if (index != -1) {
        lockUI(); m_scene->searchNodeAnimated(val, index);
    }
    else {
        if (m_currentType == TREE) {
            lockUI(); m_endAnimationMsg = QStringLiteral("未找到该数值！");
            m_scene->searchNodeAnimated(val, index);
        }
        else {
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