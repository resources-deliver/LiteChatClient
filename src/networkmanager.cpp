#include "networkmanager.h"

#include <QDataStream>
#include <QTimer>
#include <QDebug>

/**
 * @brief NetworkManager构造函数
 * @param parent 父对象
 */
NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , socket(new QTcpSocket(this))
    , serverPort(8886)
    , isConnected(false)
    , timeout(5)
{
    connect(socket, &QTcpSocket::connected, this, &NetworkManager::OnConnected);
    connect(socket, &QTcpSocket::disconnected, this, &NetworkManager::OnDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &NetworkManager::OnReadyRead);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &NetworkManager::OnError);
}

/**
 * @brief NetworkManager析构函数
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
        emit ErrorOccurred("已连接到服务器");
        return false;
    }

    serverIP = ip;
    serverPort = port;

    socket->connectToHost(QHostAddress(ip), port);

    if(!socket->waitForConnected(timeout * 1000)){
        emit ErrorOccurred("连接超时");
        return false;
    }

    return true;
}

/**
 * @brief 断开与服务器的连接
 */
void NetworkManager::DisconnectFromServer(){
    if(socket->state() != QAbstractSocket::UnconnectedState){
        socket->disconnectFromHost();
        if(socket->state() != QAbstractSocket::UnconnectedState){
            socket->waitForDisconnected(3000);
        }
    }
    receiveBuffer.clear();
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
        emit ErrorOccurred("未连接到服务器");
        return false;
    }

    QByteArray package = PackageMessage(data);
    qint64 bytesWritten = socket->write(package);
    if(bytesWritten == -1){
        emit ErrorOccurred("发送数据失败");
        return false;
    }

    return socket->waitForBytesWritten(timeout * 1000);
}

/**
 * @brief 设置超时时间
 * @param seconds 超时时间（秒）
 */
void NetworkManager::SetTimeout(int seconds){
    timeout = seconds;
}

/**
 * @brief 连接成功槽函数
 */
void NetworkManager::OnConnected(){
    isConnected = true;
    emit Connected();
}

/**
 * @brief 断开连接槽函数
 */
void NetworkManager::OnDisconnected(){
    isConnected = false;
    receiveBuffer.clear();
    emit Disconnected();
}

/**
 * @brief 有数据可读槽函数
 */
void NetworkManager::OnReadyRead(){
    receiveBuffer.append(socket->readAll());

    while(receiveBuffer.size() >= 4){
        QByteArray header = receiveBuffer.left(4);
        quint32 bodyLength = ParseMessageHeader(header);

        if(bodyLength == 0 || bodyLength > 65536){
            emit ErrorOccurred("协议错误：消息体长度不合法");
            DisconnectFromServer();
            return;
        }

        if(receiveBuffer.size() < 4 + bodyLength){
            break;
        }

        QByteArray body = receiveBuffer.mid(4, bodyLength);
        receiveBuffer.remove(0, 4 + bodyLength);

        emit DataReceived(body);
    }
}

/**
 * @brief 错误发生槽函数
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

    emit ErrorOccurred(errorMsg);
}

/**
 * @brief 解析4字节大端序消息头
 * @param header 4字节消息头
 * @return 消息体长度
 */
quint32 NetworkManager::ParseMessageHeader(const QByteArray& header){
    QDataStream stream(header);
    stream.setByteOrder(QDataStream::BigEndian);
    quint32 length;
    stream >> length;
    return length;
}

/**
 * @brief 封装消息（添加4字节大端序消息头）
 * @param body 消息体
 * @return 封装后的完整消息
 */
QByteArray NetworkManager::PackageMessage(const QByteArray& body){
    QByteArray package;
    QDataStream stream(&package, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << static_cast<quint32>(body.size());
    package.append(body);
    return package;
}
