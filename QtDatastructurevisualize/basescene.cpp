#include "basescene.h"

// === 构造函数实现 ===
BaseScene::BaseScene(QObject* parent)
    : QGraphicsScene(parent)
{
    // 设置白色背景，防止在深色模式下看不清
    setBackgroundBrush(Qt::white);
}

// === 析构函数实现 ===
BaseScene::~BaseScene()
{
}

// === 虚函数的默认实现 ===
// 这里必须写具体代码，哪怕是空的，否则链接器会报错

void BaseScene::insertNodeAnimated(int value, int index)
{
    (void)value;
    (void)index;
}

void BaseScene::removeNodeAnimated(int value, int index)
{
    (void)value;
    (void)index;
}

void BaseScene::searchNodeAnimated(int value, int index)
{
    (void)value;
    (void)index;
}