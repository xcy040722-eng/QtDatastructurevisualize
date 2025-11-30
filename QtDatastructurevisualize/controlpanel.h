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

    // 获取当前输入框的文字（给 Controller 发给 AI 用）
    QString getCommandText() const;
    // 清空输入框
    void clearCommandText();
    // 设置输入框文字（用于回填 AI 指令）
    void setCommandText(const QString& text);

signals:
    void insertRequested(const QString& value);
    void removeRequested(const QString& value);
    void findRequested(const QString& value);
    void traverseRequested(int type);
    void resetRequested();
    void structureChanged(int idx);
    void saveRequested();
    void loadRequested();
    void commandEntered(const QString& cmd);

    // === 新增：AI 请求信号 ===
    void askAiRequested();

private:
    QComboBox* m_structCombo = nullptr;
    QLineEdit* m_valueEdit = nullptr;

    QPushButton* m_insertBtn = nullptr;
    QPushButton* m_removeBtn = nullptr;
    QPushButton* m_findBtn = nullptr;

    QLabel* m_traverseLabel = nullptr;
    QComboBox* m_traverseCombo = nullptr;
    QPushButton* m_traverseBtn = nullptr;

    QPushButton* m_saveBtn = nullptr;
    QPushButton* m_loadBtn = nullptr;
    QPushButton* m_resetBtn = nullptr;

    QLineEdit* m_cmdLine = nullptr;
    QPushButton* m_helpBtn = nullptr;
    // === 新增：AI 按钮 ===
    QPushButton* m_aiBtn = nullptr;

    void setupUi();
    void setupConnections();
    void updateUIState(int index);
    void updateButtonTexts(int index);
    void showDslHelp();
};