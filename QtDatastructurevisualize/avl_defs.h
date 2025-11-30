#pragma once
#include <QPointF>
#include <QColor>
#include <QString>

// 指令类型
enum class CommandType {
    CreateNode,         // 创建新节点
    MoveNode,           // 移动节点
    SetParent,          // 设置连线
    HighlightNode,      // 节点变色
    RemoveNode,         // 删除节点
    SearchHighlight,    // 搜索探针
    UpdateResultText,   // === 新增：更新遍历结果文字 ===
    Wait                // 停顿
};

// 一条可视指令
struct VisualCommand {
    CommandType type;
    int nodeId;

    QPointF pos;
    int relatedId;
    QColor color;
    int duration;
    QString text;       // === 新增：携带文字内容 ===

    // 构造函数
    VisualCommand(CommandType t, int id, int time = 500)
        : type(t), nodeId(id), relatedId(-1), duration(time), color(Qt::black) {
    }
};