#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QTimer>

namespace Ui{
class LoginDialog;
}

class UserManager;

/**
 * @brief 登录对话框
 */
class LoginDialog : public QDialog{
    Q_OBJECT

public:
    explicit LoginDialog(UserManager* userManager, QWidget *parent = nullptr);
    ~LoginDialog() override;
    void SetUsername(const QString& username);

private slots:
    void OnLoginClicked();
    void OnRegisterClicked();
    void OnLoginTimeout();
    void OnLoginSuccess();
    void OnLoginFailed(const QString& errorMsg);

private:
    void SetupUI();
    void ShowBusyMessage();

private:
    Ui::LoginDialog *ui;
    UserManager* userManager;
    bool isProcessing;
    QTimer* timeoutTimer;
};

#endif // LOGINDIALOG_H