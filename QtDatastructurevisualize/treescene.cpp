#include "treescene.h"
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QVariantAnimation>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QTimer>
#include <QEventLoop> 
#include <algorithm> 

TreeScene::TreeScene(QObject* parent) : BaseScene(parent) {
    setSceneRect(0, 0, 2000, 1500);
}

TreeScene::~TreeScene() {
    reset();
}

void TreeScene::setAVLMode(bool enable) {
    m_isAVL = enable;
    reset();
}

void TreeScene::reset() {
    cleanTreeRecursive(root);
    root = nullptr;
    if (m_probeHalo) { removeItem(m_probeHalo); delete m_probeHalo; m_probeHalo = nullptr; }
    if (m_resultText) { removeItem(m_resultText); delete m_resultText; m_resultText = nullptr; }
    processTrashBin();
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

// === 核心重构：全屏重绘与动画阻塞 ===

void TreeScene::clearAllLines(TreeNode* node) {
    if (!node) return;
    if (node->linkToParent) {
        removeItem(node->linkToParent);
        delete node->linkToParent;
        node->linkToParent = nullptr;
    }
    clearAllLines(node->left);
    clearAllLines(node->right);
}

void TreeScene::rebuildLines(TreeNode* node) {
    if (!node) return;

    if (node->left && node->left->circle && node->circle) {
        QPointF p1 = node->circle->pos() + QPointF(NODE_RADIUS, NODE_RADIUS);
        QPointF p2 = node->left->circle->pos() + QPointF(NODE_RADIUS, NODE_RADIUS);
        QGraphicsLineItem* line = addLine(QLineF(p1, p2), QPen(Qt::black, 2));
        line->setZValue(0);
        node->left->linkToParent = line;
    }

    if (node->right && node->right->circle && node->circle) {
        QPointF p1 = node->circle->pos() + QPointF(NODE_RADIUS, NODE_RADIUS);
        QPointF p2 = node->right->circle->pos() + QPointF(NODE_RADIUS, NODE_RADIUS);
        QGraphicsLineItem* line = addLine(QLineF(p1, p2), QPen(Qt::black, 2));
        line->setZValue(0);
        node->right->linkToParent = line;
    }

    rebuildLines(node->left);
    rebuildLines(node->right);
}

void TreeScene::createMoveAnimsRecursive(TreeNode* node, QParallelAnimationGroup* group, int duration) {
    if (!node || !node->circle) return;

    QPointF endPos(node->targetX - NODE_RADIUS, node->targetY - NODE_RADIUS);

    if (node->circle->pos() != endPos) {
        QVariantAnimation* anim = new QVariantAnimation(this);
        anim->setDuration(duration);
        anim->setStartValue(node->circle->pos());
        anim->setEndValue(endPos);
        anim->setEasingCurve(QEasingCurve::OutCubic);

        // Lambda 必须捕获 this 才能访问 NODE_RADIUS
        connect(anim, &QVariantAnimation::valueChanged, this, [this, node](const QVariant& val) {
            if (node->circle) node->circle->setPos(val.toPointF());
            QPointF myCenter = val.toPointF() + QPointF(NODE_RADIUS, NODE_RADIUS);

            if (node->linkToParent) {
                QLineF line = node->linkToParent->line();
                line.setP2(myCenter);
                node->linkToParent->setLine(line);
            }
            if (node->left && node->left->linkToParent) {
                QLineF line = node->left->linkToParent->line();
                line.setP1(myCenter);
                node->left->linkToParent->setLine(line);
            }
            if (node->right && node->right->linkToParent) {
                QLineF line = node->right->linkToParent->line();
                line.setP1(myCenter);
                node->right->linkToParent->setLine(line);
            }
            });

        group->addAnimation(anim);
    }

    createMoveAnimsRecursive(node->left, group, duration);
    createMoveAnimsRecursive(node->right, group, duration);
}

void TreeScene::updateViewAndWait(int duration) {
    if (!m_isAVL && duration < 100) return;

    calculateLayout(root, ROOT_X, ROOT_Y, 200);
    clearAllLines(root);
    rebuildLines(root);

    QParallelAnimationGroup* group = new QParallelAnimationGroup;
    createMoveAnimsRecursive(root, group, duration);

    if (group->animationCount() > 0) {
        QEventLoop loop;
        connect(group, &QAbstractAnimation::finished, &loop, &QEventLoop::quit);
        QTimer::singleShot(duration + 500, &loop, &QEventLoop::quit);
        group->start();
        loop.exec();
    }
    else {
        QEventLoop loop;
        QTimer::singleShot(duration / 2, &loop, &QEventLoop::quit);
        loop.exec();
    }
    delete group;
}

void TreeScene::calculateLayout(TreeNode* node, int x, int y, int hOffset) {
    if (!node) return;
    node->targetX = x;
    node->targetY = y;
    int nextOffset = qMax(35, hOffset / 2);
    calculateLayout(node->left, x - hOffset, y + LEVEL_HEIGHT, nextOffset);
    calculateLayout(node->right, x + hOffset, y + LEVEL_HEIGHT, nextOffset);
}


// === AVL 旋转与平衡 ===
int TreeScene::getHeight(TreeNode* node) { return node ? node->height : 0; }
int TreeScene::getBalance(TreeNode* node) { return node ? getHeight(node->left) - getHeight(node->right) : 0; }
void TreeScene::updateHeight(TreeNode* node) { if (node) node->height = 1 + std::max(getHeight(node->left), getHeight(node->right)); }

TreeNode* TreeScene::rightRotate(TreeNode* y) {
    TreeNode* x = y->left;
    TreeNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    updateHeight(y);
    updateHeight(x);
    return x;
}

TreeNode* TreeScene::leftRotate(TreeNode* x) {
    TreeNode* y = x->right;
    TreeNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    updateHeight(x);
    updateHeight(y);
    return y;
}

void TreeScene::insertAVLRecursive(TreeNode*& node, TreeNode* newNode) {
    if (!node) { node = newNode; return; }

    if (newNode->value < node->value) insertAVLRecursive(node->left, newNode);
    else if (newNode->value > node->value) insertAVLRecursive(node->right, newNode);
    else return;

    if (!m_isAVL) return;

    updateHeight(node);
    int balance = getBalance(node);

    if (balance > 1 && newNode->value < node->left->value) {
        node = rightRotate(node);
        updateViewAndWait(800); return;
    }
    if (balance < -1 && newNode->value > node->right->value) {
        node = leftRotate(node);
        updateViewAndWait(800); return;
    }
    if (balance > 1 && newNode->value > node->left->value) {
        node->left = leftRotate(node->left);
        updateViewAndWait(800);
        node = rightRotate(node);
        updateViewAndWait(800); return;
    }
    if (balance < -1 && newNode->value < node->right->value) {
        node->right = rightRotate(node->right);
        updateViewAndWait(800);
        node = leftRotate(node);
        updateViewAndWait(800); return;
    }
}

TreeNode* TreeScene::deleteNodeRecursive(TreeNode*& node, int value, bool& deleted) {
    if (!node) return node;

    if (value < node->value) deleteNodeRecursive(node->left, value, deleted);
    else if (value > node->value) deleteNodeRecursive(node->right, value, deleted);
    else {
        deleted = true;
        if (node->left == nullptr || node->right == nullptr) {
            TreeNode* temp = node->left ? node->left : node->right;
            if (temp == nullptr) { markForDeletion(node); node = nullptr; }
            else { markForDeletion(node); node = temp; }
        }
        else {
            TreeNode* temp = findMin(node->right);
            node->value = temp->value;
            if (node->text) node->text->setText(QString::number(node->value));
            deleteNodeRecursive(node->right, temp->value, deleted);
        }
    }

    if (!node || !m_isAVL) return node;

    updateHeight(node);
    int balance = getBalance(node);

    if (balance > 1 && getBalance(node->left) >= 0) {
        node = rightRotate(node);
        updateViewAndWait(800); return node;
    }
    if (balance > 1 && getBalance(node->left) < 0) {
        node->left = leftRotate(node->left);
        updateViewAndWait(800);
        node = rightRotate(node);
        updateViewAndWait(800); return node;
    }
    if (balance < -1 && getBalance(node->right) <= 0) {
        node = leftRotate(node);
        updateViewAndWait(800); return node;
    }
    if (balance < -1 && getBalance(node->right) > 0) {
        node->right = rightRotate(node->right);
        updateViewAndWait(800);
        node = leftRotate(node);
        updateViewAndWait(800); return node;
    }
    return node;
}

// === 操作入口 ===

void TreeScene::insertNodeAnimated(int value, int index) {
    (void)index;

    if (!root) {
        root = new TreeNode(value);
        createVisualNode(root, ROOT_X, ROOT_Y);
        root->circle->setScale(0);
        QVariantAnimation* anim = new QVariantAnimation(this);
        anim->setDuration(500); anim->setStartValue(0.0); anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutBack);
        connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant& val) {
            if (root && root->circle) { root->circle->setScale(val.toFloat()); root->circle->setTransformOriginPoint(NODE_RADIUS, NODE_RADIUS); }
            });
        connect(anim, &QVariantAnimation::finished, this, &BaseScene::animationFinished);
        anim->start();
        return;
    }

    animSearchPath(value, [this, value](TreeNode* parent, TreeNode* current, bool isLeft) {
        if (current != nullptr) { searchNodeAnimated(value, 0); return; }
        if (m_probeHalo) m_probeHalo->setVisible(false);

        TreeNode* newNode = new TreeNode(value);
        createVisualNode(newNode, 0, 0);
        if (parent && parent->circle) newNode->circle->setPos(parent->circle->pos());
        newNode->circle->setOpacity(1.0); newNode->circle->setScale(0.1);

        if (isLeft) parent->left = newNode; else parent->right = newNode;

        QEventLoop loop;
        QVariantAnimation* appear = new QVariantAnimation(this);
        appear->setDuration(300); appear->setStartValue(0.1); appear->setEndValue(1.0);
        connect(appear, &QVariantAnimation::valueChanged, this, [this, newNode](const QVariant& val) {
            if (newNode && newNode->circle) { newNode->circle->setScale(val.toFloat()); newNode->circle->setTransformOriginPoint(NODE_RADIUS, NODE_RADIUS); }
            });
        connect(appear, &QAbstractAnimation::finished, &loop, &QEventLoop::quit);
        appear->start();
        loop.exec();

        if (m_isAVL) {
            if (isLeft) parent->left = nullptr; else parent->right = nullptr;
            insertAVLRecursive(root, newNode);
        }

        updateViewAndWait(600);
        emit animationFinished();
        });
}

