// controlpanel.cpp
#include "controlpanel.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QDebug>

ControlPanel::ControlPanel(QWidget* parent) : QWidget(parent)
{
    setupUi();
    setupConnections();

    qDebug().noquote() << u8"ControlPanel 已初始化。";
}

void ControlPanel::setupUi()
{
    m_structCombo = new QComboBox(this);
    m_structCombo->addItem(QStringLiteral("链表"));
    m_structCombo->addItem(QStringLiteral("顺序表"));
    m_structCombo->addItem(QStringLiteral("栈"));
    m_structCombo->addItem(QStringLiteral("队列"));
    m_structCombo->setCurrentIndex(0);

    m_valueEdit = new QLineEdit(this);
    m_valueEdit->setPlaceholderText(QStringLiteral("输入节点值，例如 5"));

    m_insertBtn = new QPushButton(QStringLiteral("插入"), this);
    m_removeBtn = new QPushButton(QStringLiteral("删除"), this);
    m_findBtn = new QPushButton(QStringLiteral("查找"), this);

    m_playBtn = new QPushButton(QStringLiteral("播放"), this);
    m_pauseBtn = new QPushButton(QStringLiteral("暂停"), this);
    m_resetBtn = new QPushButton(QStringLiteral("重置"), this);

    m_speedSlider = new QSlider(Qt::Horizontal, this);
    m_speedSlider->setRange(1, 100);
    m_speedSlider->setValue(50);

    // 布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(new QLabel(QStringLiteral("数据结构类型："), this));
    mainLayout->addWidget(m_structCombo);

    mainLayout->addWidget(new QLabel(QStringLiteral("节点值："), this));
    mainLayout->addWidget(m_valueEdit);

    QHBoxLayout* opLayout = new QHBoxLayout();
    opLayout->addWidget(m_insertBtn);
    opLayout->addWidget(m_removeBtn);
    opLayout->addWidget(m_findBtn);
    mainLayout->addLayout(opLayout);

    QHBoxLayout* ctrlLayout = new QHBoxLayout();
    ctrlLayout->addWidget(m_playBtn);
    ctrlLayout->addWidget(m_pauseBtn);
    ctrlLayout->addWidget(m_resetBtn);
    mainLayout->addLayout(ctrlLayout);

    mainLayout->addWidget(new QLabel(QStringLiteral("动画速度："), this));
    mainLayout->addWidget(m_speedSlider);

    // 占位，推到底部
    mainLayout->addStretch(1);

    setLayout(mainLayout);

    // 设置固定宽度（左侧控制面板）
    setFixedWidth(220);
}

void ControlPanel::setupConnections()
{
    connect(m_insertBtn, &QPushButton::clicked, this, [this]() {
        QString val = m_valueEdit->text().trimmed();
        if (val.isEmpty()) {
            qDebug().noquote() << u8"插入操作：请输入有效节点值";
            return;
        }
        qDebug().noquote() << u8"插入请求：" << val;
        emit insertRequested(val);
        });

    connect(m_removeBtn, &QPushButton::clicked, this, [this]() {
        QString val = m_valueEdit->text().trimmed();
        if (val.isEmpty()) {
            qDebug().noquote() << u8"删除操作：请输入有效节点值";
            return;
        }
        qDebug().noquote() << u8"删除请求：" << val;
        emit removeRequested(val);
        });

    connect(m_findBtn, &QPushButton::clicked, this, [this]() {
        QString val = m_valueEdit->text().trimmed();
        if (val.isEmpty()) {
            qDebug().noquote() << u8"查找操作：请输入有效节点值";
            return;
        }
        qDebug().noquote() << u8"查找请求：" << val;
        emit findRequested(val);
        });

    connect(m_playBtn, &QPushButton::clicked, this, [this]() {
        qDebug().noquote() << u8"播放动画";
        emit playRequested();
        });

    connect(m_pauseBtn, &QPushButton::clicked, this, [this]() {
        qDebug().noquote() << u8"暂停动画";
        emit pauseRequested();
        });

    connect(m_resetBtn, &QPushButton::clicked, this, [this]() {
        qDebug().noquote() << u8"重置场景";
        emit resetRequested();
        });

    connect(m_speedSlider, &QSlider::valueChanged, this, [this](int v) {
        qDebug().noquote() << u8"动画速度调整：" << v;
        emit speedChanged(v);
        });
}
