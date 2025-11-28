#include "mainwindow.h"
#include "controlpanel.h"
#include "linearlistscene.h" 
#include "controller.h"
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QHBoxLayout* layout = new QHBoxLayout(central);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    m_panel = new ControlPanel(this);

    // 使用具体的 LinearListScene
    m_scene = new LinearListScene(this);

    m_view = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_view->setBackgroundBrush(Qt::lightGray);

    layout->addWidget(m_panel);
    layout->addWidget(m_view, 1);

    m_controller = new Controller(m_scene, m_panel, this);

    qDebug() << "MainWindow Initialized";
}