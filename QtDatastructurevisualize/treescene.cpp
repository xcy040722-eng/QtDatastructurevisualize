#include "treescene.h"
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QVariantAnimation>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <Qtimer.h>

TreeScene::TreeScene(QObject* parent) : BaseScene(parent) {
    // 设置足够大的画布，支持滚动条
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
    // 清空垃圾箱
    for (auto item : m_trashItems) {
        if (item) { removeItem(item); delete item; }
    }
    m_trashItems.clear();
    update();
}

void TreeScene::cleanTreeRecursive(TreeNode* node) {
    if (!node) return;
    cleanTreeRecursive(node->left);
    cleanTreeRecursive(node->right);
    if (node->circle) { removeItem(node->circle); delete node->circle; }
    if (node->linkToParent) { removeItem(node->linkToParent); delete node->linkToParent; }
    delete node;
}

// === 1. 计算布局 (只算坐标，不动) ===
void TreeScene::calculateLayout(TreeNode* node, int x, int y, int hOffset) {
    if (!node) return;

    node->targetX = x;
    node->targetY = y;

    // 最小水平偏移量控制在 35，防止重叠
    int nextOffset = qMax(35, hOffset / 2);

    calculateLayout(node->left, x - hOffset, y + LEVEL_HEIGHT, nextOffset);
    calculateLayout(node->right, x + hOffset, y + LEVEL_HEIGHT, nextOffset);
}

// === 2. 刷新视觉 (动画移动 + 连线修复) ===
void TreeScene::refreshTreeVisuals(TreeNode* node, QPointF parentPos) {
    if (!node || !node->circle) return;

    // A. 节点移动动画
    // 注意：这里能直接用 NODE_RADIUS 是因为我们还在成员函数里，不在 lambda 里
    QPointF endPos(node->targetX - NODE_RADIUS, node->targetY - NODE_RADIUS);

    // 如果位置有变化，播放动画
    if (node->circle->pos() != endPos) {
        QVariantAnimation* anim = new QVariantAnimation(this);
        anim->setDuration(600);
        anim->setStartValue(node->circle->pos());
        anim->setEndValue(endPos);
        anim->setEasingCurve(QEasingCurve::OutCubic);

        // 修复 1：虽然这个 lambda 没用到成员变量，但为了保险可以加上 [this, node]
        connect(anim, &QVariantAnimation::valueChanged, this, [node](const QVariant& val) {
            if (node && node->circle) node->circle->setPos(val.toPointF());
            });

        // 修复 2 (关键报错点)：这里用到了 NODE_RADIUS，必须捕获 'this'
        // 修改前：[node, parentPos]
        // 修改后：[this, node, parentPos]
        connect(anim, &QVariantAnimation::valueChanged, this, [this, node, parentPos]() {
            if (node && node->linkToParent) {
                // 现在 lambda 拥有了 'this' 指针，就能看见 NODE_RADIUS 了！
                QPointF myCenter = node->circle->pos() + QPointF(NODE_RADIUS, NODE_RADIUS);
                QLineF line(parentPos, myCenter);
                node->linkToParent->setLine(line);
            }
            });

        connect(anim, &QVariantAnimation::finished, anim, &QObject::deleteLater);
        anim->start();
    }

    // ... (函数的剩余部分保持不变) ...
    // B. 连线修复 (静态或动画结束后的修正)
    if (parentPos != QPointF(-1, -1)) {
        QPointF myCenter = endPos + QPointF(NODE_RADIUS, NODE_RADIUS);
        QLineF correctLine(parentPos, myCenter);

        if (!node->linkToParent) {
            node->linkToParent = addLine(correctLine, QPen(Qt::black, 2));
            node->linkToParent->setZValue(0);
        }
        else {
            node->linkToParent->setLine(correctLine);
        }
    }
    else {
        // 根节点没有连线
        if (node->linkToParent) {
            removeItem(node->linkToParent);
            delete node->linkToParent;
            node->linkToParent = nullptr;
        }
    }

    // 递归处理子节点
    QPointF myTargetCenter(node->targetX, node->targetY);
    refreshTreeVisuals(node->left, myTargetCenter);
    refreshTreeVisuals(node->right, myTargetCenter);
}

