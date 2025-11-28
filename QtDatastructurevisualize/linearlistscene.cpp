#include "linearlistscene.h"
#include <QBrush>
#include <QPen>
#include <QVariantAnimation>
#include <QDebug>
#include <QtMath>
#include <QTimer>

LinearListScene::LinearListScene(QObject* parent) : BaseScene(parent) {
    setStructureType(0);
}

void LinearListScene::setStructureType(int type) {
    m_struct = type;
    // adjust scene rect or base coordinates if desired
    if (m_struct == 2) { // stack: show vertical layout
        setSceneRect(0, 0, 600, 800);
    }
    else {
        setSceneRect(0, 0, 1200, 600);
    }
    layoutNodesWithAnimation();
}

void LinearListScene::onNodeInserted(int value) {
    if (m_items.contains(value)) {
        qDebug() << "[Scene] value exists:" << value;
        return;
    }
    // add to order list (for array/linked we append)
    if (m_struct == 2) {
        // Stack: push to top
        m_order.append(value);
    }
    else {
        m_order.append(value);
    }
    // create graphic at above position and animate to final place
    createNodeGraphics(value, START_X, -100);
    layoutNodesWithAnimation();
}

void LinearListScene::onNodeDeleted(int value) {
    if (!m_items.contains(value)) {
        qDebug() << "[Scene] delete: not found" << value;
        return;
    }
    removeNodeGraphics(value);
    m_order.removeAll(value);
    layoutNodesWithAnimation();
}

void LinearListScene::onListCleared() {
    // delete all
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        if (it->arrow) { removeItem(it->arrow); delete it->arrow; it->arrow = nullptr; }
        if (it->text) { removeItem(it->text); delete it->text; it->text = nullptr; }
        if (it->rect) { removeItem(it->rect); delete it->rect; it->rect = nullptr; }
    }
    m_items.clear();
    m_order.clear();
}

void LinearListScene::createNodeGraphics(int value, qreal startX, qreal startY) {
    QBrush brush(Qt::yellow);
    QPen pen(Qt::black);
    auto rect = addRect(0, 0, NODE_W, NODE_H, pen, brush);
    rect->setZValue(1);
    rect->setPos(startX, startY);

    auto text = addSimpleText(QString::number(value));
    text->setParentItem(rect);
    // center text
    QRectF tb = text->boundingRect();
    text->setPos((NODE_W - tb.width()) / 2.0, (NODE_H - tb.height()) / 2.0);
    text->setZValue(2);

    NodeGraphics ng;
    ng.rect = rect; ng.text = text; ng.value = value;
    ng.arrow = nullptr;
    m_items[value] = ng;
}

void LinearListScene::removeNodeGraphics(int value) {
    if (!m_items.contains(value)) return;
    NodeGraphics ng = m_items[value];
    if (ng.arrow) { removeItem(ng.arrow); delete ng.arrow; ng.arrow = nullptr; }
    if (ng.text) { // child of rect so will be deleted with rect, but safe
        removeItem(ng.text);
        delete ng.text; ng.text = nullptr;
    }
    if (ng.rect) {
        removeItem(ng.rect);
        delete ng.rect; ng.rect = nullptr;
    }
    m_items.remove(value);
}

void LinearListScene::rebuildArrows() {
    // remove existing arrows first
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        if (it->arrow) { removeItem(it->arrow); delete it->arrow; it->arrow = nullptr; }
    }
    if (m_struct == 2) return; // stack doesn't need arrows
    // draw arrow from item i -> i+1
    for (int i = 0; i < m_order.size() - 1; ++i) {
        int v1 = m_order[i];
        int v2 = m_order[i + 1];
        if (!m_items.contains(v1) || !m_items.contains(v2)) continue;
        auto r1 = m_items[v1].rect;
        auto r2 = m_items[v2].rect;
        QPointF p1 = r1->pos() + QPointF(NODE_W, NODE_H / 2.0);
        QPointF p2 = r2->pos() + QPointF(0, NODE_H / 2.0);
        QGraphicsLineItem* line = addLine(QLineF(p1, p2), QPen(Qt::black, 2));
        line->setZValue(0.5);
        // draw simple triangular arrow head
        QPolygonF poly;
        const qreal arrowSize = 8;
        QLineF ln(p1, p2);
        double angle = ln.angle() * M_PI / 180.0;
        QPointF pHead = p2;
        QPointF pA = pHead + QPointF(qCos(angle + M_PI / 6) * arrowSize, -qSin(angle + M_PI / 6) * arrowSize);
        QPointF pB = pHead + QPointF(qCos(angle - M_PI / 6) * arrowSize, -qSin(angle - M_PI / 6) * arrowSize);
        poly << pHead << pA << pB;
        QGraphicsPolygonItem* head = addPolygon(poly, QPen(Qt::black), QBrush(Qt::black));
        head->setZValue(0.5);
        // group arrow: use a line pointer stored in first node (for removal later we only store pointer to line)
        m_items[v1].arrow = line;
        // Note: head is not tracked individually to keep code short; it's safe because it persists as scene item
    }
}

void LinearListScene::simpleLayout() {
    if (m_struct == 2) {
        // stack vertical: top at START_X, START_Y
        int x = START_X;
        int y = START_Y;
        // draw from top (last pushed at top end)
        for (int i = m_order.size() - 1; i >= 0; --i) {
            int v = m_order[i];
            if (!m_items.contains(v)) continue;
            m_items[v].rect->setPos(x, y);
            y += NODE_H + GAP;
        }
    }
    else {
        // horizontal layout left to right in insertion order
        int x = START_X;
        int y = START_Y;
        for (int v : m_order) {
            if (!m_items.contains(v)) continue;
            m_items[v].rect->setPos(x, y);
            x += NODE_W + GAP;
        }
    }
    rebuildArrows();
}

void LinearListScene::layoutNodesWithAnimation() {
    // animate every node from current pos to target pos
    // compute target positions
    QMap<int, QPointF> targetPos;
    if (m_struct == 2) {
        // stack: top at START_X/START_Y
        int x = START_X;
        int y = START_Y;
        for (int i = m_order.size() - 1; i >= 0; --i) {
            int v = m_order[i];
            targetPos[v] = QPointF(x, y);
            y += NODE_H + GAP;
        }
    }
    else {
        int x = START_X;
        int y = START_Y;
        for (int v : m_order) {
            targetPos[v] = QPointF(x, y);
            x += NODE_W + GAP;
        }
    }

    // For any items not existing in m_items (shouldn't happen) skip
    // Animate: for each item, create QVariantAnimation updating pos
    const int duration = 400;
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        int v = it.key();
        auto rect = it->rect;
        if (!rect) continue;
        QPointF from = rect->pos();
        QPointF to = targetPos.contains(v) ? targetPos[v] : from;
        if (from == to) continue;
        QVariantAnimation* anim = new QVariantAnimation(this);
        anim->setDuration(duration);
        anim->setStartValue(from);
        anim->setEndValue(to);
        connect(anim, &QVariantAnimation::valueChanged, this, [rect](const QVariant& val) {
            QPointF p = val.toPointF();
            rect->setPos(p);
            });
        connect(anim, &QVariantAnimation::finished, anim, &QObject::deleteLater);
        anim->start();
    }

    // After short delay, rebuild arrows (we can delay by same duration)
    QTimer::singleShot(duration + 30, this, [this]() { rebuildArrows(); });
}
