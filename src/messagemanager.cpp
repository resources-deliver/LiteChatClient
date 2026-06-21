#include "messagemanager.h"
#include "networkmanager.h"
#include "friendmanager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QIcon>
#include <algorithm>

/**
 * @brief MessageManager构造函数，用于初始化类内私有属性+连接信号槽
 * @param networkManager 网络管理器指针
 * @param friendManager 好友管理器指针
 * @param parent 父对象
 */
MessageManager::MessageManager(NetworkManager* networkManager, FriendManager* friendManager, QObject *parent)
    : QObject(parent)
    , networkManager(networkManager)
    , friendManager(friendManager)
{
    // 数据接收后，手动触发自定义信号，自动调用槽函数
    connect(networkManager, &NetworkManager::DataReceived, this, &MessageManager::OnDataReceived);
}

/**
 * @brief MessageManager析构函数，用于释放动态分配的资源
 */
MessageManager::~MessageManager(){}

/**
 * @brief 发送消息
 * @param receiver 接收者用户名
 * @param content 消息内容
 * @return 是否成功发送请求
 */
bool MessageManager::SendMessage(const QString& receiver, const QString& content){
    if(!ValidateMessageContent(content)){
        return false;
    }
    QJsonObject dataObj;
    dataObj["receiver"] = receiver;
    dataObj["content"] = content;
    QJsonObject requestObj;
    requestObj["type"] = "SEND_MSG";
    requestObj["data"] = dataObj;
    QJsonDocument doc(requestObj);
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);
    bool sendResult = networkManager->SendData(requestData);
    if(!sendResult){
        emit MessageSendFailed("发送失败");
        return false;
    }
    qDebug() << "[MessageManager::SendMessage]发送消息请求成功";
    return true;
}

/**
 * @brief 获取与指定好友的历史消息
 * @param friendUsername 好友用户名
 * @return 是否成功发送请求
 */
bool MessageManager::GetHistoryMessages(const QString& friendUsername){
    QJsonObject dataObj;
    dataObj["friend_username"] = friendUsername;
    QJsonObject requestObj;
    requestObj["type"] = "HISTORY_MSG";
    requestObj["data"] = dataObj;
    QJsonDocument doc(requestObj);
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);
    bool sendResult = networkManager->SendData(requestData);
    if(!sendResult){
        emit HistoryFailed("获取历史消息失败");
        return false;
    }
    qDebug() << "[MessageManager::GetHistoryMessages]获取历史消息请求成功";
    return true;
}

/**
 * @brief 获取缓存中与指定好友的消息列表
 * @param friendUsername 好友用户名
 * @return 消息列表
 */
QList<Message> MessageManager::GetCachedMessages(const QString& friendUsername) const{
    return messageCache.value(friendUsername);
}

/**
 * @brief 将消息添加到缓存中
 * @param msg 消息对象
 */
void MessageManager::AddMessageToCache(const Message& msg){
    QString friendUsername = (msg.sender == currentUsername) ? msg.receiver : msg.sender;
    messageCache[friendUsername].append(msg);
}

/**
 * @brief 设置当前登录用户名
 * @param username 用户名
 */
void MessageManager::SetCurrentUsername(const QString& username){
    currentUsername = username;
}

/**
 * @brief 按时间戳排序消息列表（从早到晚）
 * @param list 待排序的消息列表
 * @return 排序后的消息列表
 */
QList<Message> MessageManager::SortMessagesByTime(const QList<Message>& list){
    QList<Message> sorted = list;
    std::sort(sorted.begin(), sorted.end(), [](const Message& a, const Message& b){
        return a.timestamp < b.timestamp;
    });
    return sorted;
}

/**
 * @brief 接收到数据槽函数
 * @param data 接收到的数据
 */
void MessageManager::OnDataReceived(const QByteArray& data){
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if(parseError.error != QJsonParseError::NoError){
        qDebug() << "[MessageManager::OnDataReceived]JSON解析失败";
        return;
    }
    QJsonObject response = doc.object();
    QString type = response["type"].toString();
    qDebug() << "[MessageManager::OnDataReceived]接收到响应数据，类型:" << type;
    if(type == "SEND_MSG_RESPONSE"){
        HandleSendMessageResponse(response);
    }
    else if(type == "FORWARD_MSG"){
        HandleForwardMessage(response);
    }
    else if(type == "HISTORY_MSG_RESPONSE"){
        HandleHistoryResponse(response);
    }
    else if(type == "MSG_NOTIFY"){
        HandleMessageNotify(response);
    }
}

/**
 * @brief 处理发送消息响应
 * @param response 响应JSON对象
 */
