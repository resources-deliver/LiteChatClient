#ifndef USERINFODIALOG_H
#define USERINFODIALOG_H

#include <QDialog>
#include <QTimer>

namespace Ui{
class UserInfoDialog;
}

class UserManager;

/**
 * @brief 用户信息修改对话框
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
    Ui::UserInfoDialog *ui;  // 用户信息对话对话框（UI界面）的指针
    UserManager* userManager;  // 用户管理器的指针
    bool isProcessing;  // 用户信息更新请求状态
    QTimer* timeoutTimer;  // 时间定时器的指针
    QString pendingNewUsername;  // 待更新的用户名
    QString pendingNewPassword;  // 待更新的密码
};

#endif // USERINFODIALOG_H