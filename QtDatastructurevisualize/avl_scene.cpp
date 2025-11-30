#include "avl_scene.h"
#include <QTimer>
#include <QVariantAnimation>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QDebug>

AVLScene::AVLScene(QObject* parent) : BaseScene(parent), m_isAnimating(false) {
    setSceneRect(0, 0, 2000, 1500);
}

AVLScene::~AVLScene() {
    reset();
}

void AVLScene::reset() {
    m_cmdQueue.clear();
    m_nodes.clear();
    clear(); // 清空 QGraphicsScene
    m_isAnimating = false;
}

void AVLScene::executeCommands(const QQueue<VisualCommand>& cmds) {
    m_cmdQueue.append(cmds);
    // 如果当前没有动画在跑，立即启动
    if (!m_isAnimating) {
        processNextCommand();
    }
}

void AVLScene::processNextCommand() {
    if (m_cmdQueue.isEmpty()) {
        m_isAnimating = false;
        emit animationFinished();
        return;
    }

    m_isAnimating = true;
    VisualCommand cmd = m_cmdQueue.dequeue();

    switch (cmd.type) {
    case CommandType::CreateNode:
        cmdCreateNode(cmd.nodeId, cmd.pos);
        // Create 很快，不需要等待，直接下一条(除非想做淡入)
        // 这里为了稳妥，我们在 cmdCreateNode 内部如果有动画再处理，
        // 或者简单处理：创建完直接 next
        processNextCommand();
        break;
    case CommandType::MoveNode:
        cmdMoveNode(cmd.nodeId, cmd.pos, cmd.duration);
        break;
    case CommandType::SetParent:
        cmdSetParent(cmd.nodeId, cmd.relatedId);
        processNextCommand();
        break;
    case CommandType::HighlightNode:
        cmdHighlight(cmd.nodeId, cmd.color, cmd.duration);
        break;
    case CommandType::Wait:
        cmdWait(cmd.duration);
        break;
    default:
        processNextCommand();
        break;
    }
}

void AVLScene::cmdCreateNode(int id, QPointF pos) {
    if (m_nodes.contains(id)) return;

    VisualNode vn;
    // 如果逻辑层传的是空坐标(0,0)，给个默认出生点
    QPointF spawnPos = (pos.isNull()) ? QPointF(500, 0) : pos;

    vn.circle = addEllipse(0, 0, RADIUS * 2, RADIUS * 2, QPen(Qt::black, 2), QBrush(QColor(144, 238, 144)));
    vn.circle->setPos(spawnPos - QPointF(RADIUS, RADIUS));
    vn.circle->setZValue(10);

    vn.text = new QGraphicsSimpleTextItem(QString::number(id), vn.circle);
    auto b = vn.text->boundingRect();
    vn.text->setPos((RADIUS * 2 - b.width()) / 2, (RADIUS * 2 - b.height()) / 2);

    // 简单的出现效果
    vn.circle->setOpacity(1.0);

    m_nodes[id] = vn;
}

void AVLScene::cmdMoveNode(int id, QPointF targetPos, int duration) {
    if (!m_nodes.contains(id)) { processNextCommand(); return; }

    VisualNode& vn = m_nodes[id];
    QPointF startPos = vn.circle->pos();
    QPointF endPos = targetPos - QPointF(RADIUS, RADIUS);

    // 如果位置没变，直接下一条
    if ((startPos - endPos).manhattanLength() < 1.0) {
        processNextCommand();
        return;
    }

    QVariantAnimation* anim = new QVariantAnimation(this);
    anim->setDuration(duration);
    anim->setStartValue(startPos);
    anim->setEndValue(endPos);
    anim->setEasingCurve(QEasingCurve::InOutQuad); // 平滑移动

    connect(anim, &QVariantAnimation::valueChanged, this, [this, id](const QVariant& val) {
        if (m_nodes.contains(id)) {
            m_nodes[id].circle->setPos(val.toPointF());
            // 【关键】移动过程中实时更新连线
            updateRelatedLines(id);
        }
        });

    connect(anim, &QVariantAnimation::finished, this, [this]() {
        processNextCommand();
        });

    anim->start();
}

void AVLScene::cmdSetParent(int childId, int parentId) {
    if (!m_nodes.contains(childId)) return;
    VisualNode& child = m_nodes[childId];

    // 如果已经有线，先删除
    if (child.linkToParent) {
        removeItem(child.linkToParent);
        delete child.linkToParent;
        child.linkToParent = nullptr;
    }

    child.parentId = parentId;

    if (m_nodes.contains(parentId)) {
        VisualNode& parent = m_nodes[parentId];
        QPointF cPos = child.circle->pos() + QPointF(RADIUS, RADIUS);
        QPointF pPos = parent.circle->pos() + QPointF(RADIUS, RADIUS);

        child.linkToParent = addLine(QLineF(cPos, pPos), QPen(Qt::black, 2));
        child.linkToParent->setZValue(0); // 线在下层
    }
}

void AVLScene::updateRelatedLines(int nodeId) {
    // 1. 更新我指向父节点的线
    if (m_nodes.contains(nodeId)) {
        VisualNode& vn = m_nodes[nodeId];
        if (vn.linkToParent && m_nodes.contains(vn.parentId)) {
            QPointF myCenter = vn.circle->pos() + QPointF(RADIUS, RADIUS);
            QPointF pCenter = m_nodes[vn.parentId].circle->pos() + QPointF(RADIUS, RADIUS);
            vn.linkToParent->setLine(QLineF(myCenter, pCenter));
        }
    }

    // 2. 更新指向我的子节点的线 (即我是别人的父节点)
    // 效率优化：因为节点少，遍历 map 没问题
    QPointF myCenter = m_nodes[nodeId].circle->pos() + QPointF(RADIUS, RADIUS);
    for (auto& vn : m_nodes) {
        if (vn.parentId == nodeId && vn.linkToParent) {
            QPointF cCenter = vn.circle->pos() + QPointF(RADIUS, RADIUS);
            vn.linkToParent->setLine(QLineF(cCenter, myCenter));
        }
    }
}

void AVLScene::cmdHighlight(int id, QColor color, int duration) {
    if (m_nodes.contains(id)) {
        m_nodes[id].circle->setBrush(color);
    }

    // 如果有 duration，等待后再继续；否则立即继续
    if (duration > 0) {
        QTimer::singleShot(duration, this, &AVLScene::processNextCommand);
    }
    else {
        processNextCommand();
    }
}

void AVLScene::cmdWait(int duration) {
    QTimer::singleShot(duration, this, &AVLScene::processNextCommand);
}