#include "linearlistscene.h"
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QVariantAnimation>
#include <QTimer>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <QtMath>

LinearListScene::LinearListScene(QObject* parent) : BaseScene(parent) {
    setSceneRect(0, 0, 2000, 1200);
}

void LinearListScene::setStructureType(StructureType type) {
    reset();
    m_type = type;
    update();
}

void LinearListScene::reset() {
    cleanAllGraphics();
    m_dataList.clear();
    if (m_probeRect) {
        removeItem(m_probeRect);
        delete m_probeRect;
        m_probeRect = nullptr;
    }
    update();
}

void LinearListScene::cleanAllGraphics() {
    for (auto* node : m_visualNodes) {
        if (node->rect) { removeItem(node->rect); delete node->rect; }
        if (node->arrow) { removeItem(node->arrow); delete node->arrow; }
        delete node;
    }
    m_visualNodes.clear();
}

QPointF LinearListScene::getNodePos(int index) {
    if (m_type == STACK) {
        return QPointF(START_X + 200, START_Y_STACK - index * (NODE_H + 5));
    }
    else {
        return QPointF(START_X + index * (NODE_W + GAP), START_Y_LIST);
    }
}

void LinearListScene::createProbe() {
    if (m_probeRect) return;
    m_probeRect = addRect(0, 0, NODE_W + 10, NODE_H + 10, QPen(Qt::red, 3));
    m_probeRect->setZValue(999);
    m_probeRect->setVisible(false);
}

void LinearListScene::animStepSearch(int targetIndex, std::function<void()> onFinished) {
    if (m_type == STACK || targetIndex < 0) {
        onFinished();
        return;
    }
    createProbe();
    m_probeRect->setVisible(true);
    QPointF startPos = getNodePos(0) - QPointF(5, 5);
    m_probeRect->setPos(startPos);

    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);
    int steps = m_dataList.isEmpty() ? 0 : qMin(targetIndex, m_dataList.size());

    for (int i = 0; i <= steps; ++i) {
        QPointF targetPos = getNodePos(i) - QPointF(5, 5);
        QPointF stepStart = (i == 0) ? targetPos : (getNodePos(i - 1) - QPointF(5, 5));
        QVariantAnimation* anim = new QVariantAnimation(group);
        anim->setDuration(400);
        anim->setStartValue(stepStart);
        anim->setEndValue(targetPos);
        anim->setEasingCurve(QEasingCurve::InOutQuad);
        connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant& val) {
            if (m_probeRect) m_probeRect->setPos(val.toPointF());
            });
        group->addAnimation(anim);
        if (i < steps) group->addPause(150);
    }
    connect(group, &QAbstractAnimation::finished, this, [this, onFinished, group]() {
        onFinished();
        group->deleteLater();
        });
    group->start();
}

// === 修复核心：改用 QColor 进行动画插值 ===

void LinearListScene::highlightNode(int value, std::function<void()> onFinished) {
    highlightNode(value, QColor(100, 255, 100), true, onFinished);
}

void LinearListScene::highlightNode(int value, QColor color, bool autoRestore, std::function<void()> onFinished) {
    if (!m_visualNodes.contains(value)) {
        if (onFinished) onFinished();
        return;
    }

    NodeGraphics* ng = m_visualNodes[value];

    // 【关键修改】提取 Color 而不是 Brush
    QColor originalColor = ng->rect->brush().color();
    QColor targetColor = color;

    QVariantAnimation* anim = new QVariantAnimation(this);
    anim->setDuration(800);

    if (autoRestore) {
        // 闪烁：原 -> 目标 -> 原
        anim->setKeyValueAt(0.0, originalColor);
        anim->setKeyValueAt(0.2, targetColor);
        anim->setKeyValueAt(0.8, targetColor);
        anim->setKeyValueAt(1.0, originalColor);
    }
    else {
        // 变色保持：原 -> 目标 (给删除用，变红后不回去)
        anim->setKeyValueAt(0.0, originalColor);
        anim->setKeyValueAt(0.2, targetColor);
        anim->setKeyValueAt(1.0, targetColor);
    }

    // 【关键修改】在回调里重新包装 QBrush(color)
    connect(anim, &QVariantAnimation::valueChanged, this, [ng](const QVariant& val) {
        if (ng && ng->rect) {
            ng->rect->setBrush(QBrush(val.value<QColor>()));
        }
        });

    connect(anim, &QVariantAnimation::finished, this, [this, onFinished]() {
        if (m_probeRect) m_probeRect->setVisible(false);
        if (onFinished) onFinished();
        });

    anim->start();
}

// === 外部接口 ===

void LinearListScene::searchNodeAnimated(int value, int index) {
    animStepSearch(index, [this, value]() {
        highlightNode(value, [this]() { emit animationFinished(); });
        });
}

