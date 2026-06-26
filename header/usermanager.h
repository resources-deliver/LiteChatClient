#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>
#include <QString>

/**
 * @brief 网络管理器的类声明
 */
class NetworkManager;

/**
 * @brief 用户状态枚举
 */
enum class UserStatus{Online, Offline};

/**
 * @brief 用户管理器类，用于注册、登录、信息修改和状态管理
 */
class UserManager : public QObject{
    Q_OBJECT

public:
    explicit UserManager(NetworkManager* networkManager, QObject *parent = nullptr);
    ~UserManager() override;
    bool RegisterUser(const QString& username, const QString& password);
    bool LoginUser(const QString& username, const QString& password);
    bool UpdateUserInfo(const QString& newUsername, const QString& newPassword, const QString& verifyPassword);
    UserStatus QueryUserStatus();
    void UpdateUserStatusDisplay(UserStatus status);
    QString EncryptPassword(const QString& password);
    bool ValidateUsername(const QString& username);
    bool ValidatePassword(const QString& password);
    QString GetCurrentUsername() const;
    UserStatus GetCurrentUserStatus() const;
    void SetCurrentUsername(const QString& username);

signals:
    void RegisterSuccess();
    void RegisterFailed(const QString& errorMsg);
    void LoginSuccess();
    void LoginFailed(const QString& errorMsg);
    void UpdateSuccess();
    void UpdateFailed(const QString& errorMsg);
    void StatusChanged(UserStatus status);

private slots:
    void OnDataReceived(const QByteArray& data);

private:
    void HandleRegisterResponse(const QJsonObject& response);
    void HandleLoginResponse(const QJsonObject& response);
    void HandleUpdateResponse(const QJsonObject& response);
    void HandleStatusResponse(const QJsonObject& response);
    void HandleStatusNotify(const QJsonObject& notify);

private:
    NetworkManager* networkManager;  // 网络管理器的指针
    QString currentUsername;  // 当前登录用户的用户名
    UserStatus currentUserStatus;  // 当前登录用户的状态
};

#endif // USERMANAGER_H