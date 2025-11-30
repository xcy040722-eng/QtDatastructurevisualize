#pragma once
#include "avl_defs.h"
#include <QQueue>
#include <algorithm>
#include <QString>

// 纯逻辑节点
struct LogicNode {
    int val;
    int height;
    LogicNode* left = nullptr;
    LogicNode* right = nullptr;

    // 逻辑坐标（计算用）
    double x, y;

    LogicNode(int v) : val(v), height(1), left(nullptr), right(nullptr), x(0), y(0) {}
};

class AVLLogic {
public:
    AVLLogic();
    ~AVLLogic();

    void reset();

    // === 核心接口 ===
    QQueue<VisualCommand> insert(int val);
    QQueue<VisualCommand> remove(int val);
    QQueue<VisualCommand> search(int val);
    // type: 0=Pre, 1=In, 2=Post
    QQueue<VisualCommand> traverse(int type);

private:
    LogicNode* root;
    QQueue<VisualCommand> cmds; // 指令缓冲
    QString m_traverseStr;      // === 新增：用于构建遍历结果字符串 ===

    // 布局常量
    const double ROOT_X = 500;
    const double ROOT_Y = 60;
    const double LEVEL_H = 80;

    // --- 内部算法 ---
    void freeTree(LogicNode* node);

    // 递归逻辑
    LogicNode* insertRec(LogicNode* node, int val);
    LogicNode* removeRec(LogicNode* node, int val);

    // 遍历逻辑
    void preOrder(LogicNode* node);
    void inOrder(LogicNode* node);
    void postOrder(LogicNode* node);

    // AVL 基础
    int height(LogicNode* n);
    int balanceFactor(LogicNode* n);
    void updateHeight(LogicNode* n);
    LogicNode* balanceNode(LogicNode* node); // 平衡逻辑
    LogicNode* rotateRight(LogicNode* y);
    LogicNode* rotateLeft(LogicNode* x);
    LogicNode* findMin(LogicNode* node);

    // --- 快照与指令生成 ---
    void snapshotLayout();
    void calcPosRec(LogicNode* node, double x, double y, double offset);

    // 辅助指令生成
    void addHighlight(int id, int duration = 500, QColor c = QColor(255, 100, 255));
    void addWait(int duration);
    void addUpdateText(const QString& text); // === 新增 ===
};