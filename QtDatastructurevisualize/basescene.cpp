#include "basescene.h"

BaseScene::BaseScene(QObject* parent)
    : QGraphicsScene(parent)
{
    setBackgroundBrush(Qt::white);
}

BaseScene::~BaseScene()
{
}

void BaseScene::insertNodeAnimated(int value, int index)
{
    (void)value; (void)index;
}

void BaseScene::removeNodeAnimated(int value, int index)
{
    (void)value; (void)index;
}

void BaseScene::searchNodeAnimated(int value, int index)
{
    (void)value; (void)index;
}

// Ä¬ÈÏ¿ÕÊµÏÖ
void BaseScene::traverseAnimated(int type)
{
    (void)type;
}