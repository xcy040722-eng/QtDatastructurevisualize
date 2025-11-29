#pragma once
#include <QObject>
#include <vector>

class BaseScene;
class ControlPanel;

class Controller : public QObject {
    Q_OBJECT
public:
    explicit Controller(BaseScene* scene, ControlPanel* panel, QObject* parent = nullptr);

public slots:
    void onInsertRequested(const QString& value);
    void onRemoveRequested(const QString& value);
    void onFindRequested(const QString& value);
    void onResetRequested();
    void onStructureChanged(int idx);
    void onAnimationFinished();

private:
    BaseScene* m_scene = nullptr; // 当前活跃的场景
    ControlPanel* m_panel = nullptr;

    // 场景缓存
    BaseScene* m_linearScene = nullptr;
    BaseScene* m_treeScene = nullptr;

    std::vector<int> m_data;
    enum StructType { LINKED = 0, ARRAY = 1, STACK = 2, TREE = 3 } m_currentType = LINKED;
    bool m_isAnimating = false;

    int findIndex(int value);
    void lockUI();
    void unlockUI();

    void showError(const QString& msg);

    // 切换场景方法
    void switchScene(BaseScene* newScene);
};