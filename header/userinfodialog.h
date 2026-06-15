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
    Ui::UserInfoDialog *ui;
    UserManager* userManager;
    bool isProcessing;
    QTimer* timeoutTimer;
    QString pendingNewUsername;
    QString pendingNewPassword;
};

#endif // USERINFODIALOG_H