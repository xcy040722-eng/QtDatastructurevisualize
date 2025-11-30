#include "treescene.h"
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QVariantAnimation>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QTimer>

TreeScene::TreeScene(QObject* parent) : BaseScene(parent) {
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
    if (m_resultText) {
        removeItem(m_resultText);
        delete m_resultText;
        m_resultText = nullptr;
    }

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

void TreeScene::calculateLayout(TreeNode* node, int x, int y, int hOffset) {
    if (!node) return;
    node->targetX = x;
    node->targetY = y;
    int nextOffset = qMax(35, hOffset / 2);
    calculateLayout(node->left, x - hOffset, y + LEVEL_HEIGHT, nextOffset);
    calculateLayout(node->right, x + hOffset, y + LEVEL_HEIGHT, nextOffset);
}

void TreeScene::refreshTreeVisuals(TreeNode* node, QPointF parentPos) {
    if (!node || !node->circle) return;

    QPointF endPos(node->targetX - NODE_RADIUS, node->targetY - NODE_RADIUS);

    if (node->circle->pos() != endPos) {
        QVariantAnimation* anim = new QVariantAnimation(this);
        anim->setDuration(600);
        anim->setStartValue(node->circle->pos());
        anim->setEndValue(endPos);
        anim->setEasingCurve(QEasingCurve::OutCubic);

        connect(anim, &QVariantAnimation::valueChanged, this, [this, node](const QVariant& val) {
            if (node && node->circle) node->circle->setPos(val.toPointF());
            });

        connect(anim, &QVariantAnimation::valueChanged, this, [this, node, parentPos]() {
            if (node && node->linkToParent) {
                QPointF myCenter = node->circle->pos() + QPointF(NODE_RADIUS, NODE_RADIUS);
                QLineF line(parentPos, myCenter);
                node->linkToParent->setLine(line);
            }
            });

        connect(anim, &QVariantAnimation::finished, anim, &QObject::deleteLater);
        anim->start();
    }

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
        if (node->linkToParent) {
            removeItem(node->linkToParent);
            delete node->linkToParent;
            node->linkToParent = nullptr;
        }
    }

    QPointF myTargetCenter(node->targetX, node->targetY);
    refreshTreeVisuals(node->left, myTargetCenter);
    refreshTreeVisuals(node->right, myTargetCenter);
}

// === 探针动画 ===
void TreeScene::animSearchPath(int targetVal, std::function<void(TreeNode*, TreeNode*, bool)> onFinished) {
    if (!m_probeHalo) {
        m_probeHalo = addEllipse(0, 0, NODE_RADIUS * 2 + 10, NODE_RADIUS * 2 + 10,
            QPen(Qt::red, 4), Qt::NoBrush);
        m_probeHalo->setZValue(999);
    }

    QList<QPointF> pathPoints;
    TreeNode* curr = root;
    TreeNode* parent = nullptr;
    bool isLeft = false;

    if (root) {
        pathPoints.append(root->circle->pos() - QPointF(5, 5));
    }
    else {
        pathPoints.append(QPointF(ROOT_X - NODE_RADIUS - 5, ROOT_Y - NODE_RADIUS - 5));
    }

    while (curr != nullptr) {
        if (curr->value == targetVal) break;

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

    if (!pathPoints.isEmpty()) {
        m_probeHalo->setPos(pathPoints.first());
    }
    m_probeHalo->setVisible(true);
    m_probeHalo->setOpacity(1.0);

    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);

    if (pathPoints.size() <= 1 && !root) {
        QTimer::singleShot(100, [onFinished, parent, curr, isLeft]() {
            onFinished(parent, curr, isLeft);
            });
        group->deleteLater();
        return;
    }

    for (int i = 0; i < pathPoints.size() - 1; ++i) {
        QVariantAnimation* move = new QVariantAnimation(group);
        move->setDuration(500);
        move->setStartValue(pathPoints[i]);
        move->setEndValue(pathPoints[i + 1]);
        move->setEasingCurve(QEasingCurve::InOutQuad);

        connect(move, &QVariantAnimation::valueChanged, this, [this](const QVariant& val) {
            if (m_probeHalo) m_probeHalo->setPos(val.toPointF());
            });

        group->addAnimation(move);
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

// === 核心：遍历动画实现 (修复箭头逻辑) ===

void TreeScene::addVisitAnim(QSequentialAnimationGroup* group, TreeNode* node, QString& currentStr) {
    // 1. 节点变色高亮动画
    QVariantAnimation* visit = new QVariantAnimation(group);
    visit->setDuration(600);
    visit->setKeyValueAt(0.0, 0);
    visit->setKeyValueAt(0.2, 1);
    visit->setKeyValueAt(0.8, 1);
    visit->setKeyValueAt(1.0, 0);

    QBrush original = node->circle->brush();
    QBrush visitBrush(QColor(255, 165, 0)); // 橙色

    // === 修复点：更智能的字符串拼接 ===
    // 如果 currentStr 以 ": " 结尾 (例如 "前序遍历: ")，说明是第一个元素，不加箭头
    // 否则，先加箭头，再加数值
    if (!currentStr.endsWith(": ")) {
        currentStr += " -> ";
    }
    currentStr += QString::number(node->value);

    QString displayStr = currentStr;

    connect(visit, &QVariantAnimation::valueChanged, this, [this, node, visitBrush, original](const QVariant& val) {
        if (val.toInt() == 1) node->circle->setBrush(visitBrush);
        else node->circle->setBrush(original);

        if (m_probeHalo) m_probeHalo->setPos(node->circle->pos() - QPointF(5, 5));
        });

    connect(visit, &QVariantAnimation::stateChanged, this, [this, displayStr](QAbstractAnimation::State newState, QAbstractAnimation::State) {
        if (newState == QAbstractAnimation::Running && m_resultText) {
            m_resultText->setText(displayStr);
        }
        });

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

void TreeScene::traverseAnimated(int type) {
    if (!root) {
        emit animationFinished();
        return;
    }

    if (m_resultText) { removeItem(m_resultText); delete m_resultText; }
    m_resultText = new QGraphicsSimpleTextItem();
    QFont font; font.setPointSize(12); font.setBold(true);
    m_resultText->setFont(font);
    m_resultText->setBrush(Qt::blue);
    m_resultText->setPos(20, 20);
    addItem(m_resultText);

    QString typeName;
    if (type == 0) typeName = "前序遍历: ";
    else if (type == 1) typeName = "中序遍历: ";
    else typeName = "后序遍历: ";

    m_resultText->setText(typeName);

    if (!m_probeHalo) {
        m_probeHalo = addEllipse(0, 0, NODE_RADIUS * 2 + 10, NODE_RADIUS * 2 + 10, QPen(Qt::red, 4), Qt::NoBrush);
        m_probeHalo->setZValue(999);
    }
    m_probeHalo->setVisible(true);
    m_probeHalo->setPos(root->circle->pos() - QPointF(5, 5));

    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);
    QString currentStr = typeName;

    buildTraversalAnim(group, root, type, currentStr);

    connect(group, &QAbstractAnimation::finished, this, [this, group]() {
        m_probeHalo->setVisible(false);
        emit animationFinished();
        group->deleteLater();
        });

    group->start();
}

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
        if (isLeft) parent->left = newNode; else parent->right = newNode;
        calculateLayout(root, ROOT_X, ROOT_Y, 200);
        createVisualNode(newNode, newNode->targetX, newNode->targetY);
        if (parent && parent->circle) newNode->circle->setPos(parent->circle->pos());
        newNode->circle->setOpacity(0);
        refreshTreeVisuals(root, QPointF(-1, -1));
        QVariantAnimation* appear = new QVariantAnimation(this);
        appear->setDuration(600); appear->setStartValue(0.0); appear->setEndValue(1.0);
        connect(appear, &QVariantAnimation::valueChanged, this, [newNode](const QVariant& val) {
            if (newNode && newNode->circle) newNode->circle->setOpacity(val.toFloat());
            });
        connect(appear, &QVariantAnimation::finished, this, &BaseScene::animationFinished);
        appear->start();
        });
}

