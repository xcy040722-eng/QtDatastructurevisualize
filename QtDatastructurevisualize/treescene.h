#pragma once
#include "basescene.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsLineItem>
#include <functional>
#include <QList>
#include <QString>
#include <QSequentialAnimationGroup> 
#include <QParallelAnimationGroup> 
#include <QEventLoop> 

// 树节点结构体
struct TreeNode {
    int value;
    int height = 1;

    TreeNode* left = nullptr;
    TreeNode* right = nullptr;

    // 视觉元素
    QGraphicsEllipseItem* circle = nullptr;
    QGraphicsSimpleTextItem* text = nullptr;
    // 动态管理的连线
    QGraphicsLineItem* linkToParent = nullptr;

    // === 核心：布局计算的目标位置 ===
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

    // 标准接口
    void insertNodeAnimated(int value, int index) override;
    void removeNodeAnimated(int value, int index) override;
    void searchNodeAnimated(int value, int index) override;
    void traverseAnimated(int type) override;

    void setAVLMode(bool enable);

private:
    TreeNode* root = nullptr;
    QGraphicsEllipseItem* m_probeHalo = nullptr;
    QGraphicsSimpleTextItem* m_resultText = nullptr;

    QList<QGraphicsItem*> m_trashItems;
    bool m_isAVL = false;

    const int NODE_RADIUS = 25;
    const int LEVEL_HEIGHT = 80;
    const int ROOT_X = 500;
    const int ROOT_Y = 60;

    // === 视觉更新系统 ===
    void clearAllLines(TreeNode* node);
    void rebuildLines(TreeNode* node);
    void calculateLayout(TreeNode* node, int x, int y, int hOffset);
    void updateViewAndWait(int duration = 600);
    void createMoveAnimsRecursive(TreeNode* node, QParallelAnimationGroup* group, int duration);

    // 基础辅助
    void cleanTreeRecursive(TreeNode* node);
    void createVisualNode(TreeNode* node, int x, int y);
    void animSearchPath(int targetVal, std::function<void(TreeNode* parent, TreeNode* current, bool isLeft)> onFinished);

    // AVL 逻辑
    int getHeight(TreeNode* node);
    int getBalance(TreeNode* node);
    void updateHeight(TreeNode* node);

    TreeNode* rightRotate(TreeNode* y);
    TreeNode* leftRotate(TreeNode* x);
    void insertAVLRecursive(TreeNode*& node, TreeNode* newNode);

    // 删除逻辑
    TreeNode* deleteNodeRecursive(TreeNode*& node, int value, bool& deleted);
    TreeNode* findMin(TreeNode* node);

    // 垃圾回收
    void markForDeletion(TreeNode* node);
    void processTrashBin();

    // 遍历
    void buildTraversalAnim(QSequentialAnimationGroup* group, TreeNode* node, int type, QString& resultString);
    void addVisitAnim(QSequentialAnimationGroup* group, TreeNode* node, QString& currentStr);
    TreeNode* getFirstNode(TreeNode* node, int type);
};