#pragma once
#include "basescene.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsLineItem>
#include <functional>
#include <QList>

// 树节点结构体：包含数据指针和视觉元素指针
struct TreeNode {
    int value;
    TreeNode* left = nullptr;
    TreeNode* right = nullptr;

    // 视觉元素
    QGraphicsEllipseItem* circle = nullptr;  // 圆形节点
    QGraphicsSimpleTextItem* text = nullptr; // 数值文本
    QGraphicsLineItem* linkToParent = nullptr; // 指向父节点的连线

    // 布局目标位置 (用于动画插值)
    int targetX = 0;
    int targetY = 0;

    explicit TreeNode(int v) : value(v) {}
};

class TreeScene : public BaseScene {
    Q_OBJECT
public:
    explicit TreeScene(QObject* parent = nullptr);
    ~TreeScene() override;

    void reset() override;

    // === 重写基类接口 ===
    void insertNodeAnimated(int value, int index) override;
    void removeNodeAnimated(int value, int index) override;
    void searchNodeAnimated(int value, int index) override;

private:
    TreeNode* root = nullptr;
    QGraphicsEllipseItem* m_probeHalo = nullptr; // 红色探针

    // 待删除的图形项暂存区 (用于播放淡出动画)
    QList<QGraphicsItem*> m_trashItems;

    // === 布局常量 ===
    const int NODE_RADIUS = 25;  // 节点半径
    const int LEVEL_HEIGHT = 80; // 层级高度
    const int ROOT_X = 500;      // 根节点 X 坐标 (画布居中)
    const int ROOT_Y = 60;       // 根节点 Y 坐标

    // === 内部核心逻辑 ===
    void cleanTreeRecursive(TreeNode* node);

    // 1. 计算布局：只更新 targetX/Y，不移动
    void calculateLayout(TreeNode* node, int x, int y, int hOffset);

    // 2. 刷新视觉：根据 targetX/Y 移动节点，重绘连线
    void refreshTreeVisuals(TreeNode* node, QPointF parentPos);

    // 查找路径动画 (预计算路径，连续滚动)
    void animSearchPath(int targetVal, std::function<void(TreeNode* parent, TreeNode* current, bool isLeft)> onFinished);

    // 创建可视节点
    void createVisualNode(TreeNode* node, int x, int y);

    // BST 逻辑辅助
    TreeNode* findMin(TreeNode* node);
    TreeNode* deleteNodeRecursive(TreeNode* root, int value, bool& deleted);

    // 垃圾回收：将节点的图形移入垃圾箱
    void markForDeletion(TreeNode* node);
    // 清空垃圾箱（播放动画后）
    void processTrashBin();
};