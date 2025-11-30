#pragma once
#include "avl_defs.h"
#include <QQueue>
#include <algorithm>

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

    // 外部接口：执行操作并返回一系列可视化指令
    QQueue<VisualCommand> insert(int val);
    QQueue<VisualCommand> remove(int val);
    QQueue<VisualCommand> search(int val);

private:
    LogicNode* root;
    QQueue<VisualCommand> cmds; // 当前累积的指令队列

    // 布局常量
    const double ROOT_X = 500;
    const double ROOT_Y = 60;
    const double LEVEL_H = 80;

    // --- 内部算法 ---
    void freeTree(LogicNode* node);

    // 递归操作
    LogicNode* insertRec(LogicNode* node, int val);
    LogicNode* removeRec(LogicNode* node, int val);

    // AVL 基础
    int height(LogicNode* n);
    int balanceFactor(LogicNode* n);
    void updateHeight(LogicNode* n);
    LogicNode* rotateRight(LogicNode* y);
    LogicNode* rotateLeft(LogicNode* x);
    LogicNode* findMin(LogicNode* node);

    // --- 关键：快照生成 ---
    // 生成当前整棵树的 MoveNode 和 SetParent 指令
    void snapshotLayout();
    void calcPosRec(LogicNode* node, double x, double y, double offset);

    // 辅助：生成高亮指令
    void addHighlight(int id, int duration = 500);
    void addWait(int duration);
};