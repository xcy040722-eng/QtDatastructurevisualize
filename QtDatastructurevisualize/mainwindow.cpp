#include "mainwindow.h"
#include "controlpanel.h"
#include "linearlistscene.h"
#include "treescene.h" // === 关键：包含树场景 ===
#include "controller.h"
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QWheelEvent>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout* layout = new QHBoxLayout(central);
    layout->setContentsMargins(5, 5, 5, 5);

    m_panel = new ControlPanel(this);
    m_scene = new LinearListScene(this);

    m_view = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing);

    // === 布局设置 ===
    m_view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_view->setBackgroundBrush(Qt::lightGray);

    // 启用鼠标拖拽画布
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);

    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    layout->addWidget(m_panel);
    layout->addWidget(m_view, 1);

    m_controller = new Controller(m_scene, m_panel, this);

    resize(1000, 700);
    qDebug() << "MainWindow Initialized";
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