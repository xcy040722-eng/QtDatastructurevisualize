#pragma once
#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QLabel;

class ControlPanel : public QWidget {
    Q_OBJECT
public:
    explicit ControlPanel(QWidget* parent = nullptr);
    void setButtonsEnabled(bool enable);
    int getCurrentStructureIndex() const;

signals:
    void insertRequested(const QString& value);
    void removeRequested(const QString& value);
    void findRequested(const QString& value);
    // 新增信号
    void traverseRequested(int type);
    void resetRequested();
    void structureChanged(int idx);

private:
    QComboBox* m_structCombo = nullptr;
    QLineEdit* m_valueEdit = nullptr;

    // 基础操作
    QPushButton* m_insertBtn = nullptr;
    QPushButton* m_removeBtn = nullptr;
    QPushButton* m_findBtn = nullptr;

    // === 新增：遍历控制 ===
    QLabel* m_traverseLabel = nullptr;
    QComboBox* m_traverseCombo = nullptr;
    QPushButton* m_traverseBtn = nullptr;

    QPushButton* m_resetBtn = nullptr;

    void setupUi();
    void setupConnections();
    void updateUIState(int index); // 更新界面显示状态
};