// === 探针动画 (连续滚动版) ===
void TreeScene::animSearchPath(int targetVal, std::function<void(TreeNode*, TreeNode*, bool)> onFinished) {
    if (!m_probeHalo) {
        m_probeHalo = addEllipse(0, 0, NODE_RADIUS * 2 + 10, NODE_RADIUS * 2 + 10,
            QPen(Qt::red, 4), Qt::NoBrush);
        m_probeHalo->setZValue(999);
    }
    m_probeHalo->setVisible(true);
    m_probeHalo->setOpacity(1.0);

    // 1. 预先计算路径的所有关键点
    QList<QPointF> pathPoints;

    TreeNode* curr = root;
    TreeNode* parent = nullptr;
    bool isLeft = false;

    // 起点：根节点
    if (root) {
        pathPoints.append(root->circle->pos() - QPointF(5, 5));
    }
    else {
        pathPoints.append(QPointF(ROOT_X - NODE_RADIUS - 5, ROOT_Y - NODE_RADIUS - 5));
    }

    while (curr != nullptr) {
        if (curr->value == targetVal) break; // 找到了

        parent = curr;
        if (targetVal < curr->value) {
            curr = curr->left;
            isLeft = true;
        }
        else {
            curr = curr->right;
            isLeft = false;
        }

        if (curr) {
            pathPoints.append(curr->circle->pos() - QPointF(5, 5));
        }
    }

    // 2. 构建连续动画组
    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);

    // 如果没有路径（例如空树），直接回调
    if (pathPoints.size() <= 1 && !root) {
        onFinished(nullptr, nullptr, false);
        return;
    }

    for (int i = 0; i < pathPoints.size() - 1; ++i) {
        QVariantAnimation* move = new QVariantAnimation(group);
        move->setDuration(500); // 滚动速度
        move->setStartValue(pathPoints[i]);
        move->setEndValue(pathPoints[i + 1]);
        move->setEasingCurve(QEasingCurve::InOutQuad);

        connect(move, &QVariantAnimation::valueChanged, this, [this](const QVariant& val) {
            if (m_probeHalo) m_probeHalo->setPos(val.toPointF());
            });

        group->addAnimation(move);
        // 稍微停顿一下，模拟"思考"
        group->addPause(100);
    }

    connect(group, &QAbstractAnimation::finished, this, [this, onFinished, parent, curr, isLeft, group]() {
        onFinished(parent, curr, isLeft);
        group->deleteLater();
        });

    group->start();
}

void TreeScene::createVisualNode(TreeNode* node, int x, int y) {
    node->targetX = x;
    node->targetY = y;

    node->circle = addEllipse(0, 0, NODE_RADIUS * 2, NODE_RADIUS * 2,
        QPen(Qt::black, 2), QBrush(QColor(144, 238, 144)));
    node->circle->setPos(x - NODE_RADIUS, y - NODE_RADIUS);
    node->circle->setZValue(10);

    node->text = new QGraphicsSimpleTextItem(QString::number(node->value), node->circle);
    auto b = node->text->boundingRect();
    node->text->setPos((NODE_RADIUS * 2 - b.width()) / 2, (NODE_RADIUS * 2 - b.height()) / 2);
}

void TreeScene::insertNodeAnimated(int value, int index) {
    (void)index;

    // 空树特判
    if (!root) {
        root = new TreeNode(value);
        createVisualNode(root, ROOT_X, ROOT_Y);
        // 弹出动画
        root->circle->setScale(0);
        QVariantAnimation* anim = new QVariantAnimation(this);
        anim->setDuration(500);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutBack);
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

    animSearchPath(value, [this, value](TreeNode* parent, TreeNode* current, bool isLeft) {
        if (current != nullptr) {
            // 已存在：高亮一下
            searchNodeAnimated(value, 0);
            return;
        }

        if (m_probeHalo) m_probeHalo->setVisible(false);

        // 逻辑插入
        TreeNode* newNode = new TreeNode(value);
        if (isLeft) parent->left = newNode;
        else parent->right = newNode;

        // 1. 全局重新计算布局 (确定新节点位置 + 旧节点调整)
        calculateLayout(root, ROOT_X, ROOT_Y, 200);

        // 2. 创建可视元素
        createVisualNode(newNode, newNode->targetX, newNode->targetY);

        // 3. 初始状态：透明，位置在父节点处 (模拟从父节点长出来)
        if (parent && parent->circle) {
            newNode->circle->setPos(parent->circle->pos());
        }
        newNode->circle->setOpacity(0);

        // 4. 刷新全树 (这一步会把 newNode 移动到 targetX/Y)
        refreshTreeVisuals(root, QPointF(-1, -1));

        // 5. 额外的新节点出现动画
        QVariantAnimation* appear = new QVariantAnimation(this);
        appear->setDuration(600);
        appear->setStartValue(0.0);
        appear->setEndValue(1.0);
        connect(appear, &QVariantAnimation::valueChanged, this, [newNode](const QVariant& val) {
            if (newNode && newNode->circle) newNode->circle->setOpacity(val.toFloat());
            });
        connect(appear, &QVariantAnimation::finished, this, &BaseScene::animationFinished);
        appear->start();
        });
}

