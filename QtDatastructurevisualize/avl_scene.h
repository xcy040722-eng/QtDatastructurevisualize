#pragma once
#include "basescene.h"
#include "avl_defs.h"
#include <QQueue>
#include <QMap>
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsLineItem>

struct VisualNode {
    QGraphicsEllipseItem* circle = nullptr;
    QGraphicsSimpleTextItem* text = nullptr;
    QGraphicsLineItem* linkToParent = nullptr;
    int parentId = -1;
};

class AVLScene : public BaseScene {
    Q_OBJECT
public:
    explicit AVLScene(QObject* parent = nullptr);
    ~AVLScene() override;

    void reset() override;
    void executeCommands(const QQueue<VisualCommand>& cmds);

    // 适配接口
    void insertNodeAnimated(int value, int index) override {}
    void removeNodeAnimated(int value, int index) override {}
    void searchNodeAnimated(int value, int index) override {}
    void traverseAnimated(int type) override {}

private:
    QQueue<VisualCommand> m_cmdQueue;
    QMap<int, VisualNode> m_nodes;
    bool m_isAnimating;

    QGraphicsEllipseItem* m_probeHalo = nullptr;
    QGraphicsSimpleTextItem* m_resultText = nullptr; // === 新增：显示遍历结果 ===

    const int RADIUS = 25;

    void processNextCommand();

    void cmdCreateNode(int id, QPointF pos);
    void cmdMoveNode(int id, QPointF pos, int duration);
    void cmdSetParent(int childId, int parentId);
    void cmdHighlight(int id, QColor color, int duration);
    void cmdRemoveNode(int id);
    void cmdSearchHighlight(int id);
    void cmdUpdateResultText(const QString& text); // === 新增 ===
    void cmdWait(int duration);

    void updateRelatedLines(int nodeId);
    void createProbeHalo();
    void createResultText(); // 辅助
};