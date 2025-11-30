#pragma once
#include <QGraphicsScene>

class BaseScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit BaseScene(QObject* parent = nullptr);
    virtual ~BaseScene();

    virtual void reset() = 0;

    virtual void insertNodeAnimated(int value, int index);
    virtual void removeNodeAnimated(int value, int index);
    virtual void searchNodeAnimated(int value, int index);

    // === 新增：遍历接口 ===
    // type: 0=PreOrder, 1=InOrder, 2=PostOrder
    virtual void traverseAnimated(int type);

signals:
    void animationFinished();
};