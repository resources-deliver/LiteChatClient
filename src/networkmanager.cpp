#include "networkmanager.h"

#include <QDataStream>
#include <QTimer>
#include <QDebug>

/**
 * @brief NetworkManager构造函数，用于初始化类内私有属性+连接信号槽
 * @param parent 父对象
 */
NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , socket(new QTcpSocket(this))
    , serverPort(8886)
    , isConnected(false)
    , timeout(5)
{
    // 连接成功后，自动触发自带的信号，自动调用槽函数
    connect(socket, &QTcpSocket::connected, this, &NetworkManager::OnConnected);
    // 连接断开后，自动触发自带的信号，自动调用槽函数
    connect(socket, &QTcpSocket::disconnected, this, &NetworkManager::OnDisconnected);
    // 数据可读时，自动触发自带的信号，自动调用槽函数
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::OnReadyRead);
    // 错误发生时，自动触发自带的信号，自动调用槽函数
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this, &NetworkManager::OnError);
}

/**
 * @brief NetworkManager析构函数，用于释放动态分配的资源
 */
NetworkManager::~NetworkManager(){
    DisconnectFromServer();
}

/**
 * @brief 连接到服务器
 * @param ip 服务器IP地址
 * @param port 服务器端口，默认8886
 * @return 连接请求是否成功发起
 */
bool NetworkManager::ConnectToServer(const QString& ip, int port){
    if(isConnected){
        emit ErrorOccurred("已连接到服务器");  // 手动触发自定义连接错误信号，通知UI层
        return false;
    }
    serverIP = ip;
    serverPort = port;
    socket->connectToHost(QHostAddress(ip), port);  // 连接到服务器
    bool result = socket->waitForConnected(timeout * 1000);  // 等待连接成功
    if(!result){
        emit ErrorOccurred("连接超时");
        return false;
    }
    qDebug() << "[NetworkManager::ConnectToServer]连接成功IP地址:" << ip;
    return true;
}

/**
 * @brief 断开与服务器的连接
 */
void NetworkManager::DisconnectFromServer(){
    if(socket->state() != QAbstractSocket::UnconnectedState){  // 如果状态不为未连接（不为已连接、连接中）
        socket->disconnectFromHost();  // 断开连接
        if(socket->state() != QAbstractSocket::UnconnectedState){  // 再次审查连接状态
            socket->waitForDisconnected(3000);  // 等待断开连接
        }
    }
    receiveBuffer.clear();  // 清空接收缓冲区
    qDebug() << "[NetworkManager::DisconnectFromServer]已断开与服务器的连接";
}

/**
 * @brief 获取当前连接状态
 * @return 是否已连接
 */
bool NetworkManager::IsConnected() const{
    return isConnected;
}

/**
 * @brief 发送数据到服务器
 * @param data 要发送的数据（消息体）
 * @return 是否发送成功
 */
bool NetworkManager::SendData(const QByteArray& data){
    if(!isConnected){
        emit ErrorOccurred("未连接到服务器,无法发送数据");  // 手动触发自定义连接错误信号，通知UI层
        return false;
    }
    QByteArray package = PackageMessage(data);
    qint64 bytesWritten = socket->write(package);  // 发送数据
    if(bytesWritten == -1){
        emit ErrorOccurred("发送数据失败");
        return false;
    }
    bool result = socket->waitForBytesWritten(timeout * 1000);  // 等待数据发送完成
    if(!result){
        emit ErrorOccurred("发送超时");
        return false;
    }
    qDebug() << "[NetworkManager::SendData]发送成功";
    return true;
}

/**
 * @brief 设置超时时间
 * @param seconds 超时时间（秒）
 */
void NetworkManager::SetTimeout(int seconds){
    timeout = seconds;
}

/**
 * @brief 获取服务器IP地址
 * @return 服务器IP地址
 */
QString NetworkManager::GetServerIP() const{
    return serverIP;
}

