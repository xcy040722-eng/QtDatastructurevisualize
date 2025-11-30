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
    updateButtonTexts(0);
}

void ControlPanel::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    mainLayout->addWidget(new QLabel(QStringLiteral("<b>选择数据结构:</b>")));
    m_structCombo = new QComboBox();
    m_structCombo->addItem(QStringLiteral("链表 (Linked List)"));
    m_structCombo->addItem(QStringLiteral("顺序表 (Array List)"));
    m_structCombo->addItem(QStringLiteral("栈 (Stack)"));
    m_structCombo->addItem(QStringLiteral("二叉搜索树 (BST)"));
    m_structCombo->addItem(QStringLiteral("哈夫曼树 (Huffman)"));

    mainLayout->addWidget(m_structCombo);

    mainLayout->addWidget(new QLabel(QStringLiteral("<b>节点数值/权重:</b>")));
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

    // === 遍历操作区域 ===
    m_traverseLabel = new QLabel(QStringLiteral("<b>树遍历:</b>"));
    mainLayout->addWidget(m_traverseLabel);

    QHBoxLayout* travLayout = new QHBoxLayout();
    m_traverseCombo = new QComboBox();
    m_traverseCombo->addItem(QStringLiteral("前序 (Pre-Order)"));
    m_traverseCombo->addItem(QStringLiteral("中序 (In-Order)"));
    m_traverseCombo->addItem(QStringLiteral("后序 (Post-Order)"));

    m_traverseBtn = new QPushButton(QStringLiteral("执行遍历"));

    travLayout->addWidget(m_traverseCombo, 1);
    travLayout->addWidget(m_traverseBtn, 0);
    mainLayout->addLayout(travLayout);

    m_resetBtn = new QPushButton(QStringLiteral("清空 / 重置"));
    mainLayout->addWidget(m_resetBtn);

    mainLayout->addStretch();

    // === 关键修改：移除了 setFixedWidth(260); ===
    // 现在宽度由 MainWindow 的 QSplitter 控制
}

void ControlPanel::setupConnections() {
    connect(m_insertBtn, &QPushButton::clicked, this, [this]() {
        QString v = m_valueEdit->text().trimmed();
        emit insertRequested(v);
        });
    connect(m_removeBtn, &QPushButton::clicked, this, [this]() {
        QString v = m_valueEdit->text().trimmed();
        emit removeRequested(v);
        });
    connect(m_findBtn, &QPushButton::clicked, this, [this]() {
        QString v = m_valueEdit->text().trimmed();
        if (!v.isEmpty()) emit findRequested(v);
        });

    connect(m_traverseBtn, &QPushButton::clicked, this, [this]() {
        emit traverseRequested(m_traverseCombo->currentIndex());
        });

    connect(m_resetBtn, &QPushButton::clicked, this, &ControlPanel::resetRequested);

    connect(m_structCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int idx) {
            updateButtonTexts(idx);
            updateUIState(idx);
            emit structureChanged(idx);
        });
}

void ControlPanel::updateButtonTexts(int index) {
    if (index == 2) { // Stack
        m_insertBtn->setText(QStringLiteral("入栈 (Push)"));
        m_removeBtn->setText(QStringLiteral("出栈 (Pop)"));
    }
    else if (index == 4) { // Huffman
        m_insertBtn->setText(QStringLiteral("添加叶子"));
        m_removeBtn->setText(QStringLiteral("执行合并"));
    }
    else {
        m_insertBtn->setText(QStringLiteral("插入"));
        m_removeBtn->setText(QStringLiteral("删除"));
    }
}

void ControlPanel::updateUIState(int index) {
    bool isTree = (index == 3);
    m_traverseLabel->setVisible(isTree);
    m_traverseCombo->setVisible(isTree);
    m_traverseBtn->setVisible(isTree);

    bool canSearch = (index != 2 && index != 4);
    m_findBtn->setEnabled(canSearch);
    m_findBtn->setText(canSearch ? QStringLiteral("查找") : QStringLiteral("查找 (不支持)"));
}

void ControlPanel::setButtonsEnabled(bool enable) {
    m_insertBtn->setEnabled(enable);
    m_removeBtn->setEnabled(enable);
    m_resetBtn->setEnabled(enable);
    m_structCombo->setEnabled(enable);
    m_traverseBtn->setEnabled(enable);

    int currentIdx = m_structCombo->currentIndex();
    if (currentIdx == 2 || currentIdx == 4) {
        m_findBtn->setEnabled(false);
    }
    else {
        m_findBtn->setEnabled(enable);
    }
}

int ControlPanel::getCurrentStructureIndex() const {
    return m_structCombo->currentIndex();
}