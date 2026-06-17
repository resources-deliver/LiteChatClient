#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>
#include <QTimer>

namespace Ui{
class ConnectDialog;
}

class NetworkManager;

/**
 * @brief 连接服务器对话框
 */
class ConnectDialog : public QDialog{
    Q_OBJECT

public:
    explicit ConnectDialog(NetworkManager* networkManager, QWidget *parent = nullptr);
    ~ConnectDialog() override;

private slots:
    void OnConnectClicked();
    void OnSkipClicked();
    void OnConnectionTimeout();
    void OnConnected();
    void OnError(const QString& errorMsg);

private:
    void SetupUI();
    bool ValidateIP(const QString& ip);
    void ShowBusyMessage();
    void PerformConnection(const QString& ip);

private:
    Ui::ConnectDialog *ui;  // 连接对话框（UI界面）的指针
    NetworkManager* networkManager;  // 网络管理器的指针
    bool isProcessing;  // 连接请求状态
    static constexpr const char* DEFAULT_IP = "192.168.162.128";  // 静态默认（服务器端）的IP地址
    QTimer* timeoutTimer;  // 时间定时器的指针
};

#endif // CONNECTDIALOG_H