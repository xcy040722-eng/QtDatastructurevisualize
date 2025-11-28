// visualscene.cpp
#include "visualscene.h"

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsItemGroup>
#include <QFont>
#include <QDebug>

VisualScene::VisualScene(QObject* parent) : QGraphicsScene(parent)
{
    // 场景背景/坐标等基础设置（可扩展）
    setBackgroundBrush(Qt::white);
    qDebug().noquote() << u8"VisualScene 已创建。";
}

void VisualScene::addNode(int value)
{
    if (m_nodeMap.contains(value)) {
        qDebug().noquote() << u8"节点已存在：" << value;
        return;
    }

    // 创建矩形和文本，组合成一个 QGraphicsItemGroup
    QGraphicsRectItem* rect = new QGraphicsRectItem(0, 0, m_nodeWidth, m_nodeHeight);
    rect->setPen(QPen(Qt::black));
    rect->setBrush(QBrush(Qt::lightGray));
    rect->setFlag(QGraphicsItem::ItemIsSelectable, false);
    rect->setFlag(QGraphicsItem::ItemIsMovable, false);

    QGraphicsTextItem* text = new QGraphicsTextItem(QString::number(value));
    QFont f;
    f.setPointSize(10);
    text->setFont(f);
    // 将文本定位到矩形中心
    QRectF r = rect->rect();
    qreal tx = r.left() + (r.width() - text->boundingRect().width()) / 2.0;
    qreal ty = r.top() + (r.height() - text->boundingRect().height()) / 2.0;
    text->setPos(tx, ty);

    QGraphicsItemGroup* group = new QGraphicsItemGroup();
    addItem(group);
    group->addToGroup(rect);
    group->addToGroup(text);

    // 把 group 放入地图
    m_nodeMap.insert(value, group);

    // 重新布局所有节点
    relayoutNodes();

    qDebug().noquote() << u8"已添加节点：" << value;
}

void VisualScene::removeNode(int value)
{
    if (!m_nodeMap.contains(value)) {
        qDebug().noquote() << u8"删除失败：节点不存在：" << value;
        return;
    }

    QGraphicsItem* group = m_nodeMap.take(value);
    // 从场景中移除并删除（会删除子项）
    removeItem(group);
    delete group;

    relayoutNodes();

    qDebug().noquote() << u8"已删除节点：" << value;
}

void VisualScene::clearScene()
{
    // 删除所有已记录的 item
    for (auto it = m_nodeMap.begin(); it != m_nodeMap.end(); ++it) {
        QGraphicsItem* itItem = it.value();
        removeItem(itItem);
        delete itItem;
    }
    m_nodeMap.clear();
    update();
    qDebug().noquote() << u8"场景已清空";
}

void VisualScene::relayoutNodes()
{
    // 简单从左到右横向排列
    int idx = 0;
    for (auto key : m_nodeMap.keys()) {
        QGraphicsItem* group = m_nodeMap.value(key);
        qreal x = idx * (m_nodeWidth + m_hGap);
        qreal y = 20; // 固定纵坐标
        group->setPos(x, y);
        idx++;
    }

    // 调整场景范围以便视图显示全部节点
    int totalWidth = qMax(800, idx * (m_nodeWidth + m_hGap));
    setSceneRect(0, 0, totalWidth, 600);
}
