#include "usermanager.h"
#include "networkmanager.h"

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QDebug>

/**
 * @brief UserManager构造函数，用于初始化类内私有属性+连接信号槽
 * @param networkManager 网络管理器指针
 * @param parent 父对象
 */
UserManager::UserManager(NetworkManager* networkManager, QObject *parent)
    : QObject(parent)
    , networkManager(networkManager)
    , currentUserStatus(UserStatus::Offline)
{
    // 数据接收后，手动触发自定义信号，自动调用槽函数
    connect(networkManager, &NetworkManager::DataReceived, this, &UserManager::OnDataReceived);
}

/**
 * @brief UserManager析构函数，用于释放动态分配的资源
 */
UserManager::~UserManager(){}

/**
 * @brief 用户注册
 * @param username 用户名
 * @param password 密码（明文）
 * @return 是否成功发送注册请求
 */
bool UserManager::RegisterUser(const QString& username, const QString& password){
    bool result = ValidateUsername(username);  // 验证用户名格式
    if(!result){  // 如果用户名不符合格式
        qDebug() << "[UserManager::RegisterUser]用户名不符合格式";  // Debug输出
        emit RegisterFailed("用户名不符合格式");  // 手动触发自定义注册失败信号
        return false;
    }
    result = ValidatePassword(password);  // 验证密码长度
    if(!result){  // 如果密码长度不符合要求
        qDebug() << "[UserManager::RegisterUser]密码长度至少为6位";  // Debug输出
        emit RegisterFailed("密码长度至少为6位");  // 手动触发自定义注册失败信号
        return false;
    }
    QString encryptedPassword = EncryptPassword(password);  // 加密密码
    QJsonObject dataObj;  // 注册请求数据对象
    dataObj["username"] = username;  // 设置用户名
    dataObj["password"] = encryptedPassword;  // 设置加密后的密码
    QJsonObject requestObj;  // 请求对象
    requestObj["type"] = "REGISTER";  // 设置请求类型为注册
    requestObj["data"] = dataObj;  // 设置请求数据
    QJsonDocument doc(requestObj);  // 将请求对象转换为JSON文档
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);  // 将JSON文档转换为字节数组
    bool sendResult = networkManager->SendData(requestData);  // 发送请求数据
    if(!sendResult){  // 如果发送失败
        qDebug() << "[UserManager::RegisterUser]发送请求失败";  // Debug输出
        emit RegisterFailed("发送请求失败");  // 手动触发自定义注册失败信号
        return false;
    }
    qDebug() << "[UserManager::RegisterUser]发送请求成功";  // Debug输出
    // emit RegisterSuccess();  // 手动触发自定义注册成功信号
    return true;
}

/**
 * @brief 用户登录
 * @param username 用户名
 * @param password 密码（明文）
 * @return 是否成功发送登录请求
 */
bool UserManager::LoginUser(const QString& username, const QString& password){
    bool result = ValidateUsername(username);  // 验证用户名格式
    if(!result){  // 如果用户名不符合格式
        qDebug() << "[UserManager::LoginUser]用户名不符合格式";  // Debug输出
        emit LoginFailed("用户名不符合格式");  // 手动触发自定义登录失败信号
        return false;
    }
    result = ValidatePassword(password);  // 验证密码长度
    if(!result){  // 如果密码长度不符合要求
        qDebug() << "[UserManager::LoginUser]密码长度至少为6位";  // Debug输出
        emit LoginFailed("密码长度至少为6位");  // 手动触发自定义登录失败信号
        return false;
    }
    QString encryptedPassword = EncryptPassword(password);  // 加密密码
    QJsonObject dataObj;  // 登录请求数据对象
    dataObj["username"] = username;  // 设置用户名
    dataObj["password"] = encryptedPassword;  // 设置加密后的密码
    QJsonObject requestObj;  // 请求对象
    requestObj["type"] = "LOGIN";  // 设置请求类型为登录
    requestObj["data"] = dataObj;  // 设置请求数据
    QJsonDocument doc(requestObj);  // 将请求对象转换为JSON文档
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);  // 将JSON文档转换为字节数组
    bool sendResult = networkManager->SendData(requestData);  // 发送请求数据
    if(!sendResult){  // 如果发送失败
        qDebug() << "[UserManager::LoginUser]发送请求失败";  // Debug输出
        emit LoginFailed("发送请求失败");  // 手动触发自定义登录失败信号
        return false;
    }
    qDebug() << "[UserManager::LoginUser]发送请求成功";  // Debug输出
    // emit LoginSuccess();  // 手动触发自定义登录成功信号
    return true;
}

