#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "usermanager.h"

#include <QMessageBox>

/**
  * @brief RegisterDialog构造函数
  * @param userManager 用户管理器指针
  * @param parent 父窗口
  */
RegisterDialog::RegisterDialog(UserManager* userManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
    , userManager(userManager)
    , isProcessing(false)
    , timeoutTimer(new QTimer(this))
{
    ui->setupUi(this);
    SetupUI();

    connect(ui->registerButton, &QPushButton::clicked, this, &RegisterDialog::OnRegisterClicked);
    connect(ui->backButton, &QPushButton::clicked, this, &RegisterDialog::OnBackClicked);
    connect(timeoutTimer, &QTimer::timeout, this, &RegisterDialog::OnRegisterTimeout);
    connect(userManager, &UserManager::RegisterSuccess, this, &RegisterDialog::OnRegisterSuccess);
    connect(userManager, &UserManager::RegisterFailed, this, &RegisterDialog::OnRegisterFailed);
}

/**
  * @brief RegisterDialog析构函数
  */
RegisterDialog::~RegisterDialog()
{
    delete ui;
}

/**
  * @brief 获取注册的用户名
  * @return 注册成功的用户名
  */
QString RegisterDialog::GetRegisteredUsername() const
{
    return registeredUsername;
}

/**
  * @brief 设置UI界面样式
  */
void RegisterDialog::SetupUI()
{
    setWindowTitle("注册");
    setFixedSize(400, 400);
    setMinimumSize(400, 400);

    ui->titleLabel->setText("注册");
    ui->titleLabel->setAlignment(Qt::AlignCenter);
    ui->titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333;");

    ui->usernameLabel->setText("用户名");
    ui->usernameLabel->setStyleSheet("font-size: 12px; color: #666666;");

    ui->usernameLineEdit->setPlaceholderText("请输入用户名（3-20位）");
    ui->usernameLineEdit->setFixedHeight(36);
    ui->usernameLineEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "}"
    );

    ui->passwordLabel->setText("密码");
    ui->passwordLabel->setStyleSheet("font-size: 12px; color: #666666;");

    ui->passwordLineEdit->setPlaceholderText("请输入密码（至少6位）");
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    ui->passwordLineEdit->setFixedHeight(36);
    ui->passwordLineEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "}"
    );

    ui->confirmPasswordLabel->setText("确认密码");
    ui->confirmPasswordLabel->setStyleSheet("font-size: 12px; color: #666666;");

    ui->confirmPasswordLineEdit->setPlaceholderText("请再次输入密码");
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

    ui->registerButton->setText("注册");
    ui->registerButton->setFixedSize(200, 36);
    ui->registerButton->setStyleSheet(
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

    ui->backButton->setText("返回登录");
    ui->backButton->setFixedSize(200, 36);
    ui->backButton->setStyleSheet(
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
void RegisterDialog::ShowBusyMessage()
{
    QMessageBox::information(this, "提示", "在加班了，别急", QMessageBox::Ok);
}

/**
  * @brief 注册按钮点击槽函数
  */
void RegisterDialog::OnRegisterClicked()
{
    if (isProcessing)
    {
        ShowBusyMessage();
        return;
    }

    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text();
    QString confirmPassword = ui->confirmPasswordLineEdit->text();

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "错误", "用户名或密码不能为空");
        return;
    }

    if (password != confirmPassword)
    {
        QMessageBox::warning(this, "错误", "两次输入的密码不一致");
        return;
    }

    isProcessing = true;
    ui->registerButton->setEnabled(false);
    ui->backButton->setEnabled(false);

    timeoutTimer->start(5000);

    userManager->RegisterUser(username, password);
}

/**
  * @brief 返回登录按钮点击槽函数
  */
void RegisterDialog::OnBackClicked()
{
    if (isProcessing)
    {
        ShowBusyMessage();
        return;
    }

    done(1);
}

/**
  * @brief 注册超时槽函数
  */
void RegisterDialog::OnRegisterTimeout()
{
    timeoutTimer->stop();
    isProcessing = false;
    ui->registerButton->setEnabled(true);
    ui->backButton->setEnabled(true);
    QMessageBox::warning(this, "错误", "请求超时");
}

/**
  * @brief 注册成功槽函数
  */
void RegisterDialog::OnRegisterSuccess()
{
    timeoutTimer->stop();
    isProcessing = false;
    ui->registerButton->setEnabled(true);
    ui->backButton->setEnabled(true);

    registeredUsername = ui->usernameLineEdit->text().trimmed();

    QMessageBox::information(this, "成功", "注册成功");
    done(0);
}

/**
  * @brief 注册失败槽函数
  * @param errorMsg 错误信息
  */
void RegisterDialog::OnRegisterFailed(const QString& errorMsg)
{
    timeoutTimer->stop();
    isProcessing = false;
    ui->registerButton->setEnabled(true);
    ui->backButton->setEnabled(true);

    if (errorMsg == "用户名已存在")
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