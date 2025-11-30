#include "avl_logic.h"

AVLLogic::AVLLogic() : root(nullptr) {}

AVLLogic::~AVLLogic() { reset(); }

void AVLLogic::reset() {
    freeTree(root);
    root = nullptr;
}

void AVLLogic::freeTree(LogicNode* node) {
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

// === 核心：生成布局快照 ===
// 这个函数是"多阶段动画"的灵魂。每当树结构发生一点变化（如旋转了一半），
// 我们就调用一次这个函数，它会告诉界面："现在的树长这样，请把节点移过去，线连好"。
void AVLLogic::snapshotLayout() {
    if (!root) return;

    // 1. 递归计算坐标并生成 Move 指令 + SetParent 指令
    // 初始 offset 给 200
    calcPosRec(root, ROOT_X, ROOT_Y, 200);
}

void AVLLogic::calcPosRec(LogicNode* node, double x, double y, double offset) {
    if (!node) return;
    node->x = x;
    node->y = y;

    // 生成移动指令
    VisualCommand moveCmd(CommandType::MoveNode, node->val, 600); // 600ms移动动画
    moveCmd.pos = QPointF(x, y);
    cmds.enqueue(moveCmd);

    // 生成连线指令 (告诉 Scene：node 的左孩子是 left->val)
    if (node->left) {
        VisualCommand linkCmd(CommandType::SetParent, node->left->val);
        linkCmd.relatedId = node->val; // 左孩子的父节点是 node
        cmds.enqueue(linkCmd);

        double nextOffset = std::max(35.0, offset / 1.8);
        calcPosRec(node->left, x - offset, y + LEVEL_H, nextOffset);
    }

    if (node->right) {
        VisualCommand linkCmd(CommandType::SetParent, node->right->val);
        linkCmd.relatedId = node->val;
        cmds.enqueue(linkCmd);

        double nextOffset = std::max(35.0, offset / 1.8);
        calcPosRec(node->right, x + offset, y + LEVEL_H, nextOffset);
    }
}

// === AVL 基础操作 ===
int AVLLogic::height(LogicNode* n) { return n ? n->height : 0; }
int AVLLogic::balanceFactor(LogicNode* n) { return n ? height(n->left) - height(n->right) : 0; }
void AVLLogic::updateHeight(LogicNode* n) { if (n) n->height = 1 + std::max(height(n->left), height(n->right)); }

LogicNode* AVLLogic::rotateRight(LogicNode* y) {
    LogicNode* x = y->left;
    LogicNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    updateHeight(y);
    updateHeight(x);
    return x;
}

LogicNode* AVLLogic::rotateLeft(LogicNode* x) {
    LogicNode* y = x->right;
    LogicNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    updateHeight(x);
    updateHeight(y);
    return y;
}

void AVLLogic::addHighlight(int id, int duration) {
    VisualCommand h(CommandType::HighlightNode, id, duration);
    h.color = QColor(255, 100, 255); // 紫色高亮旋转轴心
    cmds.enqueue(h);
}

void AVLLogic::addWait(int duration) {
    cmds.enqueue(VisualCommand(CommandType::Wait, 0, duration));
}

// === 核心业务：插入 ===
QQueue<VisualCommand> AVLLogic::insert(int val) {
    cmds.clear();

    // 检查重复（为了简化，这里略过搜索动画，直接检查）
    LogicNode* curr = root;
    while (curr) {
        if (curr->val == val) return cmds;
        curr = (val < curr->val) ? curr->left : curr->right;
    }

    root = insertRec(root, val);

    // 确保最终状态被刷新
    snapshotLayout();
    return cmds;
}

LogicNode* AVLLogic::insertRec(LogicNode* node, int val) {
    // 1. 标准 BST 插入
    if (!node) {
        LogicNode* newNode = new LogicNode(val);
        // 生成创建指令
        VisualCommand createCmd(CommandType::CreateNode, val);
        cmds.enqueue(createCmd);
        // 刚创建时不知道位置，snapshotLayout 会稍后修正它
        return newNode;
    }

    if (val < node->val) node->left = insertRec(node->left, val);
    else if (val > node->val) node->right = insertRec(node->right, val);
    else return node;

    // 更新高度
    updateHeight(node);
    int balance = balanceFactor(node);

    // === 旋转处理 (多阶段动画的核心) ===

    // 辅助：拍快照并等待
    auto snapAndWait = [&](int waitMs) {
        snapshotLayout();
        addWait(waitMs);
        };

    // 辅助：恢复颜色
    auto restoreColor = [&](int id) {
        VisualCommand r(CommandType::HighlightNode, id, 0);
        r.color = QColor(144, 238, 144); // 默认绿
        cmds.enqueue(r);
        };

    // Case 1: LL (右旋)
    if (balance > 1 && val < node->left->val) {
        // 阶段1: 展示 BST 插入后的不平衡状态 (图1)
        snapAndWait(500);

        // 阶段2: 高亮并旋转
        addHighlight(node->val);
        addWait(500);

        node = rotateRight(node);

        // 阶段3: 旋转后 (图3，LL是一步到位的)
        snapAndWait(800);
        restoreColor(node->right->val); // 原来的 node 变成了右孩子
    }
    // Case 2: RR (左旋)
    else if (balance < -1 && val > node->right->val) {
        snapAndWait(500);

        addHighlight(node->val);
        addWait(500);

        node = rotateLeft(node);

        snapAndWait(800);
        restoreColor(node->left->val);
    }
    // Case 3: LR (先左旋左子树，再右旋自己) - 你的例子 (3, 1, 2)
    else if (balance > 1 && val > node->left->val) {
        // 阶段1: 此时树结构是 3->1->2 (图1)
        snapAndWait(500);

        // --- 子阶段 1: 左旋左子树 (1) ---
        addHighlight(node->left->val); // 高亮 1
        addWait(500);

        node->left = rotateLeft(node->left); // 逻辑变成 3->2->1

        // 阶段2: 刷新视图 (图2)
        snapAndWait(800);
        restoreColor(node->left->left->val); // 恢复 1 的颜色 (它现在是 2 的左孩子)

        // --- 子阶段 2: 右旋当前节点 (3) ---
        addHighlight(node->val); // 高亮 3
        addWait(500);

        node = rotateRight(node); // 逻辑变成 2->(1,3)

        // 阶段3: 最终状态 (图3)
        snapAndWait(800);
        restoreColor(node->right->val); // 恢复 3 的颜色
    }
    // Case 4: RL (先右旋右子树，再左旋自己)
    else if (balance < -1 && val < node->right->val) {
        snapAndWait(500);

        // 子阶段 1
        addHighlight(node->right->val);
        addWait(500);

        node->right = rotateRight(node->right);

        snapAndWait(800); // 中间态
        restoreColor(node->right->right->val);

        // 子阶段 2
        addHighlight(node->val);
        addWait(500);

        node = rotateLeft(node);

        snapAndWait(800); // 最终态
        restoreColor(node->left->val);
    }

    return node;
}

// 删除和查找逻辑简单实现框架
QQueue<VisualCommand> AVLLogic::remove(int val) {
    cmds.clear();
    // 简单实现：找到并删除
    // 若要支持删除平衡，逻辑同 insertRec
    root = removeRec(root, val);
    snapshotLayout();
    return cmds;
}

LogicNode* AVLLogic::removeRec(LogicNode* node, int val) {
    if (!node) return nullptr;
    if (val < node->val) node->left = removeRec(node->left, val);
    else if (val > node->val) node->right = removeRec(node->right, val);
    else {
        // 找到节点，生成删除指令
        // 简化：使用替换法删除，这里为了代码短省略细节，参考原 Treescene
        // 重点是每次变动都要 snapshotLayout()
        // ...
        return nullptr; // 占位
    }
    // 删除后的平衡逻辑同 Insert
    return node;
}

QQueue<VisualCommand> AVLLogic::search(int val) {
    cmds.clear();
    LogicNode* curr = root;
    while (curr) {
        VisualCommand hl(CommandType::SearchHighlight, curr->val, 300);
        cmds.enqueue(hl);
        cmds.enqueue(VisualCommand(CommandType::Wait, 0, 300)); // 走一步停一下
        if (val == curr->val) break;
        curr = (val < curr->val) ? curr->left : curr->right;
    }
    return cmds;
}