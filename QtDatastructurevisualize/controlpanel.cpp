#include "controlpanel.h"
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QFrame>
#include <QMessageBox>

ControlPanel::ControlPanel(QWidget* parent) : QWidget(parent) {
    setupUi();
    setupConnections();
    updateButtonTexts(0);
    updateUIState(0);
}

void ControlPanel::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    mainLayout->addWidget(new QLabel(QStringLiteral("<b>选择数据结构:</b>")));
    m_structCombo = new QComboBox();
    m_structCombo->addItem(QStringLiteral("链表 (Linked List)"));
    m_structCombo->addItem(QStringLiteral("顺序表 (Array List)"));
    m_structCombo->addItem(QStringLiteral("栈 (Stack)"));
    m_structCombo->addItem(QStringLiteral("二叉搜索树 (BST)"));
    // === 新增：AVL ===
    m_structCombo->addItem(QStringLiteral("平衡二叉树 (AVL)"));
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

    mainLayout->addWidget(new QLabel(QStringLiteral("<b>系统功能:</b>")));
    QHBoxLayout* sysLayout = new QHBoxLayout();
    m_saveBtn = new QPushButton(QStringLiteral("保存"));
    m_loadBtn = new QPushButton(QStringLiteral("打开"));
    m_resetBtn = new QPushButton(QStringLiteral("重置"));
    sysLayout->addWidget(m_saveBtn);
    sysLayout->addWidget(m_loadBtn);
    sysLayout->addWidget(m_resetBtn);
    mainLayout->addLayout(sysLayout);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);

    QHBoxLayout* dslHeader = new QHBoxLayout();
    dslHeader->addWidget(new QLabel(QStringLiteral("<b>智能交互 / DSL:</b>")));
    m_helpBtn = new QPushButton("?");
    m_helpBtn->setFixedSize(24, 24);
    m_helpBtn->setToolTip("DSL Help");
    m_helpBtn->setStyleSheet("QPushButton { border-radius: 12px; background-color: #ddd; font-weight: bold; }");
    dslHeader->addStretch();
    dslHeader->addWidget(m_helpBtn);
    mainLayout->addLayout(dslHeader);

    QHBoxLayout* cmdLayout = new QHBoxLayout();
    m_cmdLine = new QLineEdit();
    m_cmdLine->setPlaceholderText("输入指令 或 自然语言(点AI)");
    m_cmdLine->setStyleSheet("QLineEdit { background-color: #333; color: #0f0; font-family: Consolas; border: 1px solid #555; padding: 4px; }");

    m_aiBtn = new QPushButton("AI ✨");
    m_aiBtn->setFixedWidth(50);
    m_aiBtn->setStyleSheet("QPushButton { background-color: #6a0dad; color: white; font-weight: bold; border: none; border-radius: 4px; } QPushButton:hover { background-color: #8a2be2; }");

    cmdLayout->addWidget(m_cmdLine);
    cmdLayout->addWidget(m_aiBtn);
    mainLayout->addLayout(cmdLayout);

    mainLayout->addStretch();
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
    connect(m_saveBtn, &QPushButton::clicked, this, &ControlPanel::saveRequested);
    connect(m_loadBtn, &QPushButton::clicked, this, &ControlPanel::loadRequested);

    connect(m_cmdLine, &QLineEdit::returnPressed, this, [this]() {
        QString cmd = m_cmdLine->text().trimmed();
        if (!cmd.isEmpty()) {
            emit commandEntered(cmd);
            m_cmdLine->clear();
        }
        });

    connect(m_helpBtn, &QPushButton::clicked, this, &ControlPanel::showDslHelp);
    connect(m_aiBtn, &QPushButton::clicked, this, [this]() {
        if (!m_cmdLine->text().trimmed().isEmpty()) {
            emit askAiRequested();
        }
        });

    connect(m_structCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int idx) {
            updateButtonTexts(idx);
            updateUIState(idx);
            emit structureChanged(idx);
        });
}

void ControlPanel::showDslHelp() {
    QString helpText = "DSL 指令帮助:\nnew avl [1,2,3]\nnew bst [1,2,3]\ninsert 100\ndelete 20\ntraverse in\n...";
    QMessageBox::information(this, QStringLiteral("指令帮助"), helpText);
}

QString ControlPanel::getCommandText() const { return m_cmdLine->text().trimmed(); }
void ControlPanel::clearCommandText() { m_cmdLine->clear(); }
void ControlPanel::setCommandText(const QString& text) { m_cmdLine->setText(text); }

void ControlPanel::updateButtonTexts(int index) {
    if (index == 2) {
        m_insertBtn->setText(QStringLiteral("入栈 (Push)"));
        m_removeBtn->setText(QStringLiteral("出栈 (Pop)"));
    }
    else if (index == 5) { // Huffman is now 5
        m_insertBtn->setText(QStringLiteral("添加叶子"));
        m_removeBtn->setText(QStringLiteral("执行合并"));
    }
    else {
        m_insertBtn->setText(QStringLiteral("插入"));
        m_removeBtn->setText(QStringLiteral("删除"));
    }
}

void ControlPanel::updateUIState(int index) {
    bool isTree = (index == 3 || index == 4); // BST or AVL
    m_traverseLabel->setVisible(isTree);
    m_traverseCombo->setVisible(isTree);
    m_traverseBtn->setVisible(isTree);

    bool canSearch = (index != 2 && index != 5);
    m_findBtn->setEnabled(canSearch);
    m_findBtn->setText(canSearch ? QStringLiteral("查找") : QStringLiteral("查找 (不支持)"));
}

void ControlPanel::setButtonsEnabled(bool enable) {
    m_insertBtn->setEnabled(enable);
    m_removeBtn->setEnabled(enable);
    m_resetBtn->setEnabled(enable);
    m_structCombo->setEnabled(enable);
    m_traverseBtn->setEnabled(enable);
    m_saveBtn->setEnabled(enable);
    m_loadBtn->setEnabled(enable);
    m_cmdLine->setEnabled(enable);
    m_helpBtn->setEnabled(enable);
    m_aiBtn->setEnabled(enable);

    int currentIdx = m_structCombo->currentIndex();
    if (currentIdx == 2 || currentIdx == 5) {
        m_findBtn->setEnabled(false);
    }
    else {
        m_findBtn->setEnabled(enable);
    }
}

int ControlPanel::getCurrentStructureIndex() const {
    return m_structCombo->currentIndex();
}