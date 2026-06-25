#ifndef USERINFODIALOG_H
#define USERINFODIALOG_H

#include <QDialog>
#include <QTimer>

/**
 * @brief 用于用户信息修改对话框（UI界面）的命名空间
 */
namespace Ui{class UserInfoDialog;}

/**
 * @brief 用户管理器的类声明
 */
class UserManager;

/**
 * @brief 用户信息修改对话框，用于修改用户信息
 */
class UserInfoDialog : public QDialog{
    Q_OBJECT

public:
    explicit UserInfoDialog(UserManager* userManager, QWidget *parent = nullptr);
    ~UserInfoDialog() override;

private slots:
    void OnSaveClicked();
    void OnCancelClicked();
    void OnUpdateTimeout();
    void OnUpdateSuccess();
    void OnUpdateFailed(const QString& errorMsg);

private:
    void SetupUI();
    void ShowBusyMessage();
    void ShowVerifyPasswordDialog();

private:
    Ui::UserInfoDialog *ui;  // 用户信息修改对话框（UI界面）的指针
    UserManager* userManager;  // 用户管理器的指针
    bool isProcessing;  // 修改请求状态
    QTimer* timeoutTimer;  // 时间定时器的指针
    QString pendingNewUsername;  // 待更新的用户名
    QString pendingNewPassword;  // 待更新的密码
};

#endif // USERINFODIALOG_H