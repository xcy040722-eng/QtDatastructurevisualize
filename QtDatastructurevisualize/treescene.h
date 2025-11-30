#pragma once
#include "basescene.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsLineItem>
#include <functional>
#include <QList>
#include <QString>
#include <QSequentialAnimationGroup> 

// 树节点结构体
struct TreeNode {
    int value;
    int height = 1; // === 新增：AVL 高度 ===

    TreeNode* left = nullptr;
    TreeNode* right = nullptr;

    QGraphicsEllipseItem* circle = nullptr;
    QGraphicsSimpleTextItem* text = nullptr;
    QGraphicsLineItem* linkToParent = nullptr;

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

    void insertNodeAnimated(int value, int index) override;
    void removeNodeAnimated(int value, int index) override;
    void searchNodeAnimated(int value, int index) override;
    void traverseAnimated(int type) override;

    // === 新增：设置 AVL 模式 ===
    void setAVLMode(bool enable);

private:
    TreeNode* root = nullptr;
    QGraphicsEllipseItem* m_probeHalo = nullptr;
    QGraphicsSimpleTextItem* m_resultText = nullptr;

    QList<QGraphicsItem*> m_trashItems;
    bool m_isAVL = false; // === 新增：AVL 标志位 ===

    const int NODE_RADIUS = 25;
    const int LEVEL_HEIGHT = 80;
    const int ROOT_X = 500;
    const int ROOT_Y = 60;

    void cleanTreeRecursive(TreeNode* node);
    void calculateLayout(TreeNode* node, int x, int y, int hOffset);
    void refreshTreeVisuals(TreeNode* node, QPointF parentPos);
    void animSearchPath(int targetVal, std::function<void(TreeNode* parent, TreeNode* current, bool isLeft)> onFinished);
    void createVisualNode(TreeNode* node, int x, int y);

    TreeNode* findMin(TreeNode* node);
    TreeNode* deleteNodeRecursive(TreeNode* root, int value, bool& deleted);
    void markForDeletion(TreeNode* node);
    void processTrashBin();

    void buildTraversalAnim(QSequentialAnimationGroup* group, TreeNode* node, int type, QString& resultString);
    void addVisitAnim(QSequentialAnimationGroup* group, TreeNode* node, QString& currentStr);
    TreeNode* getFirstNode(TreeNode* node, int type);

    // === 新增：AVL 核心算法 ===
    int getHeight(TreeNode* node);
    int getBalance(TreeNode* node);
    void updateHeight(TreeNode* node);

    TreeNode* rightRotate(TreeNode* y);
    TreeNode* leftRotate(TreeNode* x);
    TreeNode* insertAVLRecursive(TreeNode* node, TreeNode* newNode); // 递归插入并平衡
};