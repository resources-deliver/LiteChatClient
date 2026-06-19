#include "friendmanager.h"
#include "networkmanager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <algorithm>

/**
 * @brief FriendManager构造函数，用于初始化类内私有属性+连接信号槽
 * @param networkManager 网络管理器指针
 * @param parent 父对象
 */
FriendManager::FriendManager(NetworkManager* networkManager, QObject *parent)
    : QObject(parent)
    , networkManager(networkManager)
{
    connect(networkManager, &NetworkManager::DataReceived, this, &FriendManager::OnDataReceived);
}

/**
 * @brief FriendManager析构函数，用于释放动态分配的资源
 */
FriendManager::~FriendManager(){}

/**
 * @brief 添加好友
 * @param username 要添加的好友用户名
 * @return 是否成功发送添加请求
 */
bool FriendManager::AddFriend(const QString& username){
    QJsonObject dataObj;
    dataObj["username"] = username;
    QJsonObject requestObj;
    requestObj["type"] = "ADD_FRIEND";
    requestObj["data"] = dataObj;
    QJsonDocument doc(requestObj);
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);
    bool sendResult = networkManager->SendData(requestData);
    if(!sendResult){
        qDebug() << "[FriendManager::AddFriend]发送请求失败";
        emit FriendAddFailed("发送请求失败");
        return false;
    }
    qDebug() << "[FriendManager::AddFriend]发送请求成功";
    return true;
}

/**
 * @brief 删除好友
 * @param username 要删除的好友用户名
 * @return 是否成功发送删除请求
 */
bool FriendManager::DeleteFriend(const QString& username){
    QJsonObject dataObj;
    dataObj["username"] = username;
    QJsonObject requestObj;
    requestObj["type"] = "DEL_FRIEND";
    requestObj["data"] = dataObj;
    QJsonDocument doc(requestObj);
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);
    bool sendResult = networkManager->SendData(requestData);
    if(!sendResult){
        qDebug() << "[FriendManager::DeleteFriend]发送请求失败";
        emit FriendDeleteFailed("发送请求失败");
        return false;
    }
    qDebug() << "[FriendManager::DeleteFriend]发送请求成功";
    return true;
}

/**
 * @brief 获取好友列表
 * @return 是否成功发送获取请求
 */
bool FriendManager::GetFriendList(){
    QJsonObject dataObj;
    QJsonObject requestObj;
    requestObj["type"] = "FRIEND_LIST";
    requestObj["data"] = dataObj;
    QJsonDocument doc(requestObj);
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);
    bool sendResult = networkManager->SendData(requestData);
    if(!sendResult){
        qDebug() << "[FriendManager::GetFriendList]发送请求失败";
        emit FriendListFailed("发送请求失败");
        return false;
    }
    qDebug() << "[FriendManager::GetFriendList]发送请求成功";
    return true;
}

/**
 * @brief 查询好友信息
 * @param username 要查询的用户名
 * @return 是否成功发送查询请求
 */
bool FriendManager::QueryFriendInfo(const QString& username){
    QJsonObject dataObj;
    dataObj["username"] = username;
    QJsonObject requestObj;
    requestObj["type"] = "QUERY_FRIEND";
    requestObj["data"] = dataObj;
    QJsonDocument doc(requestObj);
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);
    bool sendResult = networkManager->SendData(requestData);
    if(!sendResult){
        qDebug() << "[FriendManager::QueryFriendInfo]发送请求失败";
        emit QueryFriendFailed("发送请求失败");
        return false;
    }
    qDebug() << "[FriendManager::QueryFriendInfo]发送请求成功";
    return true;
}

/**
 * @brief 按在线状态排序好友列表（在线优先）
 * @param list 待排序的好友列表
 * @return 排序后的好友列表
 */
QList<FriendInfo> FriendManager::SortFriendList(const QList<FriendInfo>& list){
    QList<FriendInfo> sorted = list;
    std::sort(sorted.begin(), sorted.end(), [](const FriendInfo& a, const FriendInfo& b){
        if(a.status != b.status){
            return a.status == UserStatus::Online;
        }
        return a.username < b.username;
    });
    return sorted;
}

/**
 * @brief 更新好友状态
 * @param username 好友用户名
 * @param status 新状态
 */
void FriendManager::UpdateFriendStatus(const QString& username, UserStatus status){
    for(int i = 0; i < friendList.size(); ++i){
        if(friendList[i].username == username){
            friendList[i].status = status;
            emit FriendStatusChanged(username, status);
            return;
        }
    }
}

/**
 * @brief 获取缓存的好友列表
 * @return 好友列表
 */
QList<FriendInfo> FriendManager::GetCachedFriendList() const{
    return friendList;
}

