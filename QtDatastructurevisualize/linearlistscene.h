#pragma once
#include "basescene.h"
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsLineItem>
#include <QMap>
#include <QList>
#include <functional>

// 节点图形组合结构体
struct NodeGraphics {
    QGraphicsRectItem* rect = nullptr;
    QGraphicsSimpleTextItem* text = nullptr;
    QGraphicsLineItem* arrow = nullptr;
    int value = 0;
};

class LinearListScene : public BaseScene {
    Q_OBJECT
public:
    enum StructureType { LINKED_LIST = 0, ARRAY_LIST = 1, STACK = 2 };

    explicit LinearListScene(QObject* parent = nullptr);

    void setStructureType(StructureType type);
    void reset() override;

    void insertNodeAnimated(int value, int index) override;
    void removeNodeAnimated(int value, int index) override;
    void searchNodeAnimated(int value, int index) override;

private:
    StructureType m_type = LINKED_LIST;
    QList<int> m_dataList;
    QMap<int, NodeGraphics*> m_visualNodes;

    QGraphicsRectItem* m_probeRect = nullptr;

    const int START_X = 40;
    const int START_Y_LIST = 80;
    const int START_Y_STACK = 500;
    const int NODE_W = 60;
    const int NODE_H = 40;
    const int GAP = 50;

    void cleanAllGraphics();
    QPointF getNodePos(int index);
    void updateArrows();
    void createProbe();

    // 探针移动动画
    void animStepSearch(int targetIndex, std::function<void()> onFinished);

    // === 统一高亮接口 ===
    // 重载1：简便调用，默认绿色，自动恢复（查找用）
    void highlightNode(int value, std::function<void()> onFinished);

    // 重载2：完整调用，支持自定义颜色和是否恢复（删除用）
    void highlightNode(int value, QColor color, bool autoRestore, std::function<void()> onFinished);
};