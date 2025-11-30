#pragma once
#include <QPointF>
#include <QColor>
#include <QString>

// 指令类型
enum class CommandType {
    CreateNode,         // 创建新节点（出现）
    MoveNode,           // 移动节点（核心：用于布局变化）
    SetParent,          // 设置父子连线（关键：解决连线问题）
    HighlightNode,      // 节点变色（提示旋转轴心）
    UpdateText,         // 更新文字（用于删除时的值替换）
    RemoveNode,         // 删除节点
    Wait,               // 停顿（用于控制节奏）
    SearchHighlight     // 搜索路径高亮
};

// 一条可视指令
struct VisualCommand {
    CommandType type;
    int nodeId;         // 操作哪个节点（使用数值作为唯一ID）

    // 参数（根据类型不同使用不同字段）
    QPointF pos;        // 目标位置 (MoveNode, CreateNode)
    int relatedId;      // 关联ID (SetParent 的父节点ID)
    QColor color;       // 颜色 (HighlightNode)
    int duration;       // 动画时长(ms)

    // 构造函数
    VisualCommand(CommandType t, int id, int time = 500)
        : type(t), nodeId(id), relatedId(-1), duration(time), color(Qt::black) {
    }
};