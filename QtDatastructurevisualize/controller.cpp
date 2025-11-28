// controller.cpp
#include "controller.h"
#include "visualscene.h"
#include <QDebug>

Controller::Controller(VisualScene* scene, QObject* parent)
    : QObject(parent), m_scene(scene)
{
    qDebug().noquote() << u8"Controller 已创建。";
}

void Controller::onInsertRequested(const QString& value)
{
    bool ok = false;
    int v = value.toInt(&ok);
    if (!ok) {
        qDebug().noquote() << u8"插入失败：请输入整数值。";
        return;
    }

    // 暂时直接把数值当作节点唯一标识并加入场景
    if (m_scene) {
        m_scene->addNode(v);
        qDebug().noquote() << u8"Controller：已请求插入节点" << v;
    }
}

void Controller::onRemoveRequested(const QString& value)
{
    bool ok = false;
    int v = value.toInt(&ok);
    if (!ok) {
        qDebug().noquote() << u8"删除失败：请输入整数值。";
        return;
    }

    if (m_scene) {
        m_scene->removeNode(v);
        qDebug().noquote() << u8"Controller：已请求删除节点" << v;
    }
}

void Controller::onFindRequested(const QString& value)
{
    bool ok = false;
    int v = value.toInt(&ok);
    if (!ok) {
        qDebug().noquote() << u8"查找失败：请输入整数值。";
        return;
    }

    // Phase1：简单查找 —— 查看是否存在于 scene 的 map 中
    // (VisualScene 当前没有对外暴露查询接口，简化为：尝试在 scene 的 item map 中判断)
    // 为封装性考虑，这里不直接访问内部结构，而是将来由模型来判断
    qDebug().noquote() << u8"Controller：查找功能尚未实现模型支持（Phase1 占位）：" << v;
}

void Controller::onPlayRequested()
{
    qDebug().noquote() << u8"Controller：播放动画（Phase1 无动画，仅占位）";
}

void Controller::onPauseRequested()
{
    qDebug().noquote() << u8"Controller：暂停动画（Phase1 无动画，仅占位）";
}

void Controller::onResetRequested()
{
    qDebug().noquote() << u8"Controller：重置场景（Phase1）";
    if (m_scene) {
        m_scene->clearScene();
    }
}
