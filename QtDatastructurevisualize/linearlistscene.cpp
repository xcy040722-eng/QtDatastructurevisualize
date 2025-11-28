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
    setSceneRect(0, 0, 1200, 800);
}

void LinearListScene::setStructureType(StructureType type) {
    qDebug() << "[Scene] Setting structure type to:" << type;
    reset(); // 切换类型前必须重置
    m_type = type;
    update();
}

void LinearListScene::reset() {
    qDebug() << "[Scene] Resetting...";
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
        // 栈：垂直向上生长
        return QPointF(START_X + 200, START_Y + 300 - index * (NODE_H + 5));
    }
    else {
        // 链表/顺序表：水平排列
        return QPointF(START_X + index * (NODE_W + GAP), START_Y);
    }
}

void LinearListScene::createProbe() {
    if (m_probeRect) return;
    m_probeRect = addRect(0, 0, NODE_W + 10, NODE_H + 10, QPen(Qt::red, 3));
    m_probeRect->setZValue(999);
    m_probeRect->setVisible(false);
}

// === 动画步骤 1: 查找 ===
void LinearListScene::animStepSearch(int targetIndex, std::function<void()> onFinished) {
    if (m_type == STACK || targetIndex < 0) {
        onFinished();
        return;
    }

    qDebug() << "[Scene] Starting search animation to index:" << targetIndex;
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

        // 关键点：[this] 捕获，修复编译错误
        connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant& val) {
            if (m_probeRect) m_probeRect->setPos(val.toPointF());
            });

        group->addAnimation(anim);
        if (i < steps) group->addPause(100);
    }

    connect(group, &QAbstractAnimation::finished, this, [this, onFinished, group]() {
        qDebug() << "[Scene] Search finished";
        if (m_probeRect) m_probeRect->setVisible(false);
        onFinished();
        group->deleteLater();
        });

    group->start();
}

// === 动画步骤 2: 插入 ===
void LinearListScene::insertNodeAnimated(int value, int index) {
    qDebug() << "[Scene] insertNodeAnimated called. Value:" << value << " Index:" << index;

    animStepSearch(index, [this, value, index]() {

        NodeGraphics* ng = new NodeGraphics();
        QPen pen(Qt::black);
        QBrush brush(Qt::yellow);

        if (m_type == ARRAY_LIST) brush.setColor(QColor(200, 230, 255));
        if (m_type == STACK) brush.setColor(QColor(180, 255, 180));

        ng->rect = addRect(0, 0, NODE_W, NODE_H, pen, brush);
        ng->value = value;

        QPointF finalPos = getNodePos(index);
        QPointF startPos = finalPos - QPointF(0, 150);

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

        QVariantAnimation* vAnim = new QVariantAnimation(this);
        vAnim->setDuration(600);
        vAnim->setStartValue(startPos);
        vAnim->setEndValue(finalPos);
        vAnim->setEasingCurve(QEasingCurve::OutBounce);

        // 关键点：[this] 捕获
        connect(vAnim, &QVariantAnimation::valueChanged, this, [this, ng](const QVariant& val) {
            if (ng && ng->rect) {
                ng->rect->setPos(val.toPointF());
                ng->rect->setOpacity(1.0);
            }
            });

        connect(vAnim, &QVariantAnimation::finished, this, [this]() {
            updateArrows();

            if (m_type == ARRAY_LIST) {
                for (int i = 0; i < m_dataList.size(); ++i) {
                    int v = m_dataList[i];
                    if (m_visualNodes.contains(v)) {
                        m_visualNodes[v]->rect->setPos(getNodePos(i));
                    }
                }
            }
            emit animationFinished();
            });

        vAnim->start();
        });
}

void LinearListScene::removeNodeAnimated(int value, int index) {
    qDebug() << "[Scene] removeNodeAnimated called. Value:" << value;

    animStepSearch(index, [this, value, index]() {
        if (!m_visualNodes.contains(value)) {
            emit animationFinished();
            return;
        }

        NodeGraphics* ng = m_visualNodes[value];

        QVariantAnimation* vAnim = new QVariantAnimation(this);
        vAnim->setDuration(500);
        vAnim->setStartValue(1.0);
        vAnim->setEndValue(0.0);

        connect(vAnim, &QVariantAnimation::valueChanged, this, [this, ng](const QVariant& val) {
            if (ng && ng->rect) {
                qreal s = val.toReal();
                ng->rect->setOpacity(s);
                ng->rect->setScale(s);
                ng->rect->setTransformOriginPoint(NODE_W / 2, NODE_H / 2);
            }
            });

        connect(vAnim, &QVariantAnimation::finished, this, [this, value, index, ng]() {
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
        vAnim->start();
        });
}

void LinearListScene::updateArrows() {
    for (auto* node : m_visualNodes) {
        if (node->arrow) { delete node->arrow; node->arrow = nullptr; }
    }

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