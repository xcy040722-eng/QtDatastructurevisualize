#include "mainwindow.h"
#include "controlpanel.h"
#include "basescene.h"
#include "linearlistscene.h"
#include "controller.h"

#include <QGraphicsView>
#include <QHBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    auto* central = new QWidget(this);
    auto* h = new QHBoxLayout(central);

    m_panel = new ControlPanel(central);
    m_scene = new LinearListScene(this);
    m_view = new QGraphicsView(m_scene, central);
    m_view->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    h->addWidget(m_panel);
    h->addWidget(m_view, 1);

    setCentralWidget(central);

    // controller: connect panel -> controller; controller will connect model->scene
    m_controller = new Controller(m_scene, this);
    connect(m_panel, &ControlPanel::insertRequested, m_controller, &Controller::onInsertRequested);
    connect(m_panel, &ControlPanel::removeRequested, m_controller, &Controller::onRemoveRequested);
    connect(m_panel, &ControlPanel::findRequested, m_controller, &Controller::onFindRequested);
    connect(m_panel, &ControlPanel::resetRequested, m_controller, &Controller::onResetRequested);
    connect(m_panel, &ControlPanel::structureChanged, m_controller, &Controller::onStructureChanged);
}