/**
 * @brief 接收到数据槽函数
 * @param data 接收到的数据
 */
void FriendManager::OnDataReceived(const QByteArray& data){
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if(parseError.error != QJsonParseError::NoError){
        return;
    }
    QJsonObject response = doc.object();
    QString type = response["type"].toString();
    if(type == "ADD_FRIEND_RESPONSE"){
        HandleAddFriendResponse(response);
    }
    else if(type == "DEL_FRIEND_RESPONSE"){
        HandleDeleteFriendResponse(response);
    }
    else if(type == "FRIEND_LIST_RESPONSE"){
        HandleFriendListResponse(response);
    }
    else if(type == "QUERY_FRIEND_RESPONSE"){
        HandleQueryFriendResponse(response);
    }
    else if(type == "STATUS_NOTIFY"){
        HandleStatusNotify(response);
    }
}

/**
 * @brief 处理添加好友响应
 * @param response 响应JSON对象
 */
void FriendManager::HandleAddFriendResponse(const QJsonObject& response){
    int code = response["code"].toInt();
    switch(code){
        case 0:
            emit FriendAdded(response["data"].toObject()["username"].toString());
            break;
        case 1002:
            emit FriendAddFailed("服务器接收请求失败");
            break;
        case 3001:
            emit FriendAddFailed("该用户不存在");
            break;
        case 3002:
            emit FriendAddFailed("对方已是您的好友");
            break;
        case 3003:
            emit FriendAddFailed("怎么能添加自己呢");
            break;
        case 5001:
            emit FriendAddFailed("服务器罢工了...");
            break;
        default:
            emit FriendAddFailed(response["msg"].toString());
            break;
    }
}

/**
 * @brief 处理删除好友响应
 * @param response 响应JSON对象
 */
void FriendManager::HandleDeleteFriendResponse(const QJsonObject& response){
    int code = response["code"].toInt();
    switch(code){
        case 0:
            emit FriendDeleted(response["data"].toObject()["username"].toString());
            break;
        case 1002:
            emit FriendDeleteFailed("服务器接收请求失败");
            break;
        case 3004:
            emit FriendDeleteFailed("对方不是您的好友");
            break;
        case 5001:
            emit FriendDeleteFailed("服务器罢工了...");
            break;
        default:
            emit FriendDeleteFailed(response["msg"].toString());
            break;
    }
}

/**
 * @brief 处理好友列表响应
 * @param response 响应JSON对象
 */
void FriendManager::HandleFriendListResponse(const QJsonObject& response){
    int code = response["code"].toInt();
    switch(code){
        case 0:{
            friendList.clear();
            QJsonArray friends = response["data"].toObject()["friends"].toArray();
            for(const QJsonValue& val : friends){
                QJsonObject friendObj = val.toObject();
                FriendInfo info;
                info.username = friendObj["username"].toString();
                QString status = friendObj["status"].toString();
                info.status = (status == "online") ? UserStatus::Online : UserStatus::Offline;
                friendList.append(info);
            }
            emit FriendListReceived(SortFriendList(friendList));
            break;
        }
        case 1002:
            emit FriendListFailed("服务器接收请求失败");
            break;
        case 5001:
            emit FriendListFailed("服务器罢工了...");
            break;
        default:
            emit FriendListFailed(response["msg"].toString());
            break;
    }
}

/**
 * @brief 处理查询好友响应
 * @param response 响应JSON对象
 */
void FriendManager::HandleQueryFriendResponse(const QJsonObject& response){
    int code = response["code"].toInt();
    switch(code){
        case 0:{
            QJsonObject data = response["data"].toObject();
            FriendInfo info;
            info.username = data["username"].toString();
            QString status = data["status"].toString();
            info.status = (status == "online") ? UserStatus::Online : UserStatus::Offline;
            emit QueryFriendResult(info);
            break;
        }
        case 1002:
            emit QueryFriendFailed("服务器接收请求失败");
            break;
        case 3001:
            emit QueryFriendFailed("该用户不存在");
            break;
        case 5001:
            emit QueryFriendFailed("服务器罢工了...");
            break;
        default:
            emit QueryFriendFailed(response["msg"].toString());
            break;
    }
}

/**
 * @brief 处理状态变更通知（好友上下线）
 * @param notify 通知JSON对象
 */
void FriendManager::HandleStatusNotify(const QJsonObject& notify){
    QJsonObject data = notify["data"].toObject();
    QString username = data["username"].toString();
    QString status = data["status"].toString();
    UserStatus userStatus = (status == "online") ? UserStatus::Online : UserStatus::Offline;
    UpdateFriendStatus(username, userStatus);
}