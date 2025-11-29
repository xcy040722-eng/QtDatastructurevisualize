#include "treescene.h"
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QVariantAnimation>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <QTimer>

TreeScene::TreeScene(QObject* parent) : BaseScene(parent) {
    // 树形结构可能很宽/很深，设置较大的场景范围
    setSceneRect(0, 0, 2000, 1500);
}

TreeScene::~TreeScene() {
    reset();
}

void TreeScene::reset() {
    cleanTreeRecursive(root);
    root = nullptr;
    if (m_probeHalo) {
        removeItem(m_probeHalo);
        delete m_probeHalo;
        m_probeHalo = nullptr;
    }
    update();
}

void TreeScene::cleanTreeRecursive(TreeNode* node) {
    if (!node) return;
    cleanTreeRecursive(node->left);
    cleanTreeRecursive(node->right);

    if (node->circle) { removeItem(node->circle); delete node->circle; }
    if (node->text) {
        // text 通常是 circle 的 child item，circle 删除时 text 也会自动删除
        // 但为了保险，如果它是独立的，也可以删。这里它是 child，所以不需要手动 delete text
    }
    if (node->linkToParent) { removeItem(node->linkToParent); delete node->linkToParent; }
    delete node;
}

// === 核心布局算法 ===
// 参数 hOffset: 当前层级的水平偏移量，随深度增加而减半
// 这保证了树形状是标准的金字塔形，不会重叠
void TreeScene::updateLayout(TreeNode* node, int x, int y, int hOffset) {
    if (!node) return;

    node->x = x;
    node->y = y;

    // 递归处理左右子树
    // 下一层的偏移量减少。最小偏移量设为 40，防止挤在一起
    int nextOffset = qMax(40, hOffset / 2);

    updateLayout(node->left, x - hOffset, y + LEVEL_HEIGHT, nextOffset);
    updateLayout(node->right, x + hOffset, y + LEVEL_HEIGHT, nextOffset);
}

// 将节点平滑移动到计算好的 layout 位置
void TreeScene::animateLayout(TreeNode* node) {
    if (!node || !node->circle) return;

    // 目标位置
    QPointF endPos(node->x - NODE_RADIUS, node->y - NODE_RADIUS);

    // 如果位置变了，播放动画
    if (node->circle->pos() != endPos) {
        QVariantAnimation* vAnim = new QVariantAnimation(this);
        vAnim->setDuration(600); // 移动耗时
        vAnim->setStartValue(node->circle->pos());
        vAnim->setEndValue(endPos);
        vAnim->setEasingCurve(QEasingCurve::InOutQuad);

        connect(vAnim, &QVariantAnimation::valueChanged, this, [node](const QVariant& val) {
            if (node && node->circle) {
                node->circle->setPos(val.toPointF());
                // 同步更新连线端点
                if (node->linkToParent) {
                    QLineF line = node->linkToParent->line();
                    line.setP2(node->circle->pos() + QPointF(25, 25)); // 连接到圆心
                    node->linkToParent->setLine(line);
                }
            }
            });

        // 动画对象自动销毁
        connect(vAnim, &QVariantAnimation::finished, vAnim, &QObject::deleteLater);
        vAnim->start();
    }

    // 父节点连线的起点也需要更新 (虽然上面只更新了终点 P2)
    // 为了简化，我们假设父节点也在移动，父节点会负责更新它所有孩子的连线起点 P1
    // 或者我们在 updateLayout 后统一重绘连线。
    // Phase 3 简单版：我们只更新 P2。
    // 如果父节点移动了，P1 没动，线会断。
    // 修正：我们需要在父节点移动时，更新左右孩子的 line P1。

    // 更好的做法：在这里不做复杂的连线更新动画，而是每一帧重绘。
    // 但为了性能和代码量，我们采用简单的策略：递归调用动画
    animateLayout(node->left);
    animateLayout(node->right);
}

