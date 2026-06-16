#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QByteArray>
#include <QHostAddress>

/**
 * @brief 网络管理器类，负责与服务器的TCP连接和数据收发
 */
class NetworkManager : public QObject{
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager() override;
    bool ConnectToServer(const QString& ip, int port = 8886);
    void DisconnectFromServer();
    bool IsConnected() const;
    bool SendData(const QByteArray& data);
    void SetTimeout(int seconds);

signals:
    void Connected();
    void Disconnected();
    void DataReceived(const QByteArray& data);
    void ErrorOccurred(const QString& errorMsg);

private slots:
    void OnConnected();
    void OnDisconnected();
    void OnReadyRead();
    void OnError(QAbstractSocket::SocketError socketError);

private:
    quint32 ParseMessageHeader(const QByteArray& header);
    QByteArray PackageMessage(const QByteArray& body);

private:
    QTcpSocket* socket;  // TCP套接字的指针
    QString serverIP;  // 服务器IP地址
    int serverPort;    // 服务器端口号
    bool isConnected;  // 连接状态
    int timeout;       // 连接超时时间（秒）
    QByteArray receiveBuffer;  // 接收缓冲区
};

#endif // NETWORKMANAGER_H