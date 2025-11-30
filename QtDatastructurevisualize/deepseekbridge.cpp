#include "deepseekbridge.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>

DeepSeekBridge::DeepSeekBridge(QObject* parent) : QObject(parent) {
    m_manager = new QNetworkAccessManager(this);
}

void DeepSeekBridge::query(const QString& userPrompt) {
    // === 修复点 1：拆分 URL 和 Request 的定义，避免编译器误判 ===
    QUrl url(API_URL);
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    // 这里的 API_KEY 是你在头文件里定义的常量
    request.setRawHeader("Authorization", QString("Bearer %1").arg(API_KEY).toUtf8());

    // 构建 JSON Payload
    QJsonObject payload;
    // DeepSeek 的模型名称，如果是 v3 可能是 deepseek-chat，具体看官方文档
    payload["model"] = "deepseek-chat";
    payload["temperature"] = 0.1; // 低温度，让 AI 回答更严谨

    QJsonArray messages;

    // System Prompt
    QJsonObject sysMsg;
    sysMsg["role"] = "system";
    sysMsg["content"] = SYSTEM_PROMPT;
    messages.append(sysMsg);

    // User Prompt
    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = userPrompt;
    messages.append(userMsg);

    payload["messages"] = messages;
    // 可选：不使用流式输出，方便一次性处理
    payload["stream"] = false;

    // === 修复点 2：现在 request 是个正常对象了，post 就能匹配到了 ===
    QNetworkReply* reply = m_manager->post(request, QJsonDocument(payload).toJson());

    // 异步处理响应
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // 记得设置自动删除，防止内存泄漏
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred("网络请求失败: " + reply->errorString());
            return;
        }

        QByteArray responseData = reply->readAll();
        // 调试打印，方便看 AI 到底回了什么 (发布时可以注释掉)
        qDebug() << "AI Response Raw:" << responseData;

        QJsonDocument doc = QJsonDocument::fromJson(responseData);

        if (doc.isNull()) {
            emit errorOccurred("API 返回数据不是有效的 JSON");
            return;
        }

        // 解析 OpenAI 兼容格式响应
        // 结构通常是 { "choices": [ { "message": { "content": "..." } } ] }
        QJsonObject root = doc.object();

        // 简单的错误检查：如果 API 返回了 error 字段
        if (root.contains("error")) {
            QString apiError = root["error"].toObject()["message"].toString();
            emit errorOccurred("API 报错: " + apiError);
            return;
        }

        if (root.contains("choices")) {
            QJsonArray choices = root["choices"].toArray();
            if (!choices.isEmpty()) {
                QString content = choices[0].toObject()["message"].toObject()["content"].toString();
                // 去掉可能存在的 markdown 代码块标记 (```json ... ```)
                content = content.remove("```json").remove("```").trimmed();

                emit responseReceived(content);
                return;
            }
        }

        emit errorOccurred("无法解析 AI 响应内容");
        });
}