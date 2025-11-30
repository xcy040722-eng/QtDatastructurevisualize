#pragma once
#include <QObject>
#include <vector>

class BaseScene;
class ControlPanel;
class DeepSeekBridge;
// === 新增：前置声明具体场景类 ===
class TreeScene;
class HuffmanScene;

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

    void onAskAiRequested();
    void onAiResponse(const QString& dslCmd);
    void onAiError(const QString& errorMsg);

private:
    BaseScene* m_scene = nullptr;
    ControlPanel* m_panel = nullptr;
    DeepSeekBridge* m_ai = nullptr;

    BaseScene* m_linearScene = nullptr;
    // === 关键修改：使用具体类型指针，而不是基类指针 ===
    TreeScene* m_treeScene = nullptr;
    HuffmanScene* m_huffmanScene = nullptr;

    std::vector<int> m_data;
    enum StructType { LINKED = 0, ARRAY = 1, STACK = 2, TREE = 3, AVL = 4, HUFFMAN = 5 } m_currentType = LINKED;
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