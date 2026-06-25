#include "exceptionhandler.h"
#include "networkmanager.h"
#include "clientlogger.h"

#include <QDebug>

/**
 * @brief ExceptionHandler构造函数，用于初始化类内私有属性+连接信号槽
 * @param networkManager 网络管理器指针
 * @param parent 父对象
 */
ExceptionHandler::ExceptionHandler(NetworkManager* networkManager, QObject *parent)
    : QObject(parent)
    , networkManager(networkManager)
    , reconnectCount(0)
    , maxReconnectCount(3)
    , reconnectInterval(5)
    , timeoutTimer(new QTimer(this))
{
    // 网络连接断开后，手动触发自定义信号，自动调用槽函数
    connect(networkManager, &NetworkManager::Disconnected, this, [this](){ HandleException("服务器下机了"); });
    // 时间定时器超时后，自动触发自带的信号，自动调用槽函数
    connect(timeoutTimer, &QTimer::timeout, this, &ExceptionHandler::OnReconnectTimerTimeout);
}

/**
 * @brief ExceptionHandler析构函数，用于释放动态分配的资源
 */
ExceptionHandler::~ExceptionHandler(){
    timeoutTimer->stop();  // 停止时间定时器
}

/**
 * @brief 处理异常
 * @param errorMsg 错误信息
 */
void ExceptionHandler::HandleException(const QString& errorMsg){
    ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "ExceptionHandler", QString("捕获异常: %1").arg(errorMsg));
    if(reconnectCount >= maxReconnectCount){
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "ExceptionHandler", "重连次数已达上限，放弃重连");
        emit ReconnectFailed("重连次数已达上限，服务器下机了");  // 手动触发自定义重连失败信号，通知UI层
        return;
    }
    TryReconnect();
}

/**
 * @brief 尝试重新连接服务器
 * @return 是否启动重连尝试
 */
bool ExceptionHandler::TryReconnect(){
    if(networkManager->IsConnected()){
        ClientLogger::GetInstance().WriteLog(LogLevel::INFO, "ExceptionHandler", "当前已连接，无需重连");
        return false;
    }
    if(timeoutTimer->isActive()){  // 检查时间定时器是否已在运行中
        ClientLogger::GetInstance().WriteLog(LogLevel::WARNING, "ExceptionHandler", "时间定时器已在运行中");
        return false;
    }
    reconnectCount++;
    ClientLogger::GetInstance().WriteLog(
        LogLevel::INFO, "ExceptionHandler", 
        QString("启动第 %1/%2 次重连尝试，间隔 %3 秒").arg(reconnectCount).arg(maxReconnectCount).arg(reconnectInterval)
    );
    emit ReconnectAttempt(reconnectCount, maxReconnectCount);  // 手动触发自定义重连尝试信号，通知UI层
    timeoutTimer->start(reconnectInterval * 1000);  // 启动5秒时间定时器
    return true;
}

/**
 * @brief 重置重连计数器
 */
void ExceptionHandler::ResetReconnectCount(){
    reconnectCount = 0;
    timeoutTimer->stop();  // 停止时间定时器
    ClientLogger::GetInstance().WriteLog(LogLevel::INFO, "ExceptionHandler", "重连计数器已重置");
}

/**
 * @brief 获取当前重连次数
 * @return 当前重连次数
 */
int ExceptionHandler::GetReconnectCount() const{
    return reconnectCount;
}

/**
 * @brief 槽函数，用于响应时间定时器超时后自动触发自带的信号
 */
void ExceptionHandler::OnReconnectTimerTimeout(){
    timeoutTimer->stop();  // 停止时间定时器
    ClientLogger::GetInstance().WriteLog(
        LogLevel::INFO, "ExceptionHandler", QString("执行第 %1 次重连").arg(reconnectCount)
    );
    if(networkManager->IsConnected()){
        ResetReconnectCount();
        emit ReconnectSuccess();  // 手动触发自定义重连成功信号，通知UI层
        return;
    }
    QString ip = networkManager->GetServerIP();
    int port = networkManager->GetServerPort();
    bool result = networkManager->ConnectToServer(ip, port);
    if(!result){
        ClientLogger::GetInstance().WriteLog(
            LogLevel::WARNING, "ExceptionHandler", QString("第 %1 次重连失败").arg(reconnectCount)
        );
        if(reconnectCount >= maxReconnectCount){
            ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "ExceptionHandler", "重连次数已达上限，放弃重连");
            emit ReconnectFailed("重连次数已达上限，服务器下机了");  // 手动触发自定义重连失败信号，通知UI层
            return;
        }
        timeoutTimer->start(reconnectInterval * 1000);  // 启动5秒时间定时器
        return;
    }
    ResetReconnectCount();
    emit ReconnectSuccess();  // 手动触发自定义重连成功信号，通知UI层
}