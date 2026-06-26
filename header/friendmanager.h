#ifndef FRIENDMANAGER_H
#define FRIENDMANAGER_H

#include <QObject>
#include <QList>
#include <QString>

#include "usermanager.h"

/**
 * @brief 网络管理器的类声明
 */
class NetworkManager;

/**
 * @brief 用于存储好友信息的结构体
 */
struct FriendInfo{
    QString username;  // 好友用户名
    UserStatus status;  // 好友在线状态
};

/**
 * @brief 好友管理器类，用于好友的添加、删除、查询和列表管理
 */
class FriendManager : public QObject{
    Q_OBJECT

public:
    explicit FriendManager(NetworkManager* networkManager, QObject *parent = nullptr);
    ~FriendManager() override;
    bool AddFriend(const QString& username);
    bool DeleteFriend(const QString& username);
    bool GetFriendList();
    bool QueryFriendInfo(const QString& username);
    QList<FriendInfo> SortFriendList(const QList<FriendInfo>& list);
    void UpdateFriendStatus(const QString& username, UserStatus status);
    QList<FriendInfo> GetCachedFriendList() const;

signals:
    void FriendAdded(const QString& username);
    void FriendAddFailed(const QString& errorMsg);
    void FriendDeleted(const QString& username);
    void FriendDeleteFailed(const QString& errorMsg);
    void FriendListReceived(const QList<FriendInfo>& friendList);
    void FriendListFailed(const QString& errorMsg);
    void QueryFriendResult(const FriendInfo& info);
    void QueryFriendFailed(const QString& errorMsg);
    void FriendStatusChanged(const QString& username, UserStatus status);

private slots:
    void OnDataReceived(const QByteArray& data);

private:
    void HandleAddFriendResponse(const QJsonObject& response);
    void HandleDeleteFriendResponse(const QJsonObject& response);
    void HandleFriendListResponse(const QJsonObject& response);
    void HandleQueryFriendResponse(const QJsonObject& response);
    void HandleStatusNotify(const QJsonObject& notify);

private:
    NetworkManager* networkManager;  // 网络管理器的指针
    QList<FriendInfo> friendList;  // 好友列表缓存
    bool isFetchingFriendList;  // 是否正在获取好友列表
};

#endif // FRIENDMANAGER_H