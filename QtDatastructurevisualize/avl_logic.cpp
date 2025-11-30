#include "avl_logic.h"

AVLLogic::AVLLogic() : root(nullptr) {}

AVLLogic::~AVLLogic() { reset(); }

void AVLLogic::reset() {
    freeTree(root);
    root = nullptr;
    m_traverseStr.clear();
}

void AVLLogic::freeTree(LogicNode* node) {
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

// === 核心：生成布局快照 ===
void AVLLogic::snapshotLayout() {
    if (!root) return;
    // 计算坐标并生成 Move/SetParent 指令
    calcPosRec(root, ROOT_X, ROOT_Y, 200);
}

void AVLLogic::calcPosRec(LogicNode* node, double x, double y, double offset) {
    if (!node) return;
    node->x = x;
    node->y = y;

    // 移动指令
    VisualCommand moveCmd(CommandType::MoveNode, node->val, 600);
    moveCmd.pos = QPointF(x, y);
    cmds.enqueue(moveCmd);

    // 连线指令
    if (node->left) {
        VisualCommand linkCmd(CommandType::SetParent, node->left->val);
        linkCmd.relatedId = node->val;
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
LogicNode* AVLLogic::findMin(LogicNode* node) { while (node->left) node = node->left; return node; }

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

// 统一的平衡逻辑（插入和删除共用）
LogicNode* AVLLogic::balanceNode(LogicNode* node) {
    updateHeight(node);
    int balance = balanceFactor(node);

    // 如果平衡，直接返回
    if (balance >= -1 && balance <= 1) return node;

    // === 关键修复：检测到不平衡，先拍快照 ===
    // 这会让刚插入的节点（例如 3->1->2 中的 2）先移动到它的逻辑位置
    // 从而让用户看到“歪掉”的树，而不是直接跳到高亮旋转
    snapshotLayout();
    addWait(600); // 停顿一下，展示不平衡状态

    auto snapAndWait = [&](int waitMs) { snapshotLayout(); addWait(waitMs); };
    auto restoreColor = [&](int id) {
        VisualCommand r(CommandType::HighlightNode, id, 0);
        r.color = QColor(144, 238, 144);
        cmds.enqueue(r);
        };

    // LL
    if (balance > 1 && balanceFactor(node->left) >= 0) {
        addHighlight(node->val);
        node = rotateRight(node);
        snapAndWait(800);
        restoreColor(node->right->val);
    }
    // LR
    else if (balance > 1 && balanceFactor(node->left) < 0) {
        // 阶段1：左旋左子节点
        addHighlight(node->left->val);
        node->left = rotateLeft(node->left);
        snapAndWait(600); // 展示中间态 (3->2->1)
        restoreColor(node->left->left->val);

        // 阶段2：右旋当前节点
        addHighlight(node->val);
        node = rotateRight(node);
        snapAndWait(800); // 展示最终态 (2->1,3)
        restoreColor(node->right->val);
    }
    // RR
    else if (balance < -1 && balanceFactor(node->right) <= 0) {
        addHighlight(node->val);
        node = rotateLeft(node);
        snapAndWait(800);
        restoreColor(node->left->val);
    }
    // RL
    else if (balance < -1 && balanceFactor(node->right) > 0) {
        // 阶段1：右旋右子节点
        addHighlight(node->right->val);
        node->right = rotateRight(node->right);
        snapAndWait(600);
        restoreColor(node->right->right->val);

        // 阶段2：左旋当前节点
        addHighlight(node->val);
        node = rotateLeft(node);
        snapAndWait(800);
        restoreColor(node->left->val);
    }

    return node;
}

// === 辅助指令 ===
void AVLLogic::addHighlight(int id, int duration, QColor c) {
    VisualCommand h(CommandType::HighlightNode, id, duration);
    h.color = c;
    cmds.enqueue(h);
    // 如果有持续时间，则加一个 Wait 指令阻塞后续动作
    if (duration > 0) addWait(duration);
}

void AVLLogic::addWait(int duration) {
    cmds.enqueue(VisualCommand(CommandType::Wait, 0, duration));
}

void AVLLogic::addUpdateText(const QString& text) {
    VisualCommand cmd(CommandType::UpdateResultText, 0, 0);
    cmd.text = text;
    cmds.enqueue(cmd);
}

// =======================
// === 1. 插入 (Insert) ===
// =======================
QQueue<VisualCommand> AVLLogic::insert(int val) {
    cmds.clear();
    // 简单排重
    LogicNode* curr = root;
    while (curr) { if (curr->val == val) return cmds; curr = (val < curr->val) ? curr->left : curr->right; }

    root = insertRec(root, val);
    snapshotLayout(); // 最终确认
    return cmds;
}

LogicNode* AVLLogic::insertRec(LogicNode* node, int val) {
    if (!node) {
        LogicNode* newNode = new LogicNode(val);
        cmds.enqueue(VisualCommand(CommandType::CreateNode, val));
        return newNode;
    }
    if (val < node->val) node->left = insertRec(node->left, val);
    else if (val > node->val) node->right = insertRec(node->right, val);
    else return node;

    // 平衡修复
    return balanceNode(node);
}

// =======================
// === 2. 删除 (Remove) ===
// =======================
QQueue<VisualCommand> AVLLogic::remove(int val) {
    cmds.clear();
    root = removeRec(root, val);
    snapshotLayout();
    return cmds;
}

LogicNode* AVLLogic::removeRec(LogicNode* node, int val) {
    if (!node) return nullptr;

    if (val < node->val) {
        node->left = removeRec(node->left, val);
    }
    else if (val > node->val) {
        node->right = removeRec(node->right, val);
    }
    else {
        // === 找到节点，准备删除 ===
        addHighlight(node->val, 300, Qt::red);

        // Case 1: 叶子或单子节点
        if (!node->left || !node->right) {
            LogicNode* temp = node->left ? node->left : node->right;
            cmds.enqueue(VisualCommand(CommandType::RemoveNode, node->val));
            if (!temp) {
                node = nullptr;
            }
            else {
                node = temp;
            }
        }
        else {
            // Case 2: 双子节点
            LogicNode* temp = findMin(node->right);
            addHighlight(temp->val, 300, Qt::yellow);

            // 视觉移除旧节点
            cmds.enqueue(VisualCommand(CommandType::RemoveNode, node->val));
            // 逻辑值替换
            node->val = temp->val;
            // 递归删除
            node->right = removeRec(node->right, temp->val);
        }
    }

    if (!node) return nullptr;
    return balanceNode(node);
}

// =======================
// === 3. 查找 (Search) ===
// =======================
QQueue<VisualCommand> AVLLogic::search(int val) {
    cmds.clear();
    LogicNode* curr = root;
    while (curr) {
        VisualCommand cmd(CommandType::SearchHighlight, curr->val, 300);
        cmds.enqueue(cmd);
        addWait(200);

        if (val == curr->val) {
            addHighlight(curr->val, 500, Qt::green);
            VisualCommand r(CommandType::HighlightNode, curr->val, 0);
            r.color = QColor(144, 238, 144);
            cmds.enqueue(r);
            return cmds;
        }
        curr = (val < curr->val) ? curr->left : curr->right;
    }
    return cmds;
}

// =======================
// === 4. 遍历 (Traverse) ===
// =======================
QQueue<VisualCommand> AVLLogic::traverse(int type) {
    cmds.clear();
    m_traverseStr = (type == 0) ? "前序: " : (type == 1 ? "中序: " : "后序: ");
    addUpdateText(m_traverseStr); // 初始化文字

    if (type == 0) preOrder(root);
    else if (type == 1) inOrder(root);
    else if (type == 2) postOrder(root);
    return cmds;
}

void AVLLogic::preOrder(LogicNode* node) {
    if (!node) return;

    // 访问
    addHighlight(node->val, 400, QColor(255, 165, 0));
    m_traverseStr += QString::number(node->val) + " ";
    addUpdateText(m_traverseStr);
    addHighlight(node->val, 0, QColor(144, 238, 144)); // 恢复

    preOrder(node->left);
    preOrder(node->right);
}

void AVLLogic::inOrder(LogicNode* node) {
    if (!node) return;
    inOrder(node->left);

    addHighlight(node->val, 400, QColor(255, 165, 0));
    m_traverseStr += QString::number(node->val) + " ";
    addUpdateText(m_traverseStr);
    addHighlight(node->val, 0, QColor(144, 238, 144));

    inOrder(node->right);
}

void AVLLogic::postOrder(LogicNode* node) {
    if (!node) return;
    postOrder(node->left);
    postOrder(node->right);

    addHighlight(node->val, 400, QColor(255, 165, 0));
    m_traverseStr += QString::number(node->val) + " ";
    addUpdateText(m_traverseStr);
    addHighlight(node->val, 0, QColor(144, 238, 144));
}