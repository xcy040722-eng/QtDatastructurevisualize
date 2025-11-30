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

    // 遍历接口
    void traverseAnimated(int type) override;

private:
    TreeNode* root = nullptr;
    QGraphicsEllipseItem* m_probeHalo = nullptr;
    // 遍历结果显示文本
    QGraphicsSimpleTextItem* m_resultText = nullptr;

    QList<QGraphicsItem*> m_trashItems;

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

    // === 遍历辅助函数 (注意：它们必须是普通成员函数，不能是 static) ===
    void buildTraversalAnim(QSequentialAnimationGroup* group, TreeNode* node, int type, QString& resultString);
    void addVisitAnim(QSequentialAnimationGroup* group, TreeNode* node, QString& currentStr);
};