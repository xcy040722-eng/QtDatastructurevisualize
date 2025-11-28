#pragma once
// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ControlPanel;
class VisualScene;
class QGraphicsView;
class Controller;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    // 控制面板（左侧操作区）
    ControlPanel* m_controlPanel = nullptr;
    // 用于显示 VisualScene 的视图（右侧显示区）
    QGraphicsView* m_graphicsView = nullptr;
    // 场景（绘制节点、指针等）
    VisualScene* m_scene = nullptr;
    // 控制器：连接界面与模型/动画（当前为占位）
    Controller* m_controller = nullptr;

    void setupUi();
    void setupConnections();
};

#endif // MAINWINDOW_H