// 辅助：标记删除，将图形移入垃圾箱
void TreeScene::markForDeletion(TreeNode* node) {
    if (!node) return;
    if (node->circle) m_trashItems.append(node->circle);
    if (node->linkToParent) m_trashItems.append(node->linkToParent);
}

void TreeScene::processTrashBin() {
    if (m_trashItems.isEmpty()) return;

    // 播放淡出动画
    QVariantAnimation* anim = new QVariantAnimation(this);
    anim->setDuration(600);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);

    QList<QGraphicsItem*> items = m_trashItems;
    m_trashItems.clear();

    connect(anim, &QVariantAnimation::valueChanged, this, [items](const QVariant& val) {
        for (auto item : items) {
            if (item) item->setOpacity(val.toFloat());
        }
        });

    connect(anim, &QVariantAnimation::finished, this, [this, items, anim]() {
        for (auto item : items) {
            if (item) {
                this->removeItem(item);
                delete item;
            }
        }
        anim->deleteLater();
        });

    anim->start();
}

TreeNode* TreeScene::findMin(TreeNode* node) {
    while (node->left != nullptr) node = node->left;
    return node;
}

// 递归删除 - 关键：处理节点移动和垃圾回收
TreeNode* TreeScene::deleteNodeRecursive(TreeNode* root, int value, bool& deleted) {
    if (root == nullptr) return root;

    if (value < root->value) {
        root->left = deleteNodeRecursive(root->left, value, deleted);
    }
    else if (value > root->value) {
        root->right = deleteNodeRecursive(root->right, value, deleted);
    }
    else {
        deleted = true;

        // Case 1: 无子节点
        if (root->left == nullptr && root->right == nullptr) {
            markForDeletion(root);
            delete root;
            return nullptr;
        }
        // Case 2: 单子节点
        else if (root->left == nullptr) {
            TreeNode* temp = root->right;
            markForDeletion(root); // 删掉当前节点 visuals
            delete root;
            return temp; // 返回右孩子顶替位置
        }
        else if (root->right == nullptr) {
            TreeNode* temp = root->left;
            markForDeletion(root);
            delete root;
            return temp;
        }
        // Case 3: 双子节点
        else {
            // 找到右子树最小值
            TreeNode* temp = findMin(root->right);

            // 技巧：我们保留 root 的物理结构（视觉对象），只改值
            root->value = temp->value;
            if (root->text) root->text->setText(QString::number(root->value));

            // 递归删除那个被搬运的 temp 节点
            root->right = deleteNodeRecursive(root->right, temp->value, deleted);
        }
    }
    return root;
}

void TreeScene::removeNodeAnimated(int value, int index) {
    (void)index;

    animSearchPath(value, [this, value](TreeNode* parent, TreeNode* curr, bool) {
        if (m_probeHalo) m_probeHalo->setVisible(false);

        if (!curr && parent && parent->value != value && (!root || root->value != value)) {
            emit animationFinished();
            return;
        }

        bool deleted = false;
        // 1. 逻辑删除 (同时将废弃图形放入垃圾箱)
        root = deleteNodeRecursive(root, value, deleted);

        if (deleted) {
            // 2. 重新计算剩余节点布局
            calculateLayout(root, ROOT_X, ROOT_Y, 200);

            // 3. 播放：垃圾箱淡出 + 存活节点移动
            processTrashBin();

            // 关键：强制刷新所有存活节点的连线
            refreshTreeVisuals(root, QPointF(-1, -1));

            QTimer::singleShot(650, this, &BaseScene::animationFinished);
        }
        else {
            emit animationFinished();
        }
        });
}

void TreeScene::searchNodeAnimated(int value, int index) {
    (void)index;
    animSearchPath(value, [this](TreeNode* p, TreeNode* curr, bool) {
        if (m_probeHalo) m_probeHalo->setVisible(false);

        if (curr && curr->circle) {
            // 绿色高亮
            QBrush original = curr->circle->brush();
            curr->circle->setBrush(QBrush(Qt::green));

            QTimer::singleShot(1000, [this, curr, original]() {
                // 恢复颜色
                if (curr && curr->circle) curr->circle->setBrush(original);
                emit animationFinished();
                });
        }
        else {
            emit animationFinished();
        }
        });
}