// === 探针逻辑 ===
void TreeScene::animSearchPath(int targetVal, std::function<void(TreeNode*, TreeNode*, bool)> onFinished) {
    if (!m_probeHalo) {
        m_probeHalo = addEllipse(0, 0, NODE_RADIUS * 2 + 10, NODE_RADIUS * 2 + 10,
            QPen(Qt::red, 3), Qt::NoBrush);
        m_probeHalo->setZValue(999);
        m_probeHalo->setVisible(false);
    }

    m_probeHalo->setVisible(true);

    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);

    TreeNode* curr = root;
    TreeNode* parent = nullptr;
    bool isLeft = false;

    // 初始位置：根节点上方
    QPointF startPos(ROOT_X - NODE_RADIUS - 5, ROOT_Y - NODE_RADIUS - 5);
    if (root && root->circle) startPos = root->circle->pos() - QPointF(5, 5);

    m_probeHalo->setPos(startPos);

    // 模拟搜索路径
    while (curr != nullptr) {
        // 添加比较停顿
        group->addPause(300);

        if (curr->value == targetVal) {
            break; // 找到了
        }

        parent = curr;

        if (targetVal < curr->value) {
            curr = curr->left;
            isLeft = true;
        }
        else {
            curr = curr->right;
            isLeft = false;
        }

        if (curr && curr->circle) {
            // 移动到下一个节点
            QVariantAnimation* move = new QVariantAnimation(group);
            move->setDuration(400);
            move->setStartValue(m_probeHalo->pos());
            move->setEndValue(curr->circle->pos() - QPointF(5, 5));
            move->setEasingCurve(QEasingCurve::InOutQuad);

            // 必须捕获 this
            connect(move, &QVariantAnimation::valueChanged, this, [this](const QVariant& val) {
                if (m_probeHalo) m_probeHalo->setPos(val.toPointF());
                });
            group->addAnimation(move);
        }
    }

    connect(group, &QAbstractAnimation::finished, this, [this, onFinished, parent, curr, isLeft, group]() {
        m_probeHalo->setVisible(false);
        onFinished(parent, curr, isLeft);
        group->deleteLater();
        });

    group->start();
}

void TreeScene::createVisualNode(TreeNode* node, int x, int y) {
    node->x = x;
    node->y = y;

    // 绘制圆形
    node->circle = addEllipse(0, 0, NODE_RADIUS * 2, NODE_RADIUS * 2,
        QPen(Qt::black, 2), QBrush(QColor(144, 238, 144))); // 浅绿
    node->circle->setPos(x - NODE_RADIUS, y - NODE_RADIUS);
    node->circle->setZValue(10); // 确保在连线上方

    // 绘制文本
    node->text = new QGraphicsSimpleTextItem(QString::number(node->value), node->circle);
    auto b = node->text->boundingRect();
    // 居中
    node->text->setPos((NODE_RADIUS * 2 - b.width()) / 2, (NODE_RADIUS * 2 - b.height()) / 2);
}

void TreeScene::insertNodeAnimated(int value, int index) {
    (void)index;

    if (!root) {
        // 根节点情况
        root = new TreeNode(value);
        createVisualNode(root, ROOT_X, ROOT_Y);

        // 弹出动画
        root->circle->setScale(0);
        QVariantAnimation* anim = new QVariantAnimation(this);
        anim->setDuration(500);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant& val) {
            if (root && root->circle) {
                root->circle->setScale(val.toFloat());
                root->circle->setTransformOriginPoint(NODE_RADIUS, NODE_RADIUS);
            }
            });
        connect(anim, &QVariantAnimation::finished, this, &BaseScene::animationFinished);
        anim->start();
        return;
    }

    // 正常插入：先播放查找
    animSearchPath(value, [this, value](TreeNode* parent, TreeNode* current, bool isLeft) {
        if (current != nullptr) {
            // 已存在，不高亮也不插入，直接结束
            emit animationFinished();
            return;
        }

        // 创建新节点结构
        TreeNode* newNode = new TreeNode(value);
        if (isLeft) parent->left = newNode;
        else parent->right = newNode;

        // === 关键：先重新计算所有节点的目标位置 ===
        updateLayout(root, ROOT_X, ROOT_Y, 200);

        // 绘制新节点（先画在目标位置）
        createVisualNode(newNode, newNode->x, newNode->y);

        // 绘制连线
        QPointF pCenter(parent->x, parent->y);
        QPointF cCenter(newNode->x, newNode->y);
        QGraphicsLineItem* line = addLine(QLineF(pCenter, cCenter), QPen(Qt::black, 2));
        line->setZValue(0); // 在底层
        newNode->linkToParent = line;

        // 新节点淡入动画
        newNode->circle->setOpacity(0);
        line->setOpacity(0);

        QVariantAnimation* anim = new QVariantAnimation(this);
        anim->setDuration(500);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        connect(anim, &QVariantAnimation::valueChanged, this, [newNode, line](const QVariant& val) {
            if (newNode && newNode->circle) newNode->circle->setOpacity(val.toFloat());
            if (line) line->setOpacity(val.toFloat());
            });

        // 动画结束后，不仅解锁 UI，还可以顺便执行一次全树的平滑调整（防止布局拥挤）
        connect(anim, &QVariantAnimation::finished, this, [this]() {
            animateLayout(root); // 确保所有节点都对齐到新网格
            emit animationFinished();
            });
        anim->start();
        });
}

