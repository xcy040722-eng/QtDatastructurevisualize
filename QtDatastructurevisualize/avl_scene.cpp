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
    clear();
    m_isAnimating = false;
    m_probeHalo = nullptr;
    m_resultText = nullptr;
}

void AVLScene::executeCommands(const QQueue<VisualCommand>& cmds) {
    m_cmdQueue.append(cmds);
    if (!m_isAnimating) processNextCommand();
}

void AVLScene::processNextCommand() {
    if (m_cmdQueue.isEmpty()) {
        m_isAnimating = false;
        if (m_probeHalo) m_probeHalo->setVisible(false);
        emit animationFinished();
        return;
    }

    m_isAnimating = true;
    VisualCommand cmd = m_cmdQueue.dequeue();

    switch (cmd.type) {
    case CommandType::CreateNode:
        cmdCreateNode(cmd.nodeId, cmd.pos);
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
    case CommandType::RemoveNode:
        cmdRemoveNode(cmd.nodeId);
        break;
    case CommandType::SearchHighlight:
        cmdSearchHighlight(cmd.nodeId);
        // 探针自带 duration，使用 Timer 衔接
        QTimer::singleShot(cmd.duration, this, &AVLScene::processNextCommand);
        break;
    case CommandType::UpdateResultText: // === 新增 ===
        cmdUpdateResultText(cmd.text);
        processNextCommand();
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
    QPointF spawnPos = (pos.isNull()) ? QPointF(500, 0) : pos;

    vn.circle = addEllipse(0, 0, RADIUS * 2, RADIUS * 2, QPen(Qt::black, 2), QBrush(QColor(144, 238, 144)));
    vn.circle->setPos(spawnPos - QPointF(RADIUS, RADIUS));
    vn.circle->setZValue(10);

    vn.text = new QGraphicsSimpleTextItem(QString::number(id), vn.circle);
    auto b = vn.text->boundingRect();
    vn.text->setPos((RADIUS * 2 - b.width()) / 2, (RADIUS * 2 - b.height()) / 2);

    m_nodes[id] = vn;
}

void AVLScene::cmdMoveNode(int id, QPointF targetPos, int duration) {
    if (!m_nodes.contains(id)) { processNextCommand(); return; }

    VisualNode& vn = m_nodes[id];
    QPointF startPos = vn.circle->pos();
    QPointF endPos = targetPos - QPointF(RADIUS, RADIUS);

    if ((startPos - endPos).manhattanLength() < 1.0) {
        processNextCommand();
        return;
    }

    QVariantAnimation* anim = new QVariantAnimation(this);
    anim->setDuration(duration);
    anim->setStartValue(startPos);
    anim->setEndValue(endPos);
    anim->setEasingCurve(QEasingCurve::InOutQuad);

    connect(anim, &QVariantAnimation::valueChanged, this, [this, id](const QVariant& val) {
        if (m_nodes.contains(id)) {
            m_nodes[id].circle->setPos(val.toPointF());
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
        child.linkToParent->setZValue(0);
    }
}

void AVLScene::cmdHighlight(int id, QColor color, int duration) {
    if (m_nodes.contains(id)) {
        m_nodes[id].circle->setBrush(color);
        // 如果是绿色（成功找到），同步探针位置
        if (color == Qt::green && m_probeHalo) {
            m_probeHalo->setPos(m_nodes[id].circle->pos() - QPointF(5, 5));
            m_probeHalo->setVisible(true);
        }
    }
    if (duration > 0) QTimer::singleShot(duration, this, &AVLScene::processNextCommand);
    else processNextCommand();
}

void AVLScene::cmdRemoveNode(int id) {
    if (!m_nodes.contains(id)) {
        processNextCommand();
        return;
    }

    VisualNode vn = m_nodes[id];
    m_nodes.remove(id); // 立即从 map 移除

    QVariantAnimation* anim = new QVariantAnimation(this);
    anim->setDuration(600);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);

    QGraphicsItem* c = vn.circle;
    QGraphicsItem* l = vn.linkToParent;
    QGraphicsItem* t = vn.text;

    connect(anim, &QVariantAnimation::valueChanged, this, [c, l, t](const QVariant& val) {
        if (c) c->setOpacity(val.toFloat());
        if (l) l->setOpacity(val.toFloat());
        if (t) t->setOpacity(val.toFloat());
        });

    connect(anim, &QVariantAnimation::finished, this, [this, c, l, t]() {
        if (c) { removeItem(c); delete c; }
        if (l) { removeItem(l); delete l; }
        // text 是 circle 的子项，通常不需要单独删，但为了保险起见
        processNextCommand();
        });
    anim->start();
}

void AVLScene::createProbeHalo() {
    if (!m_probeHalo) {
        m_probeHalo = addEllipse(0, 0, RADIUS * 2 + 10, RADIUS * 2 + 10, QPen(Qt::red, 4), Qt::NoBrush);
        m_probeHalo->setZValue(999);
        m_probeHalo->setVisible(false);
    }
}

void AVLScene::cmdSearchHighlight(int id) {
    createProbeHalo();
    if (m_nodes.contains(id)) {
        m_probeHalo->setPos(m_nodes[id].circle->pos() - QPointF(5, 5));
        m_probeHalo->setVisible(true);
    }
}

// === 新增：更新结果文字 ===
void AVLScene::createResultText() {
    if (!m_resultText) {
        m_resultText = new QGraphicsSimpleTextItem();
        QFont font;
        font.setPointSize(14);
        font.setBold(true);
        m_resultText->setFont(font);
        m_resultText->setBrush(Qt::blue);
        m_resultText->setPos(20, 20); // 左上角
        addItem(m_resultText);
    }
}

void AVLScene::cmdUpdateResultText(const QString& text) {
    createResultText();
    m_resultText->setText(text);
}

void AVLScene::cmdWait(int duration) {
    QTimer::singleShot(duration, this, &AVLScene::processNextCommand);
}

void AVLScene::updateRelatedLines(int nodeId) {
    if (m_nodes.contains(nodeId)) {
        VisualNode& vn = m_nodes[nodeId];
        if (vn.linkToParent && m_nodes.contains(vn.parentId)) {
            QPointF myCenter = vn.circle->pos() + QPointF(RADIUS, RADIUS);
            QPointF pCenter = m_nodes[vn.parentId].circle->pos() + QPointF(RADIUS, RADIUS);
            vn.linkToParent->setLine(QLineF(myCenter, pCenter));
        }
    }
    QPointF myCenter = m_nodes[nodeId].circle->pos() + QPointF(RADIUS, RADIUS);
    for (auto& vn : m_nodes) {
        if (vn.parentId == nodeId && vn.linkToParent) {
            QPointF cCenter = vn.circle->pos() + QPointF(RADIUS, RADIUS);
            vn.linkToParent->setLine(QLineF(cCenter, myCenter));
        }
    }
}