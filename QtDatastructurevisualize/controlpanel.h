#pragma once
#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;

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
    void resetRequested();
    void structureChanged(int idx);

private:
    QComboBox* m_structCombo = nullptr;
    QLineEdit* m_valueEdit = nullptr;
    QPushButton* m_insertBtn = nullptr;
    QPushButton* m_removeBtn = nullptr;
    QPushButton* m_findBtn = nullptr;
    QPushButton* m_resetBtn = nullptr;

    void setupUi();
    void setupConnections();
    void updateButtonTexts(int index);
};