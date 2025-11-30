#pragma once
#include <QMainWindow>

class ControlPanel;
class BaseScene;
class QGraphicsView;
class Controller;
class QSplitter; // === 新增：前置声明 ===

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    // 滚轮缩放事件
    void wheelEvent(QWheelEvent* event) override;

private:
    QSplitter* m_splitter = nullptr; // === 新增：分裂器部件 ===
    ControlPanel* m_panel = nullptr;
    QGraphicsView* m_view = nullptr;
    BaseScene* m_scene = nullptr;
    Controller* m_controller = nullptr;
};