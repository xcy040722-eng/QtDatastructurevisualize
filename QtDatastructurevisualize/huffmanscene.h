#pragma once
#include "basescene.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsLineItem>
#include <QList>

// 哈夫曼节点
struct HuffNode {
    int weight; // 权重
    HuffNode* left = nullptr;
    HuffNode* right = nullptr;

    // 视觉元素
    QGraphicsEllipseItem* circle = nullptr;
    QGraphicsSimpleTextItem* text = nullptr;
    QGraphicsLineItem* linkLeft = nullptr;
    QGraphicsLineItem* linkRight = nullptr;

    // 布局目标
    qreal targetX = 0;
    qreal targetY = 0;
    // 树的宽度（用于布局计算）
    qreal treeWidth = 0;

    explicit HuffNode(int w) : weight(w) {}
};

class HuffmanScene : public BaseScene {
    Q_OBJECT
public:
    explicit HuffmanScene(QObject* parent = nullptr);
    ~HuffmanScene() override;

    void reset() override;

    // 重写接口
    // 哈夫曼模式下：insertNode 用于添加叶子节点
    void insertNodeAnimated(int value, int index) override;
    // 哈夫曼模式下：removeNode 用于“执行一步构建” (Build Step)
    // 我们复用 remove 接口作为“合并最小两个节点”的触发器
    void removeNodeAnimated(int value, int index) override;

    // 查找接口暂时不用
    void searchNodeAnimated(int value, int index) override {}

private:
    // 森林：存储所有当前的树根
    QList<HuffNode*> m_forest;

    // 辅助：高亮环（用于指示当前选中的最小两个）
    QGraphicsEllipseItem* m_highlight1 = nullptr;
    QGraphicsEllipseItem* m_highlight2 = nullptr;

    // 布局常量
    const int NODE_RADIUS = 25;
    const int LEVEL_HEIGHT = 80;
    const int GAP_X = 20; // 树之间的间隙

    // === 内部逻辑 ===
    void cleanNodeRecursive(HuffNode* node);

    // 1. 计算单棵树的宽度 (递归)
    qreal calculateTreeWidth(HuffNode* node);

    // 2. 计算单棵树内部节点的相对坐标 (递归)
    void calculateSubTreeLayout(HuffNode* node, qreal x, qreal y, qreal hOffset);

    // 3. 全局布局：排列森林中的所有树
    void layoutForest();

    // 4. 刷新视觉：移动所有节点到 targetX/Y
    void refreshVisuals();

    // 辅助：创建可视节点
    void createVisualNode(HuffNode* node, qreal x, qreal y);

    // 执行一步合并动画
    void mergeStep();
};