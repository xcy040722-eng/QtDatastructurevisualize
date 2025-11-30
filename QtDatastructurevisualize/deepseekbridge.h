#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

class DeepSeekBridge : public QObject {
    Q_OBJECT
public:
    explicit DeepSeekBridge(QObject* parent = nullptr);

    // 发送请求给 AI
    void query(const QString& userPrompt);

signals:
    // AI 回复了 DSL 指令（如 "new bst [1,2,3]"）
    void responseReceived(const QString& dslCmd);
    // 发生错误
    void errorOccurred(const QString& errorMsg);

private:
    QNetworkAccessManager* m_manager;

    // === 请在这里填入您的 API KEY ===
    const QString API_KEY = "sk-577f9b6bcc8a45de9c6b0d44138237da";

    // DeepSeek API 地址 (兼容 OpenAI 格式)
    const QString API_URL = "https://api.deepseek.com/chat/completions";

    // 系统提示词：教 AI 做人
    const QString SYSTEM_PROMPT =
        "你是一个数据结构可视化助手的后端逻辑。你的任务是将用户的自然语言转换为特定的 DSL 指令。\n"
        "DSL 语法定义如下：\n"
        "1. 创建/重置结构:\n"
        "   - new bst [1,2,3]\n"
        "   - new array [1,2,3]\n"
        "   - new list [1,2,3]\n"
        "   - new stack [1,2]\n"
        "   - new huffman [5,10,3]\n"
        "   - new avl [1,2,3]\n"
        "2. 操作:\n"
        "   - insert 100 (栈也用这个或 push)\n"
        "   - delete 50 (栈也用这个或 pop)\n"
        "   - find 30\n"
        "3. 遍历:\n"
        "   - traverse pre (前序)\n"
        "   - traverse in (中序)\n"
        "   - traverse post (后序)\n"
        "\n"
        "【重要规则】\n"
        "- 你必须严格且只输出 DSL 指令字符串。\n"
        "-不可以输出空序列，如new bst [],new stack [],禁止！"
        "- 不要输出任何 Markdown 格式（如 ```json），不要输出解释性文字。\n"
        "- 如果用户输入无法理解或无关，输出 'ERROR'。";
};