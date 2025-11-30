#pragma once
#include <QObject>
#include <vector>

class BaseScene;
class ControlPanel;
class DeepSeekBridge; // === 新增前置声明 ===

class Controller : public QObject {
    Q_OBJECT
public:
    explicit Controller(BaseScene* scene, ControlPanel* panel, QObject* parent = nullptr);

public slots:
    void onInsertRequested(const QString& value);
    void onRemoveRequested(const QString& value);
    void onFindRequested(const QString& value);
    void onTraverseRequested(int type);
    void onResetRequested();
    void onStructureChanged(int idx);
    void onAnimationFinished();
    void onSaveRequested();
    void onLoadRequested();
    void onCommandEntered(const QString& cmd);

    // === 新增：AI 交互槽 ===
    void onAskAiRequested();
    void onAiResponse(const QString& dslCmd);
    void onAiError(const QString& errorMsg);

private:
    BaseScene* m_scene = nullptr;
    ControlPanel* m_panel = nullptr;

    // === 新增：AI 桥接器 ===
    DeepSeekBridge* m_ai = nullptr;

    BaseScene* m_linearScene = nullptr;
    BaseScene* m_treeScene = nullptr;
    BaseScene* m_huffmanScene = nullptr;

    std::vector<int> m_data;
    enum StructType { LINKED = 0, ARRAY = 1, STACK = 2, TREE = 3, HUFFMAN = 4 } m_currentType = LINKED;
    bool m_isAnimating = false;
    QString m_endAnimationMsg;

    int findIndex(int value);
    void lockUI();
    void unlockUI();
    void showError(const QString& msg);
    void switchScene(BaseScene* newScene);
    void batchInsert(const std::vector<int>& data);
    std::vector<int> parseArrayString(const QString& str);
};