void TreeScene::removeNodeAnimated(int value, int index) {
    (void)index;
    animSearchPath(value, [this, value](TreeNode*, TreeNode*, bool) {
        if (m_probeHalo) m_probeHalo->setVisible(false);
        bool deleted = false;
        deleteNodeRecursive(root, value, deleted);
        if (deleted) {
            processTrashBin();
            updateViewAndWait(600);
        }
        emit animationFinished();
        });
}

// [剩余的查找、遍历、辅助函数保持不变，为了完整性这里列出关键实现]
void TreeScene::animSearchPath(int targetVal, std::function<void(TreeNode*, TreeNode*, bool)> onFinished) {
    if (!m_probeHalo) { m_probeHalo = addEllipse(0, 0, NODE_RADIUS * 2 + 10, NODE_RADIUS * 2 + 10, QPen(Qt::red, 4), Qt::NoBrush); m_probeHalo->setZValue(999); }
    QList<QPointF> pathPoints; TreeNode* curr = root; TreeNode* parent = nullptr; bool isLeft = false;
    if (root) pathPoints.append(root->circle->pos() - QPointF(5, 5)); else pathPoints.append(QPointF(ROOT_X - NODE_RADIUS - 5, ROOT_Y - NODE_RADIUS - 5));
    while (curr != nullptr) {
        if (curr->value == targetVal) break;
        parent = curr;
        if (targetVal < curr->value) { curr = curr->left; isLeft = true; }
        else { curr = curr->right; isLeft = false; }
        if (curr) pathPoints.append(curr->circle->pos() - QPointF(5, 5));
    }
    if (!pathPoints.isEmpty()) m_probeHalo->setPos(pathPoints.first()); m_probeHalo->setVisible(true); m_probeHalo->setOpacity(1.0);
    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);
    if (pathPoints.size() <= 1 && !root) { QTimer::singleShot(100, [onFinished, parent, curr, isLeft]() { onFinished(parent, curr, isLeft); }); group->deleteLater(); return; }
    for (int i = 0; i < pathPoints.size() - 1; ++i) {
        QVariantAnimation* move = new QVariantAnimation(group);
        move->setDuration(500); move->setStartValue(pathPoints[i]); move->setEndValue(pathPoints[i + 1]); move->setEasingCurve(QEasingCurve::InOutQuad);
        connect(move, &QVariantAnimation::valueChanged, this, [this](const QVariant& val) { if (m_probeHalo) m_probeHalo->setPos(val.toPointF()); });
        group->addAnimation(move); group->addPause(100);
    }
    connect(group, &QAbstractAnimation::finished, this, [this, onFinished, parent, curr, isLeft, group]() { onFinished(parent, curr, isLeft); group->deleteLater(); });
    group->start();
}

