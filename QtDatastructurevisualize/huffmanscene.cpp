#include "huffmanscene.h"
#include <QVariantAnimation>
#include <QSequentialAnimationGroup>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <algorithm> // for sort

HuffmanScene::HuffmanScene(QObject* parent) : BaseScene(parent) {
    setSceneRect(0, 0, 2000, 1500);
}

HuffmanScene::~HuffmanScene() {
    reset();
}

void HuffmanScene::reset() {
    for (auto node : m_forest) {
        cleanNodeRecursive(node);
    }
    m_forest.clear();

    if (m_highlight1) { removeItem(m_highlight1); delete m_highlight1; m_highlight1 = nullptr; }
    if (m_highlight2) { removeItem(m_highlight2); delete m_highlight2; m_highlight2 = nullptr; }

    update();
}

void HuffmanScene::cleanNodeRecursive(HuffNode* node) {
    if (!node) return;
    cleanNodeRecursive(node->left);
    cleanNodeRecursive(node->right);

    if (node->circle) { removeItem(node->circle); delete node->circle; }
    if (node->linkLeft) { removeItem(node->linkLeft); delete node->linkLeft; }
    if (node->linkRight) { removeItem(node->linkRight); delete node->linkRight; }
    delete node;
}

// === 1. 插入（添加叶子权重） ===
void HuffmanScene::insertNodeAnimated(int value, int index) {
    (void)index;

    // 创建新节点
    HuffNode* newNode = new HuffNode(value);

    // 初始位置：屏幕底部中间，或者上次的末尾
    // 为了动画效果，先放在屏幕下方
    createVisualNode(newNode, 500, 800);
    newNode->circle->setOpacity(0);

    m_forest.append(newNode);

    // 重新计算森林布局并播放动画
    layoutForest();
    refreshVisuals();

    // 出现动画
    QVariantAnimation* anim = new QVariantAnimation(this);
    anim->setDuration(600);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    connect(anim, &QVariantAnimation::valueChanged, this, [newNode](const QVariant& val) {
        if (newNode && newNode->circle) newNode->circle->setOpacity(val.toFloat());
        });
    connect(anim, &QVariantAnimation::finished, this, &BaseScene::animationFinished);
    anim->start();
}

// === 2. 移除（复用为：执行一步构建 / Next Step） ===
void HuffmanScene::removeNodeAnimated(int value, int index) {
    (void)value; (void)index;

    if (m_forest.size() < 2) {
        // 少于2个节点，无法合并，直接完成
        emit animationFinished();
        return;
    }

    mergeStep();
}

// === 核心逻辑：合并最小的两个 ===
void HuffmanScene::mergeStep() {
    // 1. 排序森林 (按权重从小到大)
    std::sort(m_forest.begin(), m_forest.end(), [](HuffNode* a, HuffNode* b) {
        return a->weight < b->weight;
        });

    // 2. 取出最小的两个
    HuffNode* leftChild = m_forest.takeFirst();
    HuffNode* rightChild = m_forest.takeFirst();

    // 3. 创建父节点
    HuffNode* parent = new HuffNode(leftChild->weight + rightChild->weight);
    parent->left = leftChild;
    parent->right = rightChild;

    // 创建父节点的视觉元素（初始位置设为两个子节点的中心）
    qreal startX = (leftChild->circle->pos().x() + rightChild->circle->pos().x()) / 2 + NODE_RADIUS;
    qreal startY = (leftChild->circle->pos().y() + rightChild->circle->pos().y()) / 2 + NODE_RADIUS;
    createVisualNode(parent, startX, startY);
    parent->circle->setOpacity(0); // 初始透明

    // 创建连线
    parent->linkLeft = addLine(QLineF(), QPen(Qt::black, 2));
    parent->linkLeft->setZValue(-1);
    parent->linkRight = addLine(QLineF(), QPen(Qt::black, 2));
    parent->linkRight->setZValue(-1);

    // 4. 将父节点放回森林
    m_forest.append(parent);

    // 5. 重新布局
    layoutForest();

    // 6. 动画序列
    // 阶段一：高亮选中的两个子节点 (红色框闪烁)
    // 阶段二：移动到新位置 + 父节点出现

    // 这里简化为直接移动并显示父节点
    refreshVisuals();

    // 额外的父节点淡入动画
    QVariantAnimation* anim = new QVariantAnimation(this);
    anim->setDuration(800);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    connect(anim, &QVariantAnimation::valueChanged, this, [parent](const QVariant& val) {
        if (parent && parent->circle) parent->circle->setOpacity(val.toFloat());
        if (parent && parent->linkLeft) parent->linkLeft->setOpacity(val.toFloat());
        if (parent && parent->linkRight) parent->linkRight->setOpacity(val.toFloat());
        });

    connect(anim, &QVariantAnimation::finished, this, &BaseScene::animationFinished);
    anim->start();
}

void HuffmanScene::createVisualNode(HuffNode* node, qreal x, qreal y) {
    node->targetX = x;
    node->targetY = y;

    // 区分叶子和内部节点颜色
    QColor color = (node->left || node->right) ? QColor(135, 206, 250) : QColor(255, 165, 0); // 内部蓝，叶子橙

    node->circle = addEllipse(0, 0, NODE_RADIUS * 2, NODE_RADIUS * 2,
        QPen(Qt::black, 2), QBrush(color));
    node->circle->setPos(x - NODE_RADIUS, y - NODE_RADIUS);
    node->circle->setZValue(10);

    node->text = new QGraphicsSimpleTextItem(QString::number(node->weight), node->circle);
    auto b = node->text->boundingRect();
    node->text->setPos((NODE_RADIUS * 2 - b.width()) / 2, (NODE_RADIUS * 2 - b.height()) / 2);
}

