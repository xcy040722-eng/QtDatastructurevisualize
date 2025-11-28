// mainwindow.cpp
#include "mainwindow.h"
#include "controlpanel.h"
#include "visualscene.h"
#include "controller.h"

#include <QGraphicsView>
#include <QHBoxLayout>
#include <QWidget>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setupUi();
    setupConnections();

    qDebug().noquote() << u8"MainWindow 已初始化。";
}

MainWindow::~MainWindow()
{
    // Qt 会自动删除子 widget（parent-child 机制），此处无需手动 delete
    qDebug().noquote() << u8"MainWindow 已销毁。";
}

/**
 * @brief setupUi 创建界面布局并初始化控件
 */
void MainWindow::setupUi()
{
    // 中央窗口容器
    QWidget* central = new QWidget(this);
    this->setCentralWidget(central);

    // 左侧控制面板（放按钮、输入框等）
    m_controlPanel = new ControlPanel(central);

    // 右侧：QGraphicsView + VisualScene
    m_scene = new VisualScene(this);
    m_graphicsView = new QGraphicsView(m_scene, central);
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    m_graphicsView->setMinimumSize(600, 400);

    // 布局：左侧窄列，右侧展示区
    QHBoxLayout* hLayout = new QHBoxLayout(central);
    hLayout->addWidget(m_controlPanel, 0); // stretch 0 ，固定宽度
    hLayout->addWidget(m_graphicsView, 1); // stretch 1，自适应

    // 初始化控制器
    m_controller = new Controller(m_scene, this);
}

/**
 * @brief setupConnections 连接 ControlPanel 发出的信号到 Controller（中转）
 */
void MainWindow::setupConnections()
{
    // ControlPanel -> Controller
    connect(m_controlPanel, &ControlPanel::insertRequested,
        m_controller, &Controller::onInsertRequested);
    connect(m_controlPanel, &ControlPanel::removeRequested,
        m_controller, &Controller::onRemoveRequested);
    connect(m_controlPanel, &ControlPanel::findRequested,
        m_controller, &Controller::onFindRequested);

    connect(m_controlPanel, &ControlPanel::playRequested,
        m_controller, &Controller::onPlayRequested);
    connect(m_controlPanel, &ControlPanel::pauseRequested,
        m_controller, &Controller::onPauseRequested);
    connect(m_controlPanel, &ControlPanel::resetRequested,
        m_controller, &Controller::onResetRequested);
}
