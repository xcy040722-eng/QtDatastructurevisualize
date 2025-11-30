#include "mainwindow.h"
#include "controlpanel.h"
#include "linearlistscene.h"
#include "treescene.h"
#include "huffmanscene.h"
#include "controller.h"
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QSplitter> 
#include <QWheelEvent>
#include <QScrollBar> // === 新增 ===
#include <QTimer>     // === 新增 ===
#include <QDebug>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // 1. 设置主窗口属性
    resize(1200, 800);
    setWindowTitle(QStringLiteral("数据结构可视化模拟器 (Final Version)"));

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout* layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // === 2. 初始化组件 ===
    m_panel = new ControlPanel(this);
    m_scene = new LinearListScene(this);

    m_view = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing);

    // === 关键优化 A：强制左上对齐 ===
    m_view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_view->setBackgroundBrush(QColor(245, 245, 245));

    // === 关键优化 C：交互模式 ===
    // ScrollHandDrag: 鼠标变成手掌，按住左键可拖动画布
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    // 确保滚动条按需显示
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // === 关键优化 B：Splitter 布局 ===
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->addWidget(m_panel);
    m_splitter->addWidget(m_view);

    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    m_panel->setMinimumWidth(260);

    QList<int> sizes;
    sizes << 280 << 920;
    m_splitter->setSizes(sizes);

    layout->addWidget(m_splitter);

    m_controller = new Controller(m_scene, m_panel, this);

    // === 终极修复：强制视图回到 (0,0) ===
    // 使用 Timer 是因为 View 初始化需要一点时间才能计算出滚动条范围
    QTimer::singleShot(0, this, [this]() {
        if (m_view) {
            m_view->centerOn(0, 0); // 聚焦原点
            // 或者双重保险：直接设滚动条
            if (m_view->horizontalScrollBar()) m_view->horizontalScrollBar()->setValue(0);
            if (m_view->verticalScrollBar()) m_view->verticalScrollBar()->setValue(0);
        }
        });

    qDebug() << "MainWindow Initialized with Fixed Viewport";
}

void MainWindow::wheelEvent(QWheelEvent* event) {
    if (m_view) {
        const double scaleFactor = 1.1;
        if (event->angleDelta().y() > 0) {
            m_view->scale(scaleFactor, scaleFactor);
        }
        else {
            m_view->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        }
    }
}