#include "visualscene.h"
#include <QGraphicsTextItem>
#include <QPen>
#include <QBrush>

namespace {
    constexpr int NODE_W = 60;
    constexpr int NODE_H = 40;
    constexpr int START_X = 50;
    constexpr int Y = 150;
    constexpr int GAP = 20;
}

LinkedListScene::LinkedListScene(QObject* parent) : QGraphicsScene(parent) {
    setSceneRect(0, 0, 900, 400);
}

void LinkedListScene::onNodeInserted(int value) {
    auto rect = addRect(0, 0, NODE_W, NODE_H, QPen(Qt::black), QBrush(Qt::yellow));
    rect->setData(0, value);
    auto text = addText(QString::number(value));
    text->setParentItem(rect);
    text->setPos((NODE_W - text->boundingRect().width()) / 2.0,
        (NODE_H - text->boundingRect().height()) / 2.0);
    m_nodeMap[value] = rect;
    layoutNodes();
}

void LinkedListScene::onNodeDeleted(int value) {
    if (!m_nodeMap.contains(value)) return;
    removeItem(m_nodeMap[value]);
    delete m_nodeMap[value];
    m_nodeMap.remove(value);
    layoutNodes();
}

void LinkedListScene::onListCleared() {
    clear();
    m_nodeMap.clear();
}

void LinkedListScene::layoutNodes() {
    int x = START_X;
    for (auto rect : m_nodeMap) rect->setRect(x += NODE_W + GAP, Y, NODE_W, NODE_H);
}

void LinkedListScene::clearScene() {
    onListCleared();
}