/**
 * @brief 获取服务器端口号
 * @return 服务器端口号
 */
int NetworkManager::GetServerPort() const{
    return serverPort;
}

/**
 * @brief 槽函数，用于响应连接成功后自动触发自带的信号
 */
void NetworkManager::OnConnected(){
    isConnected = true;
    emit Connected();  // 手动触发自定义连接成功信号，通知UI层
}

/**
 * @brief 槽函数，用于响应连接断开后自动触发自带的信号
 */
void NetworkManager::OnDisconnected(){
    isConnected = false;
    receiveBuffer.clear();  // 清空接收缓冲区
    emit Disconnected();  // 手动触发自定义断开连接信号，通知UI层
}

/**
 * @brief 槽函数，用于响应有数据可读时自动触发自带的信号
 */
void NetworkManager::OnReadyRead(){
    receiveBuffer.append(socket->readAll());  // 读取所有可读数据
    while(receiveBuffer.size() >= 4){  // 若接收缓冲区数据量≥4字节
        QByteArray header = receiveBuffer.left(4);  // 提取消息头
        quint32 bodyLength = ParseMessageHeader(header);
        if(bodyLength == 0 || bodyLength > 65536){  // 若消息体长度不合法
            emit ErrorOccurred("协议错误：消息体长度不合法");  // 手动触发自定义错误信号，通知UI层
            DisconnectFromServer();
            return;
        }
        if(receiveBuffer.size() < 4 + bodyLength){  // 若接收缓冲区数据量不足
            qDebug() << "[NetworkManager::OnReadyRead]接收缓冲区数据量不足，等待更多数据";
            break;
        }
        QByteArray body = receiveBuffer.mid(4, bodyLength);  // 提取消息体
        receiveBuffer.remove(0, 4 + bodyLength);  // 移除已处理数据
        qDebug() << "[NetworkManager::OnReadyRead]收到数据:" << body.toHex();  // Debug输出
        emit DataReceived(body);  // 手动触发自定义数据接收信号，通知UI层
    }
}

/**
 * @brief 槽函数，用于响应错误发生时自动触发自带的信号
 * @param socketError Socket错误类型
 */
void NetworkManager::OnError(QAbstractSocket::SocketError socketError){
    QString errorMsg;
    switch(socketError){
        case QAbstractSocket::ConnectionRefusedError:
            errorMsg = "连接被拒绝";
            break;
        case QAbstractSocket::RemoteHostClosedError:
            errorMsg = "服务器下机了";
            break;
        case QAbstractSocket::HostNotFoundError:
            errorMsg = "无法找到服务器";
            break;
        case QAbstractSocket::SocketTimeoutError:
            errorMsg = "连接超时";
            break;
        case QAbstractSocket::NetworkError:
            errorMsg = "网络异常";
            break;
        default:
            errorMsg = socket->errorString();
            break;
    }
    emit ErrorOccurred(errorMsg);  // 手动触发自定义连接错误信号，通知UI层
}

/**
 * @brief 解析4字节大端序消息头
 * @param header 4字节消息头
 * @return 消息体长度
 */
quint32 NetworkManager::ParseMessageHeader(const QByteArray& header){
    QDataStream stream(header);  // 创建数据流
    stream.setByteOrder(QDataStream::BigEndian);  // 设置字节序为大端序
    quint32 length;  // 消息体长度
    stream >> length;  // 读取消息体长度
    return length;
}

/**
 * @brief 封装消息（添加4字节大端序消息头）
 * @param body 消息体
 * @return 封装后的完整消息
 */
QByteArray NetworkManager::PackageMessage(const QByteArray& body){
    QByteArray package;  // 封装后的完整消息
    QDataStream stream(&package, QIODevice::WriteOnly);  // 创建数据流
    stream.setByteOrder(QDataStream::BigEndian);  // 设置字节序为大端序
    stream << static_cast<quint32>(body.size());  // 写入消息体长度
    package.append(body);  // 写入消息体
    return package;
}