#pragma once
// visualscene.h
#ifndef VISUALSCENE_H
#define VISUALSCENE_H

#include <QGraphicsScene>
#include <QMap>

class QGraphicsItem;

/**
 * @brief VisualScene 负责图形绘制和简单布局管理（Phase1）
 * 提供 addNode / removeNode 等接口，供 Controller 调用。
 */
class VisualScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit VisualScene(QObject* parent = nullptr);

    // 在场景末尾添加节点（简单实现，节点以 value 为唯一标识）
    void addNode(int value);
    // 删除指定节点（若存在）
    void removeNode(int value);
    // 清空场景（用于重置）
    void clearScene();

private:
    // 存储 value -> groupItem（矩形 + 文本）
    QMap<int, QGraphicsItem*> m_nodeMap;

    // 基础布局参数
    const int m_nodeWidth = 60;
    const int m_nodeHeight = 36;
    const int m_hGap = 20;

    // 重新排列所有节点位置（按插入顺序）
    void relayoutNodes();
};

#endif // VISUALSCENE_H
