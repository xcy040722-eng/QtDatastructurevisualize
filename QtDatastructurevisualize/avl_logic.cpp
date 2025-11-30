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

    // 检测到不平衡，先拍快照，让之前的操作（如插入）先"落位"
    snapshotLayout();
    addWait(600);

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
        addHighlight(node->left->val);
        node->left = rotateLeft(node->left);
        snapAndWait(600);
        restoreColor(node->left->left->val);

        addHighlight(node->val);
        node = rotateRight(node);
        snapAndWait(800);
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
        addHighlight(node->right->val);
        node->right = rotateRight(node->right);
        snapAndWait(600);
        restoreColor(node->right->right->val);

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
    snapshotLayout();
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

    // === 1. 探针动画：每下一层，生成一个探针移动指令 ===
    VisualCommand probe(CommandType::SearchHighlight, node->val, 300);
    cmds.enqueue(probe);
    addWait(200); // 稍微停顿，让探针走一会

    if (val < node->val) {
        node->left = removeRec(node->left, val);
    }
    else if (val > node->val) {
        node->right = removeRec(node->right, val);
    }
    else {
        // === 2. 找到节点：高亮红色 ===
        addHighlight(node->val, 400, Qt::red);

        // Case 1: 叶子或单子节点
        if (!node->left || !node->right) {
            LogicNode* temp = node->left ? node->left : node->right;

            // === 3. 节点淡出 (RemoveNode) ===
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
            // 这里我们不需要给 findMin 加动画，因为用户关注的是"当前节点被删除了"
            LogicNode* temp = findMin(node->right);

            // 高亮替代者
            addHighlight(temp->val, 300, Qt::yellow);

            // 视觉技巧：移除当前节点(旧值)，然后稍后 snapshot 会把替代者(temp)瞬移过来
            // 或者更平滑：把当前节点值变了？
            // 鉴于我们的指令集限制，最稳妥的是：移除旧的 visual node，然后逻辑上 ID 变了
            // 新的 snapshotLayout 会为新 ID 创建位置。

            cmds.enqueue(VisualCommand(CommandType::RemoveNode, node->val)); // 移除红色的 node

            node->val = temp->val; // 逻辑值替换

            // 递归删除原来的 temp
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
    addUpdateText(m_traverseStr);

    if (type == 0) preOrder(root);
    else if (type == 1) inOrder(root);
    else if (type == 2) postOrder(root);
    return cmds;
}

void AVLLogic::preOrder(LogicNode* node) {
    if (!node) return;

    // 访问动画
    addHighlight(node->val, 400, QColor(255, 165, 0));

    // === 格式化文字：加箭头 ===
    if (!m_traverseStr.endsWith(": ")) {
        m_traverseStr += " -> ";
    }
    m_traverseStr += QString::number(node->val);
    addUpdateText(m_traverseStr);

    addHighlight(node->val, 0, QColor(144, 238, 144)); // 恢复绿

    preOrder(node->left);
    preOrder(node->right);
}

void AVLLogic::inOrder(LogicNode* node) {
    if (!node) return;
    inOrder(node->left);

    addHighlight(node->val, 400, QColor(255, 165, 0));

    if (!m_traverseStr.endsWith(": ")) {
        m_traverseStr += " -> ";
    }
    m_traverseStr += QString::number(node->val);
    addUpdateText(m_traverseStr);

    addHighlight(node->val, 0, QColor(144, 238, 144));

    inOrder(node->right);
}

void AVLLogic::postOrder(LogicNode* node) {
    if (!node) return;
    postOrder(node->left);
    postOrder(node->right);

    addHighlight(node->val, 400, QColor(255, 165, 0));

    if (!m_traverseStr.endsWith(": ")) {
        m_traverseStr += " -> ";
    }
    m_traverseStr += QString::number(node->val);
    addUpdateText(m_traverseStr);

    addHighlight(node->val, 0, QColor(144, 238, 144));
}