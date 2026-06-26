#ifndef EXCEPTIONHANDLER_H
#define EXCEPTIONHANDLER_H

#include <QObject>
#include <QTimer>

/**
 * @brief 网络管理器的类声明
 */
class NetworkManager;

/**
 * @brief 异常处理器类，负责捕获网络异常并自动重连
 */
class ExceptionHandler : public QObject{
    Q_OBJECT

public:
    explicit ExceptionHandler(NetworkManager* networkManager, QObject *parent = nullptr);
    ~ExceptionHandler() override;
    void HandleException(const QString& errorMsg);
    bool TryReconnect();
    void ResetReconnectCount();
    int GetReconnectCount() const;

signals:
    void ReconnectSuccess();
    void ReconnectFailed(const QString& errorMsg);
    void ReconnectAttempt(int currentAttempt, int maxAttempts);

private slots:
    void OnReconnectTimerTimeout();

private:
    NetworkManager* networkManager;  // 网络管理器指针
    int reconnectCount;  // 当前重连次数
    int maxReconnectCount;  // 最大重连次数
    int reconnectInterval;  // 重连间隔时间
    QTimer* timeoutTimer;  // 时间定时器的指针
};

#endif // EXCEPTIONHANDLER_H