void TreeScene::createVisualNode(TreeNode* node, int x, int y) {
    node->targetX = x; node->targetY = y;
    node->circle = addEllipse(0, 0, NODE_RADIUS * 2, NODE_RADIUS * 2, QPen(Qt::black, 2), QBrush(QColor(144, 238, 144)));
    node->circle->setPos(x - NODE_RADIUS, y - NODE_RADIUS); node->circle->setZValue(10);
    node->text = new QGraphicsSimpleTextItem(QString::number(node->value), node->circle);
    auto b = node->text->boundingRect();
    node->text->setPos((NODE_RADIUS * 2 - b.width()) / 2, (NODE_RADIUS * 2 - b.height()) / 2);
}

void TreeScene::searchNodeAnimated(int value, int index) {
    (void)index;
    animSearchPath(value, [this, value](TreeNode*, TreeNode*, bool) {
        TreeNode* curr = root;
        while (curr) { if (curr->value == value) break; if (value < curr->value) curr = curr->left; else curr = curr->right; }
        if (m_probeHalo) m_probeHalo->setVisible(false);
        if (curr && curr->circle) {
            QBrush original = curr->circle->brush(); curr->circle->setBrush(QBrush(Qt::green));
            QTimer::singleShot(1000, [this, curr, original]() { if (curr && curr->circle) curr->circle->setBrush(original); emit animationFinished(); });
        }
        else { emit animationFinished(); }
        });
}

