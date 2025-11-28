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
    BaseScene* m_scene = nullptr;
    ControlPanel* m_panel = nullptr;

    std::vector<int> m_data;
    enum StructType { LINKED = 0, ARRAY = 1, STACK = 2 } m_currentType = LINKED;
    bool m_isAnimating = false;

    int findIndex(int value);
    void lockUI();
    void unlockUI();

    // ¸¨Öú£º±¨´íµ¯´°
    void showError(const QString& msg);
};