#pragma once
#include <QMainWindow>

class QGraphicsView;
class ControlPanel;
class BaseScene;
class Controller;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private:
    ControlPanel* m_panel = nullptr;
    QGraphicsView* m_view = nullptr;
    BaseScene* m_scene = nullptr;
    Controller* m_controller = nullptr;
};
