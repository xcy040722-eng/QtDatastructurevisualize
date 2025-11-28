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
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    mainLayout->addWidget(new QLabel(QStringLiteral("<b>选择数据结构:</b>")));
    m_structCombo = new QComboBox();
    m_structCombo->addItem(QStringLiteral("链表 (Linked List)"));
    m_structCombo->addItem(QStringLiteral("顺序表 (Array List)"));
    m_structCombo->addItem(QStringLiteral("栈 (Stack)"));
    mainLayout->addWidget(m_structCombo);

    mainLayout->addWidget(new QLabel(QStringLiteral("<b>节点数值:</b>")));
    m_valueEdit = new QLineEdit();
    m_valueEdit->setPlaceholderText(QStringLiteral("请输入整数"));
    mainLayout->addWidget(m_valueEdit);

    mainLayout->addWidget(new QLabel(QStringLiteral("<b>操作:</b>")));
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_insertBtn = new QPushButton(QStringLiteral("插入"));
    m_removeBtn = new QPushButton(QStringLiteral("删除"));
    m_findBtn = new QPushButton(QStringLiteral("查找"));
    btnLayout->addWidget(m_insertBtn);
    btnLayout->addWidget(m_removeBtn);
    btnLayout->addWidget(m_findBtn);
    mainLayout->addLayout(btnLayout);

    m_resetBtn = new QPushButton(QStringLiteral("清空 / 重置"));
    mainLayout->addWidget(m_resetBtn);

    mainLayout->addStretch();
    setFixedWidth(260);
}

void ControlPanel::setupConnections() {
    connect(m_insertBtn, &QPushButton::clicked, this, [this]() {
        QString v = m_valueEdit->text().trimmed();
        if (!v.isEmpty()) emit insertRequested(v);
        });
    connect(m_removeBtn, &QPushButton::clicked, this, [this]() {
        QString v = m_valueEdit->text().trimmed();
        if (!v.isEmpty()) emit removeRequested(v);
        });
    connect(m_findBtn, &QPushButton::clicked, this, [this]() {
        QString v = m_valueEdit->text().trimmed();
        if (!v.isEmpty()) emit findRequested(v);
        });
    connect(m_resetBtn, &QPushButton::clicked, this, &ControlPanel::resetRequested);

    connect(m_structCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &ControlPanel::structureChanged);
}

void ControlPanel::setButtonsEnabled(bool enable) {
    m_insertBtn->setEnabled(enable);
    m_removeBtn->setEnabled(enable);
    m_findBtn->setEnabled(enable);
    m_resetBtn->setEnabled(enable);
    m_structCombo->setEnabled(enable);

    if (enable) m_insertBtn->setText(QStringLiteral("插入"));
    else m_insertBtn->setText(QStringLiteral("动画中..."));
}