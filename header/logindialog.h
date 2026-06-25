#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QTimer>

/**
 * @brief 用于登录对话框（UI界面）的命名空间
 */
namespace Ui{class LoginDialog;}

/**
 * @brief 用户管理器的类声明
 */
class UserManager;

/**
 * @brief 登录对话框，用于用户登录
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
    void keyPressEvent(QKeyEvent* event) override;

private:
    Ui::LoginDialog *ui;  // 登录对话框（UI界面）的指针
    UserManager* userManager;  // 用户管理器的指针
    bool isProcessing;  // 登录请求状态
    QTimer* timeoutTimer;  // 时间定时器的指针
};

#endif // LOGINDIALOG_H