void TreeScene::removeNodeAnimated(int value, int index) {
    (void)index;
    animSearchPath(value, [this, value](TreeNode* parent, TreeNode* curr, bool) {
        if (m_probeHalo) m_probeHalo->setVisible(false);
        if (!curr && parent && parent->value != value && (!root || root->value != value)) { emit animationFinished(); return; }
        bool deleted = false;
        root = deleteNodeRecursive(root, value, deleted);
        if (deleted) {
            calculateLayout(root, ROOT_X, ROOT_Y, 200);
            processTrashBin();
            refreshTreeVisuals(root, QPointF(-1, -1));
            QTimer::singleShot(650, this, &BaseScene::animationFinished);
        }
        else { emit animationFinished(); }
        });
}

void TreeScene::searchNodeAnimated(int value, int index) {
    (void)index;
    animSearchPath(value, [this](TreeNode* p, TreeNode* curr, bool) {
        if (m_probeHalo) m_probeHalo->setVisible(false);
        if (curr && curr->circle) {
            QBrush original = curr->circle->brush();
            curr->circle->setBrush(QBrush(Qt::green));
            QTimer::singleShot(1000, [this, curr, original]() {
                if (curr && curr->circle) curr->circle->setBrush(original);
                emit animationFinished();
                });
        }
        else { emit animationFinished(); }
        });
}

TreeNode* TreeScene::findMin(TreeNode* node) {
    while (node->left != nullptr) node = node->left;
    return node;
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
    QList<QGraphicsItem*> items = m_trashItems;
    m_trashItems.clear();
    connect(anim, &QVariantAnimation::valueChanged, this, [items](const QVariant& val) {
        for (auto item : items) if (item) item->setOpacity(val.toFloat());
        });
    connect(anim, &QVariantAnimation::finished, this, [this, items, anim]() {
        for (auto item : items) { if (item) { this->removeItem(item); delete item; } }
        anim->deleteLater();
        });
    anim->start();
}

TreeNode* TreeScene::deleteNodeRecursive(TreeNode* root, int value, bool& deleted) {
    if (root == nullptr) return root;
    if (value < root->value) root->left = deleteNodeRecursive(root->left, value, deleted);
    else if (value > root->value) root->right = deleteNodeRecursive(root->right, value, deleted);
    else {
        deleted = true;
        if (root->left == nullptr && root->right == nullptr) { markForDeletion(root); delete root; return nullptr; }
        else if (root->left == nullptr) { TreeNode* temp = root->right; markForDeletion(root); delete root; return temp; }
        else if (root->right == nullptr) { TreeNode* temp = root->left; markForDeletion(root); delete root; return temp; }
        else {
            TreeNode* temp = findMin(root->right);
            root->value = temp->value;
            if (root->text) root->text->setText(QString::number(root->value));
            root->right = deleteNodeRecursive(root->right, temp->value, deleted);
        }
    }
    return root;
}