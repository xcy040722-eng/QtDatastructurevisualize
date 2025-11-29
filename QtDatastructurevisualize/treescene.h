#pragma once
#include "basescene.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsLineItem>
#include <functional>

// 树节点结构体：数据 + 视觉
struct TreeNode {
    int value;
    TreeNode* left = nullptr;
    TreeNode* right = nullptr;

    // 视觉元素
    QGraphicsEllipseItem* circle = nullptr;  // 圆形节点
    QGraphicsSimpleTextItem* text = nullptr; // 数值文本
    QGraphicsLineItem* linkToParent = nullptr; // 指向父节点的连线

    // 布局辅助
    int x = 0;
    int y = 0;

    explicit TreeNode(int v) : value(v) {}
};

class TreeScene : public BaseScene {
    Q_OBJECT
public:
    explicit TreeScene(QObject* parent = nullptr);
    ~TreeScene() override;

    void reset() override;

    // 重写基类接口
    void insertNodeAnimated(int value, int index) override;
    void removeNodeAnimated(int value, int index) override;
    void searchNodeAnimated(int value, int index) override;

private:
    TreeNode* root = nullptr; // 根节点指针

    // 辅助图形：红色探针环
    QGraphicsEllipseItem* m_probeHalo = nullptr;

    // === 布局常量 ===
    const int NODE_RADIUS = 25; // 半径
    const int LEVEL_HEIGHT = 80; // 层高
    const int ROOT_X = 500;      // 根节点 X 坐标
    const int ROOT_Y = 60;       // 根节点 Y 坐标

    // === 内部核心逻辑 ===
    void cleanTreeRecursive(TreeNode* node);

    // 递归计算节点位置
    void updateLayout(TreeNode* node, int x, int y, int hOffset);

    // 递归移动节点到新位置 (动画)
    void animateLayout(TreeNode* node);

    // 查找路径动画 (弹珠下落)
    void animSearchPath(int targetVal, std::function<void(TreeNode* parent, TreeNode* current, bool isLeft)> onFinished);

    // 辅助：创建可视节点
    void createVisualNode(TreeNode* node, int x, int y);
    // 辅助：移除可视节点
    void removeVisualNode(TreeNode* node);

    // BST 逻辑辅助
    TreeNode* findMin(TreeNode* node);
    TreeNode* deleteNodeRecursive(TreeNode* root, int value, bool& deleted);
};