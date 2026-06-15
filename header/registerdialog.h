#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QTimer>

namespace Ui{
class RegisterDialog;
}

class UserManager;

/**
 * @brief 注册对话框
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

private:
    Ui::RegisterDialog *ui;
    UserManager* userManager;
    bool isProcessing;
    QTimer* timeoutTimer;
    QString registeredUsername;
};

#endif // REGISTERDIALOG_H