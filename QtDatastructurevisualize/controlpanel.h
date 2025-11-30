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

    // 用于在动画播放时禁用按钮
    void setButtonsEnabled(bool enable);

    // 获取当前选择的结构索引
    int getCurrentStructureIndex() const;

signals:
    // 用户操作信号
    void insertRequested(const QString& value);
    void removeRequested(const QString& value);
    void findRequested(const QString& value);

    // 遍历信号 (0:前序, 1:中序, 2:后序)
    void traverseRequested(int type);

    void resetRequested();
    void structureChanged(int idx);

private:
    QComboBox* m_structCombo = nullptr;
    QLineEdit* m_valueEdit = nullptr;

    // 基础操作按钮
    QPushButton* m_insertBtn = nullptr;
    QPushButton* m_removeBtn = nullptr;
    QPushButton* m_findBtn = nullptr;

    // === 遍历操作相关控件 ===
    QLabel* m_traverseLabel = nullptr;
    QComboBox* m_traverseCombo = nullptr;
    QPushButton* m_traverseBtn = nullptr;

    QPushButton* m_resetBtn = nullptr;

    // 内部初始化与逻辑
    void setupUi();
    void setupConnections();

    // 根据结构类型更新界面 (显示/隐藏遍历按钮)
    void updateUIState(int index);

    // 更新按钮文字 (如 Push/Pop, Add Leaf/Merge)
    void updateButtonTexts(int index);
};