void LinearListScene::insertNodeAnimated(int value, int index) {
    animStepSearch(index, [this, value, index]() {
        NodeGraphics* ng = new NodeGraphics();
        QPen pen(Qt::black);
        QBrush brush(Qt::yellow);
        if (m_type == ARRAY_LIST) brush.setColor(QColor(173, 216, 230));
        if (m_type == STACK) brush.setColor(QColor(255, 182, 193));

        ng->rect = addRect(0, 0, NODE_W, NODE_H, pen, brush);
        ng->value = value;

        QPointF finalPos = getNodePos(index);
        QPointF startPos = finalPos - QPointF(0, 100);

        ng->rect->setPos(startPos);
        ng->rect->setOpacity(0);

        ng->text = new QGraphicsSimpleTextItem(QString::number(value), ng->rect);
        auto b = ng->text->boundingRect();
        ng->text->setPos((NODE_W - b.width()) / 2, (NODE_H - b.height()) / 2);

        if (m_type == LINKED_LIST) {
            QGraphicsLineItem* split = new QGraphicsLineItem(NODE_W * 0.7, 0, NODE_W * 0.7, NODE_H, ng->rect);
            (void)split;
        }

        m_visualNodes[value] = ng;
        m_dataList.insert(index, value);

        if (m_probeRect) m_probeRect->setVisible(false);

        QVariantAnimation* vAnim = new QVariantAnimation(this);
        vAnim->setDuration(600);
        vAnim->setStartValue(startPos); vAnim->setEndValue(finalPos);
        vAnim->setEasingCurve(QEasingCurve::OutBounce);

        connect(vAnim, &QVariantAnimation::valueChanged, this, [this, ng](const QVariant& val) {
            if (ng && ng->rect) { ng->rect->setPos(val.toPointF()); ng->rect->setOpacity(1.0); }
            });

        connect(vAnim, &QVariantAnimation::finished, this, [this]() {
            updateArrows();
            if (m_type == ARRAY_LIST) {
                for (int i = 0; i < m_dataList.size(); ++i) {
                    int v = m_dataList[i];
                    if (m_visualNodes.contains(v)) m_visualNodes[v]->rect->setPos(getNodePos(i));
                }
            }
            emit animationFinished();
            });
        vAnim->start();
        });
}

void LinearListScene::removeNodeAnimated(int value, int index) {
    // 1. 探针
    animStepSearch(index, [this, value, index]() {
        if (!m_visualNodes.contains(value)) {
            if (m_probeRect) m_probeRect->setVisible(false);
            emit animationFinished();
            return;
        }

        // 2. 变红 (现在肯定能看见变红了)
        highlightNode(value, Qt::red, false, [this, value, index]() {

            // 3. 淡出
            NodeGraphics* ng = m_visualNodes[value];
            QVariantAnimation* fadeAnim = new QVariantAnimation(this);
            fadeAnim->setDuration(600);
            fadeAnim->setStartValue(1.0);
            fadeAnim->setEndValue(0.0);
            QPointF startPos = ng->rect->pos();

            connect(fadeAnim, &QVariantAnimation::valueChanged, this, [this, ng, startPos](const QVariant& val) {
                if (ng && ng->rect) {
                    qreal progress = val.toReal();
                    ng->rect->setOpacity(progress);
                    if (m_type == STACK) ng->rect->setPos(startPos.x(), startPos.y() - (1.0 - progress) * 50);
                    else { ng->rect->setScale(progress); ng->rect->setTransformOriginPoint(NODE_W / 2, NODE_H / 2); }
                }
                });

            connect(fadeAnim, &QVariantAnimation::finished, this, [this, value, index, ng]() {
                if (ng->rect) delete ng->rect;
                if (ng->arrow) delete ng->arrow;
                delete ng;
                m_visualNodes.remove(value);
                m_dataList.removeAt(index);

                if (m_type == ARRAY_LIST || m_type == LINKED_LIST) {
                    for (int i = 0; i < m_dataList.size(); ++i) {
                        int v = m_dataList[i];
                        if (m_visualNodes.contains(v)) {
                            m_visualNodes[v]->rect->setPos(getNodePos(i));
                            m_visualNodes[v]->rect->setScale(1.0);
                            m_visualNodes[v]->rect->setOpacity(1.0);
                        }
                    }
                }
                updateArrows();
                emit animationFinished();
                });
            fadeAnim->start();
            });
        });
}

void LinearListScene::updateArrows() {
    for (auto* node : m_visualNodes) { if (node->arrow) { delete node->arrow; node->arrow = nullptr; } }
    if (m_type != LINKED_LIST) return;
    for (int i = 0; i < m_dataList.size() - 1; ++i) {
        int v1 = m_dataList[i];
        int v2 = m_dataList[i + 1];
        if (!m_visualNodes.contains(v1) || !m_visualNodes.contains(v2)) continue;
        QPointF start = m_visualNodes[v1]->rect->pos() + QPointF(NODE_W, NODE_H / 2);
        QPointF end = m_visualNodes[v2]->rect->pos() + QPointF(0, NODE_H / 2);
        QGraphicsLineItem* line = addLine(QLineF(start, end), QPen(Qt::black, 2));
        line->setZValue(-1);
        m_visualNodes[v1]->arrow = line;
    }
}