#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QMap>

class NetworkManager;
class FriendManager;

/**
 * @brief 消息方向枚举
 */
enum class MessageDirection {
    Sent,     // 发送的消息
    Received  // 接收的消息
};

/**
 * @brief 消息数据结构体
 */
struct Message{
    QString sender;    // 发送者用户名
    QString receiver;  // 接收者用户名
    QString content;   // 消息内容
    QString timestamp; // 消息时间戳
    bool isRead;       // 消息是否已读
    bool isStranger;   // 是否为陌生人消息（发送者不在好友列表中）
};

/**
 * @brief 消息管理器类，负责消息的发送、接收、历史记录管理
 */
class MessageManager : public QObject{
    Q_OBJECT

public:
    explicit MessageManager(NetworkManager* networkManager, FriendManager* friendManager, QObject *parent = nullptr);
    ~MessageManager() override;
    bool SendMessage(const QString& receiver, const QString& content);
    bool GetHistoryMessages(const QString& friendUsername);
    QList<Message> GetCachedMessages(const QString& friendUsername) const;
    void AddMessageToCache(const Message& msg);
    void SetCurrentUsername(const QString& username);
    QList<Message> SortMessagesByTime(const QList<Message>& list);
    bool ValidateMessageContent(const QString& content);
    void MarkAsStrangerMessage(Message& message);
    void ShowSystemTrayNotification(const QString& sender, const QString& summary);
    void HandleIncomingMessage(const QJsonObject& message);

signals:
    void MessageReceived(const Message& msg);
    void HistoryReceived(const QString& friendUsername, const QList<Message>& messages);
    void MessageSendSuccess(const Message& msg);
    void MessageSendFailed(const QString& errorMsg);
    void HistoryFailed(const QString& errorMsg);
    void MessageNotify(const QString& sender, int count);

private slots:
    void OnDataReceived(const QByteArray& data);

private:
    void HandleSendMessageResponse(const QJsonObject& response);
    void HandleForwardMessage(const QJsonObject& msg);
    void HandleHistoryResponse(const QJsonObject& response);
    void HandleMessageNotify(const QJsonObject& notify);

private:
    NetworkManager* networkManager;  // 网络管理器的指针
    FriendManager* friendManager;    // 好友管理器的指针
    QString currentUsername;         // 当前登录用户的用户名
    QMap<QString, QList<Message>> messageCache;  // 消息缓存，键为好友用户名，值为消息列表
};

#endif // MESSAGEMANAGER_H