// 辅助：找最小
TreeNode* TreeScene::findMin(TreeNode* node) {
    while (node->left != nullptr) node = node->left;
    return node;
}

// 辅助：递归删除逻辑
TreeNode* TreeScene::deleteNodeRecursive(TreeNode* root, int value, bool& deleted) {
    if (root == nullptr) return root;

    if (value < root->value) {
        root->left = deleteNodeRecursive(root->left, value, deleted);
    }
    else if (value > root->value) {
        root->right = deleteNodeRecursive(root->right, value, deleted);
    }
    else {
        // 找到目标
        deleted = true;

        // Case 1: 无子节点 或 仅有一个子节点
        if (root->left == nullptr) {
            TreeNode* temp = root->right;
            // 删除视觉项
            if (root->circle) { removeItem(root->circle); delete root->circle; }
            if (root->linkToParent) { removeItem(root->linkToParent); delete root->linkToParent; }
            delete root;
            return temp;
        }
        else if (root->right == nullptr) {
            TreeNode* temp = root->left;
            if (root->circle) { removeItem(root->circle); delete root->circle; }
            if (root->linkToParent) { removeItem(root->linkToParent); delete root->linkToParent; }
            delete root;
            return temp;
        }

        // Case 2: 两个子节点 -> 找右子树最小值顶替
        TreeNode* temp = findMin(root->right);
        root->value = temp->value; // 逻辑值替换

        // 视觉值替换
        if (root->text) root->text->setText(QString::number(root->value));

        // 递归删除那个被拿来顶替的节点
        root->right = deleteNodeRecursive(root->right, temp->value, deleted);
    }
    return root;
}

void TreeScene::removeNodeAnimated(int value, int index) {
    (void)index;

    animSearchPath(value, [this, value](TreeNode* parent, TreeNode* curr, bool) {
        // 这里的 curr 只是动画寻址的终点，不一定是真正的逻辑节点（因为 animSearchPath 只是模拟）
        // 真正的查找在 deleteNodeRecursive 里做

        bool deleted = false;
        root = deleteNodeRecursive(root, value, deleted);

        if (deleted) {
            // === 修复：使用布局刷新代替 cleanAllGraphics ===
            // 1. 重新计算位置 (因为节点少了，树可能变窄)
            updateLayout(root, ROOT_X, ROOT_Y, 200);

            // 2. 播放调整动画 (让剩下的节点飘到新位置)
            animateLayout(root);
        }

        emit animationFinished();
        });
}

void TreeScene::searchNodeAnimated(int value, int index) {
    (void)index;
    animSearchPath(value, [this, value](TreeNode* p, TreeNode* c, bool) {
        // 注意：animSearchPath 结束时 c 可能为空(没找到)，也可能不为空(找到了)
        // 我们需要再次确认一下
        if (c && c->value == value && c->circle) {
            // 变绿
            QBrush originalBrush = c->circle->brush();
            c->circle->setBrush(QBrush(Qt::green));

            // 1秒后恢复
            QTimer::singleShot(1000, [c, originalBrush]() {
                if (c && c->circle) c->circle->setBrush(originalBrush);
                });
        }
        emit animationFinished();
        });
}