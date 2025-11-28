#pragma once
// controlpanel.h
#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QSlider;

/**
 * @brief ControlPanel 控制面板（左侧）
 * 提供用户交互控件：结构选择、数值输入、操作按钮、动画控制等。
 */
class ControlPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ControlPanel(QWidget* parent = nullptr);

signals:
    // 用户希望插入某个值（字符串形式，以便扩展）
    void insertRequested(const QString& value);
    // 用户希望删除某个值
    void removeRequested(const QString& value);
    // 查找
    void findRequested(const QString& value);

    // 动画控制
    void playRequested();
    void pauseRequested();
    void resetRequested();
    void speedChanged(int speed);

private:
    QComboBox* m_structCombo = nullptr;
    QLineEdit* m_valueEdit = nullptr;
    QPushButton* m_insertBtn = nullptr;
    QPushButton* m_removeBtn = nullptr;
    QPushButton* m_findBtn = nullptr;

    QPushButton* m_playBtn = nullptr;
    QPushButton* m_pauseBtn = nullptr;
    QPushButton* m_resetBtn = nullptr;
    QSlider* m_speedSlider = nullptr;

    void setupUi();
    void setupConnections();
};

#endif // CONTROLPANEL_H
