#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QTimer>

/**
 * @brief 用于注册对话框（UI界面）的命名空间    
 */
namespace Ui{class RegisterDialog;}

/**
 * @brief 用户管理器的类声明
 */
class UserManager;

/**
 * @brief 注册对话框，用于用户注册
 */
class RegisterDialog : public QDialog{
    Q_OBJECT

public:
    explicit RegisterDialog(UserManager* userManager, QWidget *parent = nullptr);
    ~RegisterDialog() override;
    QString GetRegisteredUsername() const;

private slots:
    void OnRegisterClicked();
    void OnBackClicked();
    void OnRegisterTimeout();
    void OnRegisterSuccess();
    void OnRegisterFailed(const QString& errorMsg);

private:
    void SetupUI();
    void ShowBusyMessage();
    void keyPressEvent(QKeyEvent* event) override;

private:
    Ui::RegisterDialog *ui;  // 注册对话框（UI界面）的指针
    UserManager* userManager;  // 用户管理器的指针
    bool isProcessing;  // 注册请求状态
    QTimer* timeoutTimer;  // 时间定时器的指针
    QString registeredUsername;  // 注册成功的用户名
};

#endif // REGISTERDIALOG_H