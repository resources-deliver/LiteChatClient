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
    // 数据接收后，手动触发自定义信号，自动调用槽函数
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
    QJsonObject dataObj;  // 添加请求数据对象
    dataObj["username"] = username;  // 添加请求数据（用户名）
    QJsonObject requestObj;  // 添加请求对象
    requestObj["type"] = "ADD_FRIEND";  // 添加请求类型
    requestObj["data"] = dataObj;  // 添加请求数据
    QJsonDocument doc(requestObj);  // 将请求对象转换为JSON文档
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);  // 将JSON文档转换为字节数组
    bool sendResult = networkManager->SendData(requestData);
    if(!sendResult){
        emit FriendAddFailed("发送数据失败");  // 手动触发自定义添加失败信号，通知UI层
        return false;
    }
    return true;
}

/**
 * @brief 删除好友
 * @param username 要删除的好友用户名
 * @return 是否成功发送删除请求
 */
bool FriendManager::DeleteFriend(const QString& username){
    QJsonObject dataObj;  // 删除请求数据对象
    dataObj["username"] = username;  // 删除请求数据（用户名）
    QJsonObject requestObj;  // 删除请求对象
    requestObj["type"] = "DEL_FRIEND";  // 删除请求类型
    requestObj["data"] = dataObj;  // 删除请求数据
    QJsonDocument doc(requestObj);  // 将请求对象转换为JSON文档
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);  // 将JSON文档转换为字节数组
    bool sendResult = networkManager->SendData(requestData);
    if(!sendResult){
        emit FriendDeleteFailed("发送数据失败");  // 手动触发自定义删除失败信号，通知UI层
        return false;
    }
    return true;
}

/**
 * @brief 获取好友列表
 * @return 是否成功发送获取请求
 */
bool FriendManager::GetFriendList(){
    QJsonObject dataObj;  // 获取请求数据对象
    QJsonObject requestObj;  // 获取请求对象
    requestObj["type"] = "FRIEND_LIST";  // 获取请求类型
    requestObj["data"] = dataObj;  // 获取请求数据
    QJsonDocument doc(requestObj);  // 将请求对象转换为JSON文档
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);  // 将JSON文档转换为字节数组
    bool sendResult = networkManager->SendData(requestData);
    if(!sendResult){
        emit FriendListFailed("发送数据失败");  // 手动触发自定义获取失败信号，通知UI层
        return false;
    }
    return true;
}

/**
 * @brief 查询好友信息
 * @param username 要查询的用户名
 * @return 是否成功发送查询请求
 */
bool FriendManager::QueryFriendInfo(const QString& username){
    QJsonObject dataObj;  // 查询请求数据对象
    dataObj["username"] = username;  // 查询请求数据（用户名）
    QJsonObject requestObj;  // 查询请求对象
    requestObj["type"] = "QUERY_FRIEND";  // 查询请求类型
    requestObj["data"] = dataObj;  // 查询请求数据
    QJsonDocument doc(requestObj);  // 将请求对象转换为JSON文档
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);  // 将JSON文档转换为字节数组
    bool sendResult = networkManager->SendData(requestData);
    if(!sendResult){
        emit QueryFriendFailed("发送数据失败");  // 手动触发自定义查询失败信号，通知UI层
        return false;
    }
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
            emit FriendStatusChanged(username, status);  // 手动触发自定义好友状态改变信号，通知UI层
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
 * @brief 槽函数，用于响应接收数据后手动触发的自定义信号
 * @param data 接收到的数据
 */
void FriendManager::OnDataReceived(const QByteArray& data){
    QJsonParseError parseError;  // JSON解析错误对象
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);  // 将字节数组转换为JSON文档
    if(parseError.error != QJsonParseError::NoError){  // 如果解析失败
        qDebug() << "[FriendManager::OnDataReceived]JSON解析失败";
        return;
    }
    QJsonObject response = doc.object();  // 从JSON文档中提取响应对象
    QString type = response["type"].toString();  // 从响应对象中提取类型字段
    qDebug() << "[FriendManager::OnDataReceived]接收到响应数据，类型:" << type;
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
    int code = response["code"].toInt();  // 从响应对象中提取状态码字段
    switch(code){
        case 0:
            emit FriendAdded(response["data"].toObject()["username"].toString());  // 手动触发自定义好友添加成功信号，通知UI层
            break;
        case 1002:
            emit FriendAddFailed("服务器接收请求失败");  // 手动触发自定义好友添加失败信号，通知UI层
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
    int code = response["code"].toInt();  // 从响应对象中提取状态码字段
    switch(code){
        case 0:
            emit FriendDeleted(response["data"].toObject()["username"].toString());  // 手动触发自定义好友删除成功信号，通知UI层
            break;
        case 1002:
            emit FriendDeleteFailed("服务器接收请求失败");  // 手动触发自定义好友删除失败信号，通知UI层
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
    int code = response["code"].toInt();  // 从响应对象中提取状态码字段
    switch(code){
        case 0:{
            friendList.clear();  // 清空缓存的好友列表
            QJsonArray friends = response["data"].toObject()["friends"].toArray();  // 从响应对象中提取好友列表数组
            for(const QJsonValue& val : friends){
                QJsonObject friendObj = val.toObject();  // 从JSON值中提取好友对象
                FriendInfo info;
                info.username = friendObj["username"].toString();  // 从好友对象中提取用户名字段
                QString status = friendObj["status"].toString();  // 从好友对象中提取状态字段
                info.status = (status == "online") ? UserStatus::Online : UserStatus::Offline;
                friendList.append(info);  // 将好友信息添加到好友列表中
            }
            emit FriendListReceived(SortFriendList(friendList));  // 手动触发自定义好友列表接收成功信号，通知UI层
            break;
        }
        case 1002:
            emit FriendListFailed("服务器接收请求失败");  // 手动触发自定义好友列表接收失败信号，通知UI层
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
    int code = response["code"].toInt();  // 从响应对象中提取状态码字段
    switch(code){
        case 0:{
            QJsonObject data = response["data"].toObject();  // 从响应对象中提取数据对象
            FriendInfo info;
            info.username = data["username"].toString();  // 从数据对象中提取用户名字段
            QString status = data["status"].toString();  // 从数据对象中提取状态字段
            info.status = (status == "online") ? UserStatus::Online : UserStatus::Offline;
            emit QueryFriendResult(info);  // 手动触发自定义查询好友结果信号，通知UI层
            break;
        }
        case 1002:
            emit QueryFriendFailed("服务器接收请求失败");  // 手动触发自定义查询好友失败信号，通知UI层
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
    QJsonObject data = notify["data"].toObject();  // 从通知对象中提取数据对象
    QString username = data["username"].toString();  // 从数据对象中提取用户名字段
    QString status = data["status"].toString();  // 从数据对象中提取状态字段
    UserStatus userStatus = (status == "online") ? UserStatus::Online : UserStatus::Offline;
    UpdateFriendStatus(username, userStatus);
}