// === 布局算法 ===

qreal HuffmanScene::calculateTreeWidth(HuffNode* node) {
    if (!node) return 0;
    if (!node->left && !node->right) {
        node->treeWidth = NODE_RADIUS * 2 + GAP_X; // 叶子宽度
        return node->treeWidth;
    }
    node->treeWidth = calculateTreeWidth(node->left) + calculateTreeWidth(node->right) + GAP_X;
    return node->treeWidth;
}

void HuffmanScene::calculateSubTreeLayout(HuffNode* node, qreal x, qreal y, qreal hOffset) {
    if (!node) return;

    node->targetX = x;
    node->targetY = y;

    // 简单的递归布局：父节点位于子树宽度范围的中间
    // 这里为了哈夫曼树的特殊展示，我们采用更紧凑的方式
    // 如果有左右孩子，父节点 X 是左右孩子 X 的中心

    if (node->left && node->right) {
        qreal leftW = node->left->treeWidth;
        qreal rightW = node->right->treeWidth;

        // 左孩子位置：当前中心 - 右半宽 - 左半宽/2 ??? 
        // 简单策略：按宽度分配空间

        qreal currentStart = x - (leftW + rightW) / 2.0;
        qreal leftX = currentStart + leftW / 2.0;
        qreal rightX = currentStart + leftW + rightW / 2.0;

        calculateSubTreeLayout(node->left, leftX, y + LEVEL_HEIGHT, 0);
        calculateSubTreeLayout(node->right, rightX, y + LEVEL_HEIGHT, 0);
    }
}

void HuffmanScene::layoutForest() {
    // 1. 先算每棵树的宽度
    for (auto node : m_forest) {
        calculateTreeWidth(node);
    }

    // 2. 只有一棵树时，居中显示
    if (m_forest.size() == 1) {
        calculateSubTreeLayout(m_forest[0], 500, 60, 0);
        return;
    }

    // 3. 多棵树时，在屏幕下方一字排开 (或居中排列)
    qreal totalWidth = 0;
    for (auto node : m_forest) totalWidth += node->treeWidth;

    qreal startX = 500 - totalWidth / 2.0; // 居中起始点
    qreal currentX = startX;

    for (auto node : m_forest) {
        qreal w = node->treeWidth;
        // 树根的 X 坐标 = 当前区域起始 + 半宽
        qreal rootX = currentX + w / 2.0;
        // 树根的 Y 坐标 = 屏幕下方区域 (留出空间给合并后的树向上生长)
        // 比如固定在 Y=600，随着合并，新树也是根，也会排在这里
        // 但哈夫曼是自底向上的，所以我们把森林的根都放在同一层比较好看
        // 比如 Y = 400
        calculateSubTreeLayout(node, rootX, 400, 0);

        currentX += w;
    }
}

void HuffmanScene::refreshVisuals() {
    // 遍历森林中所有的树，移动所有节点
    // 由于 m_forest 只存根，我们需要递归遍历
    std::function<void(HuffNode*, QPointF)> updateNode =
        [&](HuffNode* node, QPointF parentPos) {
        if (!node || !node->circle) return;

        QPointF endPos(node->targetX - NODE_RADIUS, node->targetY - NODE_RADIUS);

        // 移动动画
        if (node->circle->pos() != endPos) {
            QVariantAnimation* anim = new QVariantAnimation(this);
            anim->setDuration(600);
            anim->setStartValue(node->circle->pos());
            anim->setEndValue(endPos);
            anim->setEasingCurve(QEasingCurve::InOutQuad);

            // 捕获 this
            connect(anim, &QVariantAnimation::valueChanged, this, [this, node, parentPos](const QVariant& val) {
                if (node && node->circle) node->circle->setPos(val.toPointF());

                // 实时更新与孩子的连线（这里简化，只在自己移动时更新自己作为父节点的连线？）
                // 不，连线是 Parent -> Child
                // 这里的 parentPos 是父节点的位置。
                // 还是更新 node 指向其孩子的连线比较好控制，或者由父节点控制
                // 这里的结构是 node 存了 linkLeft/linkRight 指向孩子

                QPointF myCenter = node->circle->pos() + QPointF(NODE_RADIUS, NODE_RADIUS);

                if (node->left && node->linkLeft) {
                    QPointF leftCenter = node->left->circle->pos() + QPointF(NODE_RADIUS, NODE_RADIUS);
                    node->linkLeft->setLine(QLineF(myCenter, leftCenter));
                }
                if (node->right && node->linkRight) {
                    QPointF rightCenter = node->right->circle->pos() + QPointF(NODE_RADIUS, NODE_RADIUS);
                    node->linkRight->setLine(QLineF(myCenter, rightCenter));
                }
                });
            connect(anim, &QVariantAnimation::finished, anim, &QObject::deleteLater);
            anim->start();
        }
        else {
            // 位置没变，但可能需要更新连线（如果子节点动了）
            // 简单起见，强制刷一次连线
            QPointF myCenter = endPos + QPointF(NODE_RADIUS, NODE_RADIUS);
            if (node->left && node->linkLeft) {
                QPointF leftCenter = node->left->circle->pos() + QPointF(NODE_RADIUS, NODE_RADIUS);
                node->linkLeft->setLine(QLineF(myCenter, leftCenter));
            }
            if (node->right && node->linkRight) {
                QPointF rightCenter = node->right->circle->pos() + QPointF(NODE_RADIUS, NODE_RADIUS);
                node->linkRight->setLine(QLineF(myCenter, rightCenter));
            }
        }

        QPointF myCenter(node->targetX, node->targetY);
        updateNode(node->left, myCenter);
        updateNode(node->right, myCenter);
        };

    for (auto root : m_forest) {
        updateNode(root, QPointF(-1, -1));
    }
}