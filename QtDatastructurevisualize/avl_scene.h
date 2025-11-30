#pragma once
#include "basescene.h"
#include "avl_defs.h"
#include <QQueue>
#include <QMap>
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsLineItem>

// 视觉节点结构：只包含图形元素
struct VisualNode {
    QGraphicsEllipseItem* circle = nullptr;
    QGraphicsSimpleTextItem* text = nullptr;
    QGraphicsLineItem* linkToParent = nullptr; // 指向父节点的连线
    int parentId = -1; // 记录父节点ID，用于移动时更新连线
};

class AVLScene : public BaseScene {
    Q_OBJECT
public:
    explicit AVLScene(QObject* parent = nullptr);
    ~AVLScene() override;

    void reset() override;

    // === 核心接口：接收并执行指令包 ===
    void executeCommands(const QQueue<VisualCommand>& cmds);

    // 必须实现的基类接口 (转接或空实现)
    void insertNodeAnimated(int value, int index) override {}
    void removeNodeAnimated(int value, int index) override {}
    void searchNodeAnimated(int value, int index) override {}
    void traverseAnimated(int type) override {}

private:
    QQueue<VisualCommand> m_cmdQueue;
    QMap<int, VisualNode> m_nodes; // value -> visual components
    bool m_isAnimating;

    const int RADIUS = 25;

    // 调度器
    void processNextCommand();

    // 具体指令执行函数
    void cmdCreateNode(int id, QPointF pos);
    void cmdMoveNode(int id, QPointF pos, int duration);
    void cmdSetParent(int childId, int parentId);
    void cmdHighlight(int id, QColor color, int duration);
    void cmdWait(int duration);

    // 辅助：更新与某节点相关的所有连线 (自身连父 + 子连自身)
    void updateRelatedLines(int nodeId);
};