#include "usermanager.h"
#include "networkmanager.h"
#include "clientlogger.h"

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
    bool result = ValidateUsername(username);
    if(!result){
        emit RegisterFailed("用户名不符合格式");  // 手动触发自定义注册失败信号，通知UI层
        return false;
    }
    result = ValidatePassword(password);
    if(!result){
        emit RegisterFailed("密码长度至少为6位");
        return false;
    }
    QString encryptedPassword = EncryptPassword(password);
    QJsonObject dataObj;  // 注册请求数据对象
    dataObj["username"] = username;  // 注册请求数据（用户名）
    dataObj["password"] = encryptedPassword;  // 注册请求数据（加密后的密码）
    QJsonObject requestObj;  // 注册请求对象
    requestObj["type"] = "REGISTER";  // 注册请求类型
    requestObj["data"] = dataObj;  // 注册请求数据
    QJsonDocument doc(requestObj);  // 将请求对象转换为JSON文档
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);  // 将JSON文档转换为字节数组
    bool sendResult = networkManager->SendData(requestData);
    if(!sendResult){
        emit RegisterFailed("发送数据失败");
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
bool UserManager::LoginUser(const QString& username, const QString& password){
    bool result = ValidateUsername(username);
    if(!result){
        emit LoginFailed("用户名不符合格式");  // 手动触发自定义登录失败信号，通知UI层
        return false;
    }
    result = ValidatePassword(password);
    if(!result){
        emit LoginFailed("密码长度至少为6位");
        return false;
    }
    QString encryptedPassword = EncryptPassword(password);
    QJsonObject dataObj;  // 登录请求数据对象
    dataObj["username"] = username;  // 登录请求数据（用户名）
    dataObj["password"] = encryptedPassword;  // 登录请求数据（加密后的密码）
    QJsonObject requestObj;  // 登录请求对象
    requestObj["type"] = "LOGIN";  // 登录请求类型
    requestObj["data"] = dataObj;  // 登录请求数据
    QJsonDocument doc(requestObj);  // 将请求对象转换为JSON文档
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);  // 将JSON文档转换为字节数组
    currentUsername = username;
    bool sendResult = networkManager->SendData(requestData);
    if(!sendResult){
        emit LoginFailed("发送数据失败");
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
bool UserManager::UpdateUserInfo(const QString& newUsername, const QString& newPassword, const QString& verifyPassword){
    if(verifyPassword.isEmpty()){  // 如果当前密码验证为空
        emit UpdateFailed("密码不能为空");  // 手动触发自定义更新失败信号，通知UI层
        return false;
    }
    if(newUsername.isEmpty() && newPassword.isEmpty()){  // 如果新用户名和新密码都为空
        emit UpdateFailed("未修改任何信息");
        return false;
    }
    if(!newUsername.isEmpty() && !ValidateUsername(newUsername)){  // 如果新用户名不为空且不符合格式
        emit UpdateFailed("用户名不符合格式");
        return false;
    }
    if(!newPassword.isEmpty() && !ValidatePassword(newPassword)){  // 如果新密码不为空且不符合格式
        emit UpdateFailed("密码长度至少为6位");
        return false;
    }
    QJsonObject dataObj;  // 更新信息请求数据对象
    dataObj["verify_password"] = EncryptPassword(verifyPassword);  // 更新信息请求数据（当前密码验证）
    if(!newUsername.isEmpty()){  // 如果新用户名不为空
        dataObj["new_username"] = newUsername;  // 更新信息请求数据（新用户名）
    }
    if(!newPassword.isEmpty()){  // 如果新密码不为空
        dataObj["new_password"] = EncryptPassword(newPassword);  // 更新信息请求数据（加密后的新密码）
    }
    QJsonObject requestObj;  // 更新信息请求对象
    requestObj["type"] = "UPDATE_USER";  // 更新信息请求类型
    requestObj["data"] = dataObj;  // 更新信息请求数据
    QJsonDocument doc(requestObj);  // 将更新信息请求对象转换为JSON文档
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);  // 将JSON文档转换为字节数组
    bool sendResult = networkManager->SendData(requestData);
    if(!sendResult){
        emit UpdateFailed("发送数据失败");
        return false;
    }
    return true;
}

/**
 * @brief 查询用户状态
 * @return 当前用户状态
 */
UserStatus UserManager::QueryUserStatus(){
    QJsonObject dataObj;  // 查询用户状态请求数据对象
    dataObj["username"] = currentUsername;  // 查询用户状态请求数据（用户名）
    QJsonObject requestObj;  // 查询用户状态请求对象
    requestObj["type"] = "QUERY_STATUS";  // 查询用户状态请求类型
    requestObj["data"] = dataObj;  // 查询用户状态请求数据
    QJsonDocument doc(requestObj);  // 将查询用户状态请求对象转换为JSON文档
    QByteArray requestData = doc.toJson(QJsonDocument::Compact);  // 将JSON文档转换为字节数组
    bool sendResult = networkManager->SendData(requestData);
    if(!sendResult){
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "UserManager", "发送数据失败");        
        return UserStatus();
    }
    ClientLogger::GetInstance().WriteLog(LogLevel::INFO, "UserManager", "发送数据成功");
    return currentUserStatus;
}

/**
 * @brief 更新用户状态显示
 * @param status 用户状态
 */
