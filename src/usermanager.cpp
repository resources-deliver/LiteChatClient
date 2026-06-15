#include "usermanager.h"
#include "networkmanager.h"

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

/**
  * @brief UserManager构造函数
  * @param networkManager 网络管理器指针
  * @param parent 父对象
  */
UserManager::UserManager(NetworkManager* networkManager, QObject *parent)
    : QObject(parent)
    , networkManager(networkManager)
    , currentUserStatus(UserStatus::Offline)
{
    connect(networkManager, &NetworkManager::DataReceived, this, &UserManager::OnDataReceived);
}

/**
  * @brief UserManager析构函数
  */
UserManager::~UserManager()
{
}

/**
  * @brief 用户注册
  * @param username 用户名
  * @param password 密码（明文）
  * @return 是否成功发送注册请求
  */
bool UserManager::RegisterUser(const QString& username, const QString& password)
{
    if (!ValidateUsername(username))
    {
        emit RegisterFailed("用户名不符合格式");
        return false;
    }

    if (!ValidatePassword(password))
    {
        emit RegisterFailed("密码长度至少为6位");
        return false;
    }

    QString encryptedPassword = EncryptPassword(password);

    QJsonObject dataObj;
    dataObj["username"] = username;
    dataObj["password"] = encryptedPassword;

    QJsonObject requestObj;
    requestObj["type"] = "REGISTER";
    requestObj["data"] = dataObj;

    QJsonDocument doc(requestObj);
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);

    if (!networkManager->SendData(requestData))
    {
        emit RegisterFailed("发送请求失败");
        return false;
    }

    return true;
}

/**
  * @brief 用户登录
  * @param username 用户名
  * @param password 密码（明文）
  * @return 是否成功发送登录请求
  */
bool UserManager::LoginUser(const QString& username, const QString& password)
{
    if (!ValidateUsername(username))
    {
        emit LoginFailed("用户名不符合格式");
        return false;
    }

    if (!ValidatePassword(password))
    {
        emit LoginFailed("密码长度至少为6位");
        return false;
    }

    QString encryptedPassword = EncryptPassword(password);

    QJsonObject dataObj;
    dataObj["username"] = username;
    dataObj["password"] = encryptedPassword;

    QJsonObject requestObj;
    requestObj["type"] = "LOGIN";
    requestObj["data"] = dataObj;

    QJsonDocument doc(requestObj);
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);

    if (!networkManager->SendData(requestData))
    {
        emit LoginFailed("发送请求失败");
        return false;
    }

    return true;
}

/**
  * @brief 修改用户信息
  * @param newUsername 新用户名（可选，为空表示不修改）
  * @param newPassword 新密码（可选，为空表示不修改）
  * @param verifyPassword 当前密码验证（必填）
  * @return 是否成功发送修改请求
  */
bool UserManager::UpdateUserInfo(const QString& newUsername, const QString& newPassword, const QString& verifyPassword)
{
    if (verifyPassword.isEmpty())
    {
        emit UpdateFailed("密码不能为空");
        return false;
    }

    if (newUsername.isEmpty() && newPassword.isEmpty())
    {
        emit UpdateFailed("未修改任何信息");
        return false;
    }

    if (!newUsername.isEmpty() && !ValidateUsername(newUsername))
    {
        emit UpdateFailed("用户名不符合格式");
        return false;
    }

    if (!newPassword.isEmpty() && !ValidatePassword(newPassword))
    {
        emit UpdateFailed("密码长度至少为6位");
        return false;
    }

    QJsonObject dataObj;
    dataObj["verify_password"] = EncryptPassword(verifyPassword);

    if (!newUsername.isEmpty())
    {
        dataObj["new_username"] = newUsername;
    }

    if (!newPassword.isEmpty())
    {
        dataObj["new_password"] = EncryptPassword(newPassword);
    }

    QJsonObject requestObj;
    requestObj["type"] = "UPDATE_USER";
    requestObj["data"] = dataObj;

    QJsonDocument doc(requestObj);
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);

    if (!networkManager->SendData(requestData))
    {
        emit UpdateFailed("发送请求失败");
        return false;
    }

    return true;
}

/**
  * @brief 查询用户状态
  * @return 当前用户状态
  */
UserStatus UserManager::QueryUserStatus()
{
    QJsonObject dataObj;
    dataObj["username"] = currentUsername;

    QJsonObject requestObj;
    requestObj["type"] = "QUERY_STATUS";
    requestObj["data"] = dataObj;

    QJsonDocument doc(requestObj);
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);

    networkManager->SendData(requestData);

    return currentUserStatus;
}

/**
  * @brief 更新用户状态显示
  * @param status 用户状态
  */
void UserManager::UpdateUserStatusDisplay(UserStatus status)
{
    currentUserStatus = status;
    emit StatusChanged(status);
}

/**
  * @brief 使用MD5加密密码
  * @param password 明文密码
  * @return MD5加密后的密码字符串
  */
QString UserManager::EncryptPassword(const QString& password)
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5);
    return hash.toHex();
}

/**
  * @brief 校验用户名格式
  * @param username 用户名
  * @return 是否合法
  */
bool UserManager::ValidateUsername(const QString& username)
{
    QRegularExpression regex("^[a-zA-Z0-9_]{3,20}$");
    return regex.match(username).hasMatch();
}

/**
  * @brief 校验密码格式
  * @param password 密码
  * @return 是否合法
  */
bool UserManager::ValidatePassword(const QString& password)
{
    return password.length() >= 6;
}

/**
  * @brief 获取当前登录用户名
  * @return 当前用户名
  */
QString UserManager::GetCurrentUsername() const
{
    return currentUsername;
}

/**
  * @brief 获取当前用户状态
  * @return 当前用户状态
  */
UserStatus UserManager::GetCurrentUserStatus() const
{
    return currentUserStatus;
}

/**
  * @brief 设置当前登录用户名
  * @param username 用户名
  */
void UserManager::SetCurrentUsername(const QString& username)
{
    currentUsername = username;
}

/**
  * @brief 接收到数据槽函数
  * @param data 接收到的数据
  */
void UserManager::OnDataReceived(const QByteArray& data)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        return;
    }

    QJsonObject response = doc.object();
    QString type = response["type"].toString();

    if (type == "REGISTER")
    {
        HandleRegisterResponse(response);
    }
    else if (type == "LOGIN")
    {
        HandleLoginResponse(response);
    }
    else if (type == "UPDATE_USER")
    {
        HandleUpdateResponse(response);
    }
    else if (type == "QUERY_STATUS")
    {
        HandleStatusResponse(response);
    }
}

/**
  * @brief 处理注册响应
  * @param response 响应JSON对象
  */
void UserManager::HandleRegisterResponse(const QJsonObject& response)
{
    int code = response["code"].toInt();

    switch (code)
    {
    case 0:
        emit RegisterSuccess();
        break;
    case 1002:
        emit RegisterFailed("服务器接收请求失败");
        break;
    case 2001:
        emit RegisterFailed("用户名不符合格式");
        break;
    case 2002:
        emit RegisterFailed("用户名已存在");
        break;
    case 5001:
        emit RegisterFailed("服务器罢工了...");
        break;
    default:
        emit RegisterFailed(response["msg"].toString());
        break;
    }
}

/**
  * @brief 处理登录响应
  * @param response 响应JSON对象
  */
void UserManager::HandleLoginResponse(const QJsonObject& response)
{
    int code = response["code"].toInt();

    switch (code)
    {
    case 0:
        currentUsername = response["data"].toObject()["username"].toString();
        emit LoginSuccess();
        break;
    case 1002:
        emit LoginFailed("服务器接收请求失败");
        break;
    case 2003:
        emit LoginFailed("该用户不存在");
        break;
    case 2004:
        emit LoginFailed("用户名或密码错误");
        break;
    case 2005:
        emit LoginFailed("该账号已在其他地方登录");
        break;
    case 5001:
        emit LoginFailed("服务器罢工了...");
        break;
    default:
        emit LoginFailed(response["msg"].toString());
        break;
    }
}

/**
  * @brief 处理修改用户信息响应
  * @param response 响应JSON对象
  */
void UserManager::HandleUpdateResponse(const QJsonObject& response)
{
    int code = response["code"].toInt();

    switch (code)
    {
    case 0:
        emit UpdateSuccess();
        break;
    case 1002:
        emit UpdateFailed("服务器接收请求失败");
        break;
    case 2006:
        emit UpdateFailed("验证密码错误");
        break;
    case 2002:
        emit UpdateFailed("用户名已存在");
        break;
    case 5001:
        emit UpdateFailed("服务器罢工了...");
        break;
    default:
        emit UpdateFailed(response["msg"].toString());
        break;
    }
}

/**
  * @brief 处理状态查询响应
  * @param response 响应JSON对象
  */
void UserManager::HandleStatusResponse(const QJsonObject& response)
{
    int code = response["code"].toInt();

    if (code == 0)
    {
        QString status = response["data"].toObject()["status"].toString();
        if (status == "online")
        {
            currentUserStatus = UserStatus::Online;
        }
        else
        {
            currentUserStatus = UserStatus::Offline;
        }
        emit StatusChanged(currentUserStatus);
    }
    else if (code == 5001)
    {
        emit UpdateFailed("服务器罢工了...");
    }
}