void MessageManager::HandleSendMessageResponse(const QJsonObject& response){
    int code = response["code"].toInt();
    switch(code){
        case 0:{
            QJsonObject data = response["data"].toObject();
            Message msg;
            msg.sender = data["sender"].toString();
            msg.receiver = data["receiver"].toString();
            msg.content = data["content"].toString();
            msg.timestamp = data["timestamp"].toString();
            msg.isRead = true;
            AddMessageToCache(msg);
            emit MessageSendSuccess(msg);
            break;
        }
        case 1002:
            emit MessageSendFailed("服务器接收请求失败");
            break;
        case 4001:
            emit MessageSendFailed("接收方已离线，消息已存储");
            break;
        case 4002:
            emit MessageSendFailed("消息发送失败");
            break;
        case 5001:
            emit MessageSendFailed("服务器罢工了...");
            break;
        default:
            emit MessageSendFailed(response["msg"].toString());
            break;
    }
}

/**
 * @brief 处理服务器转发的消息
 * @param msg 转发消息JSON对象
 */
void MessageManager::HandleForwardMessage(const QJsonObject& msg){
    HandleIncomingMessage(msg);
}

/**
 * @brief 处理历史消息响应
 * @param response 响应JSON对象
 */
void MessageManager::HandleHistoryResponse(const QJsonObject& response){
    int code = response["code"].toInt();
    switch(code){
        case 0:{
            QJsonObject data = response["data"].toObject();
            QString friendUsername = data["friend_username"].toString();
            QJsonArray messages = data["messages"].toArray();
            QList<Message> historyList;
            for(const QJsonValue& val : messages){
                QJsonObject msgObj = val.toObject();
                Message msg;
                msg.sender = msgObj["sender"].toString();
                msg.receiver = msgObj["receiver"].toString();
                msg.content = msgObj["content"].toString();
                msg.timestamp = msgObj["timestamp"].toString();
                msg.isRead = true;
                historyList.append(msg);
            }
            historyList = SortMessagesByTime(historyList);
            messageCache[friendUsername] = historyList;
            emit HistoryReceived(friendUsername, historyList);
            break;
        }
        case 1002:
            emit HistoryFailed("服务器接收请求失败");
            break;
        case 4003:
            emit HistoryReceived(response["data"].toObject()["friend_username"].toString(), QList<Message>());
            break;
        case 5001:
            emit HistoryFailed("服务器罢工了...");
            break;
        default:
            emit HistoryFailed(response["msg"].toString());
            break;
    }
}

/**
 * @brief 处理消息通知
 * @param notify 通知JSON对象
 */
void MessageManager::HandleMessageNotify(const QJsonObject& notify){
    QJsonObject data = notify["data"].toObject();
    QString sender = data["sender"].toString();
    int count = data["count"].toInt();
    qDebug() << "[MessageManager::HandleMessageNotify]收到消息通知，发送者:" << sender << "，未读数量:" << count;
    emit MessageNotify(sender, count);
}

/**
 * @brief 校验消息内容是否合法（非空且长度≤64KB）
 * @param content 消息内容
 * @return 内容是否合法
 */
bool MessageManager::ValidateMessageContent(const QString& content){
    if(content.trimmed().isEmpty()){
        emit MessageSendFailed("消息内容不能为空");
        return false;
    }
    if(content.toUtf8().size() > 64 * 1024){
        emit MessageSendFailed("消息过长");
        return false;
    }
    return true;
}

/**
 * @brief 标记陌生人消息，检查发送者是否在好友列表中
 * @param message 消息对象引用
 */
void MessageManager::MarkAsStrangerMessage(Message& message){
    QList<FriendInfo> friendList = friendManager->GetCachedFriendList();
    bool isFriend = false;
    for(const FriendInfo& info : friendList){
        if(info.username == message.sender){
            isFriend = true;
            break;
        }
    }
    message.isStranger = !isFriend;
}

/**
 * @brief 显示系统托盘通知
 * @param sender 发送者用户名
 * @param summary 消息摘要
 */
void MessageManager::ShowSystemTrayNotification(const QString& sender, const QString& summary){
    QSystemTrayIcon* trayIcon = new QSystemTrayIcon();
    trayIcon->setIcon(QIcon(":/icons/app.png"));
    trayIcon->show();
    QString title = QString("新消息来自 %1").arg(sender);
    QString content = summary.length() > 30 ? summary.left(30) + "..." : summary;
    trayIcon->showMessage(title, content, QSystemTrayIcon::Information, 5000);
    QTimer::singleShot(6000, trayIcon, &QObject::deleteLater);
}

/**
 * @brief 处理接收到的消息（区分普通消息/陌生人消息）
 * @param message 接收到的消息JSON对象
 */
void MessageManager::HandleIncomingMessage(const QJsonObject& message){
    QJsonObject data = message["data"].toObject();
    Message msg;
    msg.sender = data["sender"].toString();
    msg.receiver = data["receiver"].toString();
    msg.content = data["content"].toString();
    msg.timestamp = data["timestamp"].toString();
    msg.isRead = false;
    msg.isStranger = false;
    MarkAsStrangerMessage(msg);
    AddMessageToCache(msg);
    emit MessageReceived(msg);
    qDebug() << "[MessageManager::HandleIncomingMessage]收到来自" << msg.sender << "的消息，陌生人:" << msg.isStranger;
}