// === 遍历、辅助与垃圾回收 ===
void TreeScene::traverseAnimated(int type) {
    if (!root) { emit animationFinished(); return; }
    if (m_resultText) { removeItem(m_resultText); delete m_resultText; }
    m_resultText = new QGraphicsSimpleTextItem();
    QFont font; font.setPointSize(12); font.setBold(true);
    m_resultText->setFont(font); m_resultText->setBrush(Qt::blue); m_resultText->setPos(20, 20); addItem(m_resultText);
    QString typeName = (type == 0) ? "前序: " : (type == 1 ? "中序: " : "后序: ");
    m_resultText->setText(typeName);
    if (!m_probeHalo) { m_probeHalo = addEllipse(0, 0, NODE_RADIUS * 2 + 10, NODE_RADIUS * 2 + 10, QPen(Qt::red, 4), Qt::NoBrush); m_probeHalo->setZValue(999); }
    m_probeHalo->setVisible(true);
    TreeNode* first = getFirstNode(root, type);
    if (first && first->circle) m_probeHalo->setPos(first->circle->pos() - QPointF(5, 5)); else m_probeHalo->setPos(root->circle->pos() - QPointF(5, 5));
    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);
    QString str = typeName; buildTraversalAnim(group, root, type, str);
    connect(group, &QAbstractAnimation::finished, this, [this, group]() { m_probeHalo->setVisible(false); emit animationFinished(); group->deleteLater(); });
    group->start();
}

