#include "controller.h"
#include "basescene.h"
#include "linearlistscene.h"
#include "treescene.h" 
#include "huffmanscene.h" 
#include "controlpanel.h"
#include "deepseekbridge.h" // === 包含 AI 头文件 ===
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
#include <QRegularExpression>

Controller::Controller(BaseScene* scene, ControlPanel* panel, QObject* parent)
    : QObject(parent), m_panel(panel)
{
    m_linearScene = scene;
    m_scene = m_linearScene;
    m_treeScene = new TreeScene(this);
    m_huffmanScene = new HuffmanScene(this);

    // === 初始化 AI ===
    m_ai = new DeepSeekBridge(this);
    connect(m_ai, &DeepSeekBridge::responseReceived, this, &Controller::onAiResponse);
    connect(m_ai, &DeepSeekBridge::errorOccurred, this, &Controller::onAiError);

    connect(m_panel, &ControlPanel::insertRequested, this, &Controller::onInsertRequested);
    connect(m_panel, &ControlPanel::removeRequested, this, &Controller::onRemoveRequested);
    connect(m_panel, &ControlPanel::findRequested, this, &Controller::onFindRequested);
    connect(m_panel, &ControlPanel::traverseRequested, this, &Controller::onTraverseRequested);
    connect(m_panel, &ControlPanel::resetRequested, this, &Controller::onResetRequested);
    connect(m_panel, &ControlPanel::structureChanged, this, &Controller::onStructureChanged);

    connect(m_panel, &ControlPanel::saveRequested, this, &Controller::onSaveRequested);
    connect(m_panel, &ControlPanel::loadRequested, this, &Controller::onLoadRequested);
    connect(m_panel, &ControlPanel::commandEntered, this, &Controller::onCommandEntered);

    // === 连接 AI 按钮 ===
    connect(m_panel, &ControlPanel::askAiRequested, this, &Controller::onAskAiRequested);

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

void Controller::onSaveRequested() {
    if (m_data.empty()) { showError("无数据可保存"); return; }
    QString fileName = QFileDialog::getSaveFileName(m_panel, "保存", "", "JSON (*.json)");
    if (fileName.isEmpty()) return;

    QJsonObject rootObj;
    rootObj["type"] = static_cast<int>(m_currentType);
    QJsonArray arr;
    for (int val : m_data) arr.append(val);
    rootObj["data"] = arr;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(rootObj).toJson());
        file.close();
    }
}

void Controller::onLoadRequested() {
    if (m_isAnimating) return;
    QString fileName = QFileDialog::getOpenFileName(m_panel, "打开", "", "JSON (*.json)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject rootObj = doc.object();
    int type = rootObj["type"].toInt();

    QComboBox* combo = m_panel->findChild<QComboBox*>();
    if (combo) {
        if (combo->currentIndex() != type) combo->setCurrentIndex(type);
        else onResetRequested();
    }

    std::vector<int> newData;
    for (const auto& val : rootObj["data"].toArray()) newData.push_back(val.toInt());
    batchInsert(newData);
}

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
            timer->stop(); timer->deleteLater(); delete ctx; return;
        }
        int val = ctx->vals[ctx->idx];
        lockUI();
        int index = m_data.size(); // 修复后的 index 逻辑
        m_data.push_back(val);
        m_scene->insertNodeAnimated(val, index);
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

// === DSL 解析辅助 ===
std::vector<int> Controller::parseArrayString(const QString& str) {
    std::vector<int> res;
    // 匹配 [1, 2, 3] 格式
    QRegularExpression re("\\[(.*?)\\]");
    QRegularExpressionMatch match = re.match(str);
    if (match.hasMatch()) {
        QString content = match.captured(1);
        QStringList parts = content.split(QRegularExpression("[,\\s]+"), Qt::SkipEmptyParts);
        for (const QString& part : parts) {
            bool ok;
            int v = part.toInt(&ok);
            if (ok) res.push_back(v);
        }
    }
    return res;
}
// === AI 交互逻辑 ===

void Controller::onAskAiRequested() {
    if (m_isAnimating) return;
    QString prompt = m_panel->getCommandText();
    if (prompt.isEmpty()) {
        showError("请先在输入框中描述您的需求！");
        return;
    }

    // 锁定 UI，并显示加载状态
    lockUI();
    m_panel->setCommandText("AI 思考中...");

    // 发送请求
    m_ai->query(prompt);
}

void Controller::onAiResponse(const QString& dslCmd) {
    // 收到回复，解锁 UI（注意：onCommandEntered 会再次锁定 UI 进行动画，所以这里先解锁是安全的，或者直接衔接）
    unlockUI();

    if (dslCmd == "ERROR") {
        m_panel->setCommandText("AI 无法理解该指令");
        showError("AI 无法理解您的需求，请尝试换种说法。\n例如：'建一个包含1,2,3的树'");
    }
    else {
        qDebug() << "AI Generated DSL:" << dslCmd;
        m_panel->setCommandText(dslCmd); // 将翻译结果填回输入框
        // 立即执行
        onCommandEntered(dslCmd);
    }
}

void Controller::onAiError(const QString& errorMsg) {
    unlockUI();
    m_panel->clearCommandText();
    showError("AI 服务连接失败:\n" + errorMsg);
}



// === 核心：DSL 指令执行 ===
void Controller::onCommandEntered(const QString& rawCmd) {
    if (m_isAnimating) return;

    QString cmd = rawCmd.trimmed().toLower();
    QStringList parts = cmd.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (parts.isEmpty()) return;

    QString action = parts[0];

    // 1. 快速构建: new bst [1,2,3]
    if (action == "new" || action == "build") {
        if (parts.size() < 3) { showError("语法错误: new <type> [data]"); return; }
        QString typeStr = parts[1];
        int typeIdx = -1;

        if (typeStr == "list") typeIdx = LINKED;
        else if (typeStr == "array") typeIdx = ARRAY;
        else if (typeStr == "stack") typeIdx = STACK;
        else if (typeStr == "bst" || typeStr == "tree") typeIdx = TREE;
        else if (typeStr == "huffman") typeIdx = HUFFMAN;

        if (typeIdx == -1) { showError("未知类型: " + typeStr); return; }

        // 切换类型
        QComboBox* combo = m_panel->findChild<QComboBox*>();
        if (combo) {
            if (combo->currentIndex() != typeIdx) combo->setCurrentIndex(typeIdx);
            else onResetRequested();
        }

        // 解析数据并批量插入
        std::vector<int> data = parseArrayString(rawCmd); // 传原始 cmd 以保留括号
        if (!data.empty()) batchInsert(data);
        return;
    }

    // 2. 插入/入栈: insert 50 / push 50
    if (action == "insert" || action == "push" || action == "add") {
        if (parts.size() < 2) { showError("请输入数值"); return; }
        onInsertRequested(parts[1]);
        return;
    }

    // 3. 删除/出栈: delete 50 / pop
    if (action == "delete" || action == "remove" || action == "pop") {
        if (m_currentType == STACK || m_currentType == HUFFMAN) {
            onRemoveRequested(""); // 栈/哈夫曼不需要参数
        }
        else {
            if (parts.size() < 2) { showError("请输入数值"); return; }
            onRemoveRequested(parts[1]);
        }
        return;
    }

    // 4. 查找: find 50
    if (action == "find" || action == "search") {
        if (parts.size() < 2) { showError("请输入数值"); return; }
        onFindRequested(parts[1]);
        return;
    }

    // 5. 遍历: traverse pre
    if (action == "traverse") {
        if (parts.size() < 2) return;
        QString mode = parts[1];
        int t = -1;
        if (mode.startsWith("pre")) t = 0;
        else if (mode.startsWith("in")) t = 1;
        else if (mode.startsWith("post")) t = 2;

        if (t != -1) onTraverseRequested(t);
        else showError("遍历模式: pre, in, post");
        return;
    }

    showError("未知指令: " + action);
}

// ... [原有 onInsertRequested 等实现保持不变] ...
// 为节省空间，请保留原文件下方的 onInsertRequested, onRemoveRequested 等函数
void Controller::onInsertRequested(const QString& valueStr) {
    if (m_isAnimating) return;
    bool ok; int val = valueStr.toInt(&ok);
    if (!ok) { showError("请输入有效的整数！"); return; }
    if (m_currentType != STACK && m_currentType != HUFFMAN && findIndex(val) != -1) {
        showError("该数值已存在！"); return;
    }
    lockUI();
    int index = m_data.size();
    m_data.push_back(val);
    m_scene->insertNodeAnimated(val, index);
}

void Controller::onRemoveRequested(const QString& valueStr) {
    if (m_isAnimating) return;
    if (m_currentType == HUFFMAN) { lockUI(); m_scene->removeNodeAnimated(0, 0); return; }
    if (m_currentType == STACK) {
        if (m_data.empty()) { showError("栈已经空了！"); return; }
        lockUI();
        int val = m_data.back(); int index = m_data.size() - 1; m_data.pop_back();
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
    if (index != -1) { lockUI(); m_scene->searchNodeAnimated(val, index); }
    else {
        if (m_currentType == TREE) { lockUI(); m_endAnimationMsg = "未找到该数值！"; m_scene->searchNodeAnimated(val, index); }
        else { showError("未找到该数值！"); }
    }
}

void Controller::onTraverseRequested(int type) {
    if (m_isAnimating) return;
    if (m_currentType != TREE) return;
    lockUI();
    m_scene->traverseAnimated(type);
}