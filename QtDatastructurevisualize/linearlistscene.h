#pragma once
#include "basescene.h"
#include <QMap>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>

struct NodeGraphics {
    QGraphicsRectItem* rect = nullptr;
    QGraphicsSimpleTextItem* text = nullptr;
    QGraphicsLineItem* arrow = nullptr; // arrow to next
    int value = 0;
};

class LinearListScene : public BaseScene {
    Q_OBJECT
public:
    explicit LinearListScene(QObject* parent = nullptr);

    void setStructureType(int type) override;

public slots:
    void onNodeInserted(int value) override;
    void onNodeDeleted(int value) override;
    void onListCleared() override;

private:
    // maintain insertion order explicitly
    QList<int> m_order;
    QMap<int, NodeGraphics> m_items; // value -> graphics
    int m_struct = 0; // 0 linked,1 array,2 stack

    // layout params
    const int NODE_W = 80;
    const int NODE_H = 40;
    const int START_X = 40;
    const int START_Y = 120;
    const int GAP = 30;

    void layoutNodesWithAnimation();
    void simpleLayout(); // immediate
    void createNodeGraphics(int value, qreal startX = 0, qreal startY = 0);
    void removeNodeGraphics(int value);
    void rebuildArrows();
};
