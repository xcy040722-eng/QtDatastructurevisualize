#pragma once
#include <QGraphicsScene>

/**
 * @brief BaseScene 场景基类
 * 接口定义文件。注意：这里只有分号结尾的声明，绝对没有大括号 {...} 实现。
 */
class BaseScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit BaseScene(QObject* parent = nullptr);
    virtual ~BaseScene();

    // === 纯虚函数 (必须由子类实现) ===
    // 用于重置场景，清空所有图形项和数据
    virtual void reset() = 0;

    // === 虚函数 (提供默认空实现) ===
    // 插入动画接口
    virtual void insertNodeAnimated(int value, int index);

    // 删除动画接口
    virtual void removeNodeAnimated(int value, int index);

    // 查找动画接口
    virtual void searchNodeAnimated(int value, int index);

signals:
    // 信号：通知外部动画已结束，用于解锁UI交互
    void animationFinished();
};