TreeNode* TreeScene::getFirstNode(TreeNode* node, int type) {
    if (!node) return nullptr;
    if (type == 0) return node;
    if (type == 1) return node->left ? getFirstNode(node->left, 1) : node;
    if (type == 2) { if (node->left) return getFirstNode(node->left, 2); if (node->right) return getFirstNode(node->right, 2); return node; }
    return node;
}

void TreeScene::addVisitAnim(QSequentialAnimationGroup* group, TreeNode* node, QString& currentStr) {
    QVariantAnimation* visit = new QVariantAnimation(group);
    visit->setDuration(600); visit->setKeyValueAt(0.0, 0); visit->setKeyValueAt(0.2, 1); visit->setKeyValueAt(0.8, 1); visit->setKeyValueAt(1.0, 0);
    QBrush original = node->circle->brush(); QBrush visitBrush(QColor(255, 165, 0));
    if (!currentStr.endsWith(": ")) currentStr += " -> "; currentStr += QString::number(node->value); QString displayStr = currentStr;
    connect(visit, &QVariantAnimation::valueChanged, this, [this, node, visitBrush, original](const QVariant& val) {
        if (val.toInt() == 1) node->circle->setBrush(visitBrush); else node->circle->setBrush(original);
        if (m_probeHalo) m_probeHalo->setPos(node->circle->pos() - QPointF(5, 5));
        });
    connect(visit, &QVariantAnimation::stateChanged, this, [this, displayStr](QAbstractAnimation::State ns, QAbstractAnimation::State) { if (ns == QAbstractAnimation::Running && m_resultText) m_resultText->setText(displayStr); });
    group->addAnimation(visit);
}

void TreeScene::buildTraversalAnim(QSequentialAnimationGroup* group, TreeNode* node, int type, QString& resultString) {
    if (!node) return;
    if (type == 0) addVisitAnim(group, node, resultString);
    if (node->left) buildTraversalAnim(group, node->left, type, resultString);
    if (type == 1) addVisitAnim(group, node, resultString);
    if (node->right) buildTraversalAnim(group, node->right, type, resultString);
    if (type == 2) addVisitAnim(group, node, resultString);
}

void TreeScene::markForDeletion(TreeNode* node) {
    if (!node) return;
    if (node->circle) m_trashItems.append(node->circle);
    if (node->linkToParent) m_trashItems.append(node->linkToParent);
}

void TreeScene::processTrashBin() {
    if (m_trashItems.isEmpty()) return;
    QVariantAnimation* anim = new QVariantAnimation(this);
    anim->setDuration(600); anim->setStartValue(1.0); anim->setEndValue(0.0);
    QList<QGraphicsItem*> items = m_trashItems; m_trashItems.clear();
    connect(anim, &QVariantAnimation::valueChanged, this, [items](const QVariant& val) { for (auto item : items) if (item) item->setOpacity(val.toFloat()); });
    connect(anim, &QVariantAnimation::finished, this, [this, items, anim]() { for (auto item : items) { if (item) { this->removeItem(item); delete item; } } anim->deleteLater(); });
    anim->start();
}

TreeNode* TreeScene::findMin(TreeNode* node) { while (node->left) node = node->left; return node; }