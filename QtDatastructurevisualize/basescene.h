#pragma once
#include <QGraphicsScene>

class BaseScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit BaseScene(QObject* parent = nullptr);
    virtual ~BaseScene() override = default;

    // These are called by models via Controller bindings
public slots:
    virtual void onNodeInserted(int value);
    virtual void onNodeDeleted(int value);
    virtual void onListCleared();
    virtual void setStructureType(int type); // 0: linked,1:array,2:stack
};
