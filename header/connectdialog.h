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
    Ui::ConnectDialog *ui;
    NetworkManager* networkManager;
    bool isProcessing;
    static constexpr const char* DEFAULT_IP = "192.168.162.128";
    QTimer* timeoutTimer;
};

#endif // CONNECTDIALOG_H