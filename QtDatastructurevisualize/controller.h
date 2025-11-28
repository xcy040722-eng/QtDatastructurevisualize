#pragma once
// controller.h
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>

class VisualScene;

/**
 * @brief Controller 负责在 Phase1 中作为界面与场景间的中转层
 * 未来会扩展为：处理 DSModel 的变更、生成 AnimationStep 等。
 */
class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(VisualScene* scene, QObject* parent = nullptr);

public slots:
    // 来自 ControlPanel 的信号
    void onInsertRequested(const QString& value);
    void onRemoveRequested(const QString& value);
    void onFindRequested(const QString& value);

    void onPlayRequested();
    void onPauseRequested();
    void onResetRequested();

private:
    VisualScene* m_scene = nullptr;
};

#endif // CONTROLLER_H
