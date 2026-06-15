#include "userinfodialog.h"
#include "ui_userinfodialog.h"
#include "usermanager.h"

#include <QMessageBox>
#include <QInputDialog>

/**
  * @brief UserInfoDialog构造函数
  * @param userManager 用户管理器指针
  * @param parent 父窗口
  */
UserInfoDialog::UserInfoDialog(UserManager* userManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UserInfoDialog)
    , userManager(userManager)
    , isProcessing(false)
    , timeoutTimer(new QTimer(this))
{
    ui->setupUi(this);
    SetupUI();

    connect(ui->saveButton, &QPushButton::clicked, this, &UserInfoDialog::OnSaveClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &UserInfoDialog::OnCancelClicked);
    connect(timeoutTimer, &QTimer::timeout, this, &UserInfoDialog::OnUpdateTimeout);
    connect(userManager, &UserManager::UpdateSuccess, this, &UserInfoDialog::OnUpdateSuccess);
    connect(userManager, &UserManager::UpdateFailed, this, &UserInfoDialog::OnUpdateFailed);

    ui->currentUsernameLabel->setText("当前用户名: " + userManager->GetCurrentUsername());
}

/**
  * @brief UserInfoDialog析构函数
  */
UserInfoDialog::~UserInfoDialog()
{
    delete ui;
}

/**
  * @brief 设置UI界面样式
  */
void UserInfoDialog::SetupUI()
{
    setWindowTitle("个人信息设置");
    setFixedSize(400, 450);
    setMinimumSize(400, 450);

    ui->titleLabel->setText("个人信息设置");
    ui->titleLabel->setAlignment(Qt::AlignCenter);
    ui->titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333;");

    ui->currentUsernameLabel->setStyleSheet("font-size: 12px; color: #666666;");

    ui->newUsernameLabel->setText("新用户名（可选）");
    ui->newUsernameLabel->setStyleSheet("font-size: 12px; color: #666666;");

    ui->newUsernameLineEdit->setPlaceholderText("请输入新用户名（3-20位）");
    ui->newUsernameLineEdit->setFixedHeight(36);
    ui->newUsernameLineEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "}"
    );

    ui->newPasswordLabel->setText("新密码（可选）");
    ui->newPasswordLabel->setStyleSheet("font-size: 12px; color: #666666;");

    ui->newPasswordLineEdit->setPlaceholderText("请输入新密码（至少6位）");
    ui->newPasswordLineEdit->setEchoMode(QLineEdit::Password);
    ui->newPasswordLineEdit->setFixedHeight(36);
    ui->newPasswordLineEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "}"
    );

    ui->confirmPasswordLabel->setText("确认新密码");
    ui->confirmPasswordLabel->setStyleSheet("font-size: 12px; color: #666666;");

    ui->confirmPasswordLineEdit->setPlaceholderText("请再次输入新密码");
    ui->confirmPasswordLineEdit->setEchoMode(QLineEdit::Password);
    ui->confirmPasswordLineEdit->setFixedHeight(36);
    ui->confirmPasswordLineEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "}"
    );

    ui->saveButton->setText("保存修改");
    ui->saveButton->setFixedSize(200, 36);
    ui->saveButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #12B7F5;"
        "   border: none;"
        "   border-radius: 4px;"
        "   color: #FFFFFF;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #0EA5D9;"
        "}"
        "QPushButton:disabled {"
        "   background-color: #BFBFBF;"
        "}"
    );

    ui->cancelButton->setText("取消");
    ui->cancelButton->setFixedSize(200, 36);
    ui->cancelButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   color: #333333;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #F5F5F5;"
        "}"
    );
}

/**
  * @brief 显示忙碌提示
  */
void UserInfoDialog::ShowBusyMessage()
{
    QMessageBox::information(this, "提示", "在加班了，别急", QMessageBox::Ok);
}

/**
  * @brief 显示身份验证密码对话框
  */
void UserInfoDialog::ShowVerifyPasswordDialog()
{
    bool ok;
    QString verifyPassword = QInputDialog::getText(this, "身份验证",
        "请输入当前密码以验证身份：", QLineEdit::Password, "", &ok);

    if (ok && !verifyPassword.isEmpty())
    {
        isProcessing = true;
        ui->saveButton->setEnabled(false);
        ui->cancelButton->setEnabled(false);

        timeoutTimer->start(5000);

        userManager->UpdateUserInfo(pendingNewUsername, pendingNewPassword, verifyPassword);
    }
}

/**
  * @brief 保存修改按钮点击槽函数
  */
void UserInfoDialog::OnSaveClicked()
{
    if (isProcessing)
    {
        ShowBusyMessage();
        return;
    }

    pendingNewUsername = ui->newUsernameLineEdit->text().trimmed();
    pendingNewPassword = ui->newPasswordLineEdit->text();
    QString confirmPassword = ui->confirmPasswordLineEdit->text();

    if (pendingNewUsername.isEmpty() && pendingNewPassword.isEmpty())
    {
        QMessageBox::warning(this, "错误", "未修改任何信息");
        return;
    }

    if (!pendingNewPassword.isEmpty() && pendingNewPassword != confirmPassword)
    {
        QMessageBox::warning(this, "错误", "两次输入的密码不一致");
        return;
    }

    ShowVerifyPasswordDialog();
}

/**
  * @brief 取消按钮点击槽函数
  */
void UserInfoDialog::OnCancelClicked()
{
    if (isProcessing)
    {
        ShowBusyMessage();
        return;
    }

    reject();
}

/**
  * @brief 修改超时槽函数
  */
void UserInfoDialog::OnUpdateTimeout()
{
    timeoutTimer->stop();
    isProcessing = false;
    ui->saveButton->setEnabled(true);
    ui->cancelButton->setEnabled(true);
    QMessageBox::warning(this, "错误", "请求超时");
}

/**
  * @brief 修改成功槽函数
  */
void UserInfoDialog::OnUpdateSuccess()
{
    timeoutTimer->stop();
    isProcessing = false;
    ui->saveButton->setEnabled(true);
    ui->cancelButton->setEnabled(true);

    QMessageBox::information(this, "成功", "修改成功");
    accept();
}

/**
  * @brief 修改失败槽函数
  * @param errorMsg 错误信息
  */
void UserInfoDialog::OnUpdateFailed(const QString& errorMsg)
{
    timeoutTimer->stop();
    isProcessing = false;
    ui->saveButton->setEnabled(true);
    ui->cancelButton->setEnabled(true);

    if (errorMsg == "验证密码错误")
    {
        QMessageBox::warning(this, "错误", "验证密码错误");
    }
    else if (errorMsg == "用户名已存在")
    {
        QMessageBox::warning(this, "错误", "用户名已存在");
    }
    else if (errorMsg == "服务器接收请求失败")
    {
        QMessageBox::warning(this, "错误", "服务器接收请求失败");
    }
    else if (errorMsg == "服务器罢工了...")
    {
        QMessageBox::warning(this, "错误", "服务器罢工了...");
    }
    else
    {
        QMessageBox::warning(this, "错误", errorMsg);
    }
}