void UserManager::UpdateUserStatusDisplay(UserStatus status){
    currentUserStatus = status;
    emit StatusChanged(status);  // 手动触发自定义状态改变信号，通知UI层
}

/**
 * @brief 使用MD5加密密码
 * @param password 明文密码
 * @return MD5加密后的密码字符串
 */
QString UserManager::EncryptPassword(const QString& password){
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5);  // 使用MD5加密密码
    QString hex = hash.toHex();  // 将加密后的密码转换为十六进制字符串
    return hex;
}

/**
 * @brief 校验用户名格式
 * @param username 用户名
 * @return 是否合法
 */
bool UserManager::ValidateUsername(const QString& username){
    QRegularExpression regex("^[a-zA-Z0-9_]{3,20}$");  // 编译正则表达式
    bool matchResult = regex.match(username).hasMatch();  // 匹配用户名
    return matchResult;
}

/**
 * @brief 校验密码格式
 * @param password 密码
 * @return 是否合法
 */
bool UserManager::ValidatePassword(const QString& password){
    bool matchResult = password.length() >= 6;  // 密码长度至少为6位
    return matchResult;
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
 * @brief 槽函数，用于响应接收数据后手动触发的自定义信号
 * @param data 接收到的数据
 */
void UserManager::OnDataReceived(const QByteArray& data){
    QJsonParseError parseError;  // JSON解析错误对象
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);  // 将字节数组转换为JSON文档
    if(parseError.error != QJsonParseError::NoError){  // 如果解析失败
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "UserManager", "JSON解析失败:" + parseError.errorString());
        return;
    }
    QJsonObject response = doc.object();  // 从JSON文档中提取响应对象
    QString type = response["type"].toString();  // 从响应对象中提取类型字段
    ClientLogger::GetInstance().WriteLog(LogLevel::INFO, "UserManager", "接收到响应数据，类型:" + type);
    if(type == "REGISTER_RESPONSE"){
        HandleRegisterResponse(response);
    }
    else if(type == "LOGIN_RESPONSE"){
        HandleLoginResponse(response);
    }
    else if(type == "UPDATE_USER_RESPONSE"){
        HandleUpdateResponse(response);
    }
    else if(type == "QUERY_STATUS_RESPONSE"){
        HandleStatusResponse(response);
    }
    else if(type == "STATUS_NOTIFY"){
        HandleStatusNotify(response);
    }
    else if(type == "HEARTBEAT"){
        // 心跳由服务器端管理，客户端仅接收并忽略
    }
    else if(type == "ADD_FRIEND_RESPONSE" || type == "DEL_FRIEND_RESPONSE" ||
            type == "FRIEND_LIST_RESPONSE" || type == "QUERY_FRIEND_RESPONSE"){
        // 好友相关响应由FriendManager处理，此处忽略
    }
    else{
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "UserManager", "未知的响应类型:" + type);
    }
}

/**
 * @brief 处理注册响应
 * @param response 响应JSON对象
 */
void UserManager::HandleRegisterResponse(const QJsonObject& response){
    int code = response["code"].toInt();  // 从响应对象中提取状态码字段
    switch(code){
        case 0:
            emit RegisterSuccess();  // 手动触发自定义注册成功信号，通知UI层
            break;
        case 1002:
            emit RegisterFailed("服务器接收请求失败");  // 手动触发自定义注册失败信号，通知UI层
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
    int code = response["code"].toInt();  // 从响应对象中提取状态码字段
    switch(code){
        case 0:
            emit LoginSuccess();  // 手动触发自定义登录成功信号，通知UI层
            break;
        case 1002:
            emit LoginFailed("服务器接收请求失败");  // 手动触发自定义登录失败信号，通知UI层
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
    int code = response["code"].toInt();  // 从响应对象中提取状态码字段
    switch(code){
        case 0:
            emit UpdateSuccess();  // 手动触发自定义修改成功信号，通知UI层
            break;
        case 1002:
            emit UpdateFailed("服务器接收请求失败");  // 手动触发自定义修改失败信号，通知UI层
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
    int code = response["code"].toInt();  // 从响应对象中提取状态码字段
    if(code == 0){
        QString status = response["data"].toObject()["status"].toString();  // 从响应对象中提取状态字段
        if(status == "online"){
            currentUserStatus = UserStatus::Online;
        }
        else{
            currentUserStatus = UserStatus::Offline;
        }
        emit StatusChanged(currentUserStatus);  // 手动触发自定义状态变更信号，通知UI层
    }
    else if(code == 5001){
        emit UpdateFailed("服务器罢工了...");  // 手动触发自定义修改失败信号，通知UI层
    }
}

/**
 * @brief 处理状态变更通知
 * @param notify 通知JSON对象
 */
void UserManager::HandleStatusNotify(const QJsonObject& notify){
    QJsonObject data = notify["data"].toObject();  // 从通知对象中提取数据字段
    QString username = data["username"].toString();  // 从数据对象中提取用户名字段
    QString status = data["status"].toString();  // 从数据对象中提取状态字段
    qDebug() << "[UserManager::HandleStatusNotify]收到状态变更通知，用户名:" << username << "，状态:" << status;
    if(username == currentUsername){
        if(status == "online"){
            currentUserStatus = UserStatus::Online;
        }
        else{
            currentUserStatus = UserStatus::Offline;
        }
        emit StatusChanged(currentUserStatus);  // 手动触发自定义状态变更信号，通知UI层
    }
}