/**
 * @brief 修改用户信息
 * @param newUsername 新用户名（可选，为空表示不修改）
 * @param newPassword 新密码（可选，为空表示不修改）
 * @param verifyPassword 当前密码验证（必填）
 * @return 是否成功发送修改请求
 */
bool UserManager::UpdateUserInfo(const QString& newUsername, const QString& newPassword, const QString& verifyPassword){
    if(verifyPassword.isEmpty()){  // 如果当前密码验证为空
        qDebug() << "[UserManager::UpdateUserInfo]密码不能为空";  // Debug输出
        emit UpdateFailed("密码不能为空");  // 手动触发自定义更新失败信号
        return false;
    }
    if(newUsername.isEmpty() && newPassword.isEmpty()){  // 如果新用户名和新密码都为空
        qDebug() << "[UserManager::UpdateUserInfo]未修改任何信息";  // Debug输出
        emit UpdateFailed("未修改任何信息");  // 手动触发自定义更新失败信号
        return false;
    }
    if(!newUsername.isEmpty() && !ValidateUsername(newUsername)){  // 如果新用户名不为空且不符合格式
        qDebug() << "[UserManager::UpdateUserInfo]用户名不符合格式";  // Debug输出
        emit UpdateFailed("用户名不符合格式");  // 手动触发自定义更新失败信号
        return false;
    }
    if(!newPassword.isEmpty() && !ValidatePassword(newPassword)){  // 如果新密码不为空且不符合格式
        qDebug() << "[UserManager::UpdateUserInfo]密码长度至少为6位";  // Debug输出
        emit UpdateFailed("密码长度至少为6位");  // 手动触发自定义更新失败信号
        return false;
    }
    QJsonObject dataObj;  // 更新用户数据对象
    dataObj["verify_password"] = EncryptPassword(verifyPassword);  // 加密当前密码验证
    if(!newUsername.isEmpty()){  // 如果新用户名不为空
        dataObj["new_username"] = newUsername;  // 设置新用户名
    }
    if(!newPassword.isEmpty()){  // 如果新密码不为空
        dataObj["new_password"] = EncryptPassword(newPassword);  // 加密新密码
    }
    QJsonObject requestObj;  // 更新用户请求对象
    requestObj["type"] = "UPDATE_USER";  // 设置请求类型为更新用户信息
    requestObj["data"] = dataObj;  // 设置请求数据
    QJsonDocument doc(requestObj);  // 将请求对象转换为JSON文档
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);  // 将JSON文档转换为字节数组
    bool sendResult = networkManager->SendData(requestData);  // 发送请求数据
    if(!sendResult){  // 如果发送失败
        qDebug() << "[UserManager::UpdateUserInfo]发送请求失败";  // Debug输出
        emit UpdateFailed("发送请求失败");  // 手动触发自定义更新失败信号
        return false;
    }
    qDebug() << "[UserManager::UpdateUserInfo]发送请求成功";  // Debug输出
    // emit UpdateSuccess();  // 手动触发自定义更新成功信号
    return true;
}

/**
 * @brief 查询用户状态
 * @return 当前用户状态
 */
UserStatus UserManager::QueryUserStatus(){
    QJsonObject dataObj;  // 查询用户状态数据对象
    dataObj["username"] = currentUsername;  // 设置用户名
    QJsonObject requestObj;  // 查询用户状态请求对象
    requestObj["type"] = "QUERY_STATUS";  // 设置请求类型为查询用户状态
    requestObj["data"] = dataObj;  // 设置请求数据
    QJsonDocument doc(requestObj);  // 将请求对象转换为JSON文档
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);  // 将JSON文档转换为字节数组
    bool sendResult = networkManager->SendData(requestData);  // 发送请求数据
    if(!sendResult){  // 如果发送失败
        qDebug() << "[UserManager::QueryUserStatus]发送请求失败";  // Debug输出
        return UserStatus();
    }
    qDebug() << "[UserManager::QueryUserStatus]发送请求成功";  // Debug输出
    return currentUserStatus;
}

/**
 * @brief 更新用户状态显示
 * @param status 用户状态
 */
void UserManager::UpdateUserStatusDisplay(UserStatus status){
    currentUserStatus = status;  // 更新当前用户状态
    emit StatusChanged(status);  // 手动触发自定义状态改变信号
}

/**
 * @brief 使用MD5加密密码
 * @param password 明文密码
 * @return MD5加密后的密码字符串
 */
QString UserManager::EncryptPassword(const QString& password){
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5);
    return hash.toHex();
}

/**
 * @brief 校验用户名格式
 * @param username 用户名
 * @return 是否合法
 */
bool UserManager::ValidateUsername(const QString& username){
    QRegularExpression regex("^[a-zA-Z0-9_]{3,20}$");
    return regex.match(username).hasMatch();
}

/**
 * @brief 校验密码格式
 * @param password 密码
 * @return 是否合法
 */
bool UserManager::ValidatePassword(const QString& password){
    return password.length() >= 6;
}

/**
 * @brief 获取当前登录用户名
 * @return 当前用户名
 */
QString UserManager::GetCurrentUsername() const{
    return currentUsername;
}

/**
 * @brief 获取当前用户状态
 * @return 当前用户状态
 */
UserStatus UserManager::GetCurrentUserStatus() const{
    return currentUserStatus;
}

/**
 * @brief 设置当前登录用户名
 * @param username 用户名
 */
void UserManager::SetCurrentUsername(const QString& username){
    currentUsername = username;
}

/**
 * @brief 接收到数据槽函数
 * @param data 接收到的数据
 */
void UserManager::OnDataReceived(const QByteArray& data){
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if(parseError.error != QJsonParseError::NoError){
        return;
    }
    QJsonObject response = doc.object();
    QString type = response["type"].toString();
    if(type == "REGISTER"){
        HandleRegisterResponse(response);
    }
    else if(type == "LOGIN"){
        HandleLoginResponse(response);
    }
    else if(type == "UPDATE_USER"){
        HandleUpdateResponse(response);
    }
    else if(type == "QUERY_STATUS"){
        HandleStatusResponse(response);
    }
}

/**
 * @brief 处理注册响应
 * @param response 响应JSON对象
 */
void UserManager::HandleRegisterResponse(const QJsonObject& response){
    int code = response["code"].toInt();
    switch(code){
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
void UserManager::HandleLoginResponse(const QJsonObject& response){
    int code = response["code"].toInt();
    switch(code){
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
void UserManager::HandleUpdateResponse(const QJsonObject& response){
    int code = response["code"].toInt();
    switch(code){
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
void UserManager::HandleStatusResponse(const QJsonObject& response){
    int code = response["code"].toInt();
    if(code == 0){
        QString status = response["data"].toObject()["status"].toString();
        if(status == "online"){
            currentUserStatus = UserStatus::Online;
        }
        else{
            currentUserStatus = UserStatus::Offline;
        }
        emit StatusChanged(currentUserStatus);
    }
    else if(code == 5001){
        emit UpdateFailed("服务器罢工了...");
    }
}
