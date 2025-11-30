#pragma once
#include <QObject>
#include <vector>

class BaseScene;
class ControlPanel;
class DeepSeekBridge;

// 场景类前置声明
class TreeScene;
class HuffmanScene;
class LinearListScene;
// === 新增：AVL 新架构 ===
class AVLScene;
#include "avl_logic.h" // 包含逻辑层头文件

class Controller : public QObject {
    Q_OBJECT
public:
    explicit Controller(BaseScene* scene, ControlPanel* panel, QObject* parent = nullptr);

public slots:
    // UI 交互槽函数
    void onInsertRequested(const QString& value);
    void onRemoveRequested(const QString& value);
    void onFindRequested(const QString& value);
    void onTraverseRequested(int type);
    void onResetRequested();
    void onStructureChanged(int idx);
    void onAnimationFinished();

    // 系统功能
    void onSaveRequested();
    void onLoadRequested();
    void onCommandEntered(const QString& cmd);

    // AI 相关
    void onAskAiRequested();
    void onAiResponse(const QString& dslCmd);
    void onAiError(const QString& errorMsg);

private:
    // === 核心成员 ===
    BaseScene* m_scene = nullptr;       // 当前激活的场景接口
    ControlPanel* m_panel = nullptr;
    DeepSeekBridge* m_ai = nullptr;

    // 各个具体场景实例
    BaseScene* m_linearScene = nullptr;
    TreeScene* m_treeScene = nullptr;
    HuffmanScene* m_huffmanScene = nullptr;

    // === 新增：AVL 专用成员 ===
    AVLScene* m_avlScene = nullptr;     // 渲染器
    AVLLogic m_avlLogic;                // 逻辑核心

    // 数据与状态
    std::vector<int> m_data;
    enum StructType { LINKED = 0, ARRAY = 1, STACK = 2, TREE = 3, AVL = 4, HUFFMAN = 5 } m_currentType = LINKED;
    bool m_isAnimating = false;
    QString m_endAnimationMsg;

    // 辅助函数
    int findIndex(int value);
    void lockUI();
    void unlockUI();
    void showError(const QString& msg);
    void switchScene(BaseScene* newScene);
    void batchInsert(const std::vector<int>& data);
    std::vector<int> parseArrayString(const QString& str);
};