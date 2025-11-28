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

    // 重写基类方法，实现具体动画
    void insertNodeAnimated(int value, int index) override;
    void removeNodeAnimated(int value, int index) override;
    // 关键修复：声明查找动画
    void searchNodeAnimated(int value, int index) override;

private:
    StructureType m_type = LINKED_LIST;
    QList<int> m_dataList; // 数据副本，用于计算位置
    QMap<int, NodeGraphics*> m_visualNodes; // 图形映射

    // 辅助图形：红色探针
    QGraphicsRectItem* m_probeRect = nullptr;

    // 布局常量 (调整得更紧凑，方便查看)
    const int START_X = 40;     // 原 60 -> 40
    const int START_Y_LIST = 80; // 原 150 -> 80
    const int START_Y_STACK = 500;
    const int NODE_W = 60;
    const int NODE_H = 40;
    const int GAP = 50;

    // 内部助手函数
    void cleanAllGraphics();
    QPointF getNodePos(int index);
    void updateArrows();
    void createProbe();

    // 动画步骤
    void animStepSearch(int targetIndex, std::function<void()> onFinished);

    // 新增：高亮节点动画
    void highlightNode(int value, std::function<void()> onFinished);
};