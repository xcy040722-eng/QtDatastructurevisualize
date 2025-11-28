#pragma once

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QMap>

class LinkedListScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit LinkedListScene(QObject* parent = nullptr);

    void clearScene();

public slots:
    void onNodeInserted(int value);
    void onNodeDeleted(int value);
    void onListCleared();

private:
    void layoutNodes();
    QMap<int, QGraphicsRectItem*> m_nodeMap;
};
