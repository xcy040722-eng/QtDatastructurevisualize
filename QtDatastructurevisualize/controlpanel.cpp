#include "controlpanel.h"
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

ControlPanel::ControlPanel(QWidget* parent) : QWidget(parent) {
    setupUi();
    setupConnections();
}

void ControlPanel::setupUi() {
    m_structCombo = new QComboBox(this);
    m_structCombo->addItem(u8"链表 (Linked List)");
    m_structCombo->addItem(u8"顺序表 (ArrayList)");
    m_structCombo->addItem(u8"栈 (Stack)");

    m_valueEdit = new QLineEdit(this);
    m_valueEdit->setPlaceholderText(u8"输入整数值");

    m_insertBtn = new QPushButton(u8"插入", this);
    m_removeBtn = new QPushButton(u8"删除", this);
    m_findBtn = new QPushButton(u8"查找", this);
    m_resetBtn = new QPushButton(u8"重置", this);

    QVBoxLayout* main = new QVBoxLayout(this);
    main->addWidget(new QLabel(u8"数据结构类型：", this));
    main->addWidget(m_structCombo);
    main->addWidget(new QLabel(u8"节点值：", this));
    main->addWidget(m_valueEdit);

    QHBoxLayout* ops = new QHBoxLayout();
    ops->addWidget(m_insertBtn);
    ops->addWidget(m_removeBtn);
    ops->addWidget(m_findBtn);
    main->addLayout(ops);

    main->addWidget(m_resetBtn);
    main->addStretch(1);
    setFixedWidth(240);
}

void ControlPanel::setupConnections() {
    connect(m_insertBtn, &QPushButton::clicked, this, [this]() {
        QString v = m_valueEdit->text().trimmed();
        if (v.isEmpty()) { qDebug() << "请输入值"; return; }
        emit insertRequested(v);
        });
    connect(m_removeBtn, &QPushButton::clicked, this, [this]() {
        QString v = m_valueEdit->text().trimmed();
        if (v.isEmpty()) { qDebug() << "请输入值"; return; }
        emit removeRequested(v);
        });
    connect(m_findBtn, &QPushButton::clicked, this, [this]() {
        QString v = m_valueEdit->text().trimmed();
        if (v.isEmpty()) { qDebug() << "请输入值"; return; }
        emit findRequested(v);
        });
    connect(m_resetBtn, &QPushButton::clicked, this, [this]() {
        emit resetRequested();
        });
    connect(m_structCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        emit structureChanged(idx);
        });
}
