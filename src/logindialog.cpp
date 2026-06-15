#include "logindialog.h"
#include "ui_logindialog.h"
#include "usermanager.h"

#include <QMessageBox>
#include <QKeyEvent>

/**
  * @brief LoginDialog构造函数
  * @param userManager 用户管理器指针
  * @param parent 父窗口
  */
LoginDialog::LoginDialog(UserManager* userManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , userManager(userManager)
    , isProcessing(false)
    , timeoutTimer(new QTimer(this))
{
    ui->setupUi(this);
    SetupUI();

    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::OnLoginClicked);
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginDialog::OnRegisterClicked);
    connect(timeoutTimer, &QTimer::timeout, this, &LoginDialog::OnLoginTimeout);
    connect(userManager, &UserManager::LoginSuccess, this, &LoginDialog::OnLoginSuccess);
    connect(userManager, &UserManager::LoginFailed, this, &LoginDialog::OnLoginFailed);
}

/**
  * @brief LoginDialog析构函数
  */
LoginDialog::~LoginDialog()
{
    delete ui;
}

/**
  * @brief 设置用户名（用于注册后自动填充）
  * @param username 用户名
  */
void LoginDialog::SetUsername(const QString& username)
{
    ui->usernameLineEdit->setText(username);
}

/**
  * @brief 设置UI界面样式
  */
void LoginDialog::SetupUI()
{
    setWindowTitle("登录");
    setFixedSize(400, 350);
    setMinimumSize(400, 350);

    ui->titleLabel->setText("登录");
    ui->titleLabel->setAlignment(Qt::AlignCenter);
    ui->titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333;");

    ui->usernameLabel->setText("用户名");
    ui->usernameLabel->setStyleSheet("font-size: 12px; color: #666666;");

    ui->usernameLineEdit->setPlaceholderText("请输入用户名");
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

    ui->passwordLineEdit->setPlaceholderText("请输入密码");
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

    ui->loginButton->setText("登录");
    ui->loginButton->setFixedSize(200, 36);
    ui->loginButton->setStyleSheet(
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

    ui->registerButton->setText("注册");
    ui->registerButton->setFixedSize(200, 36);
    ui->registerButton->setStyleSheet(
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
void LoginDialog::ShowBusyMessage()
{
    QMessageBox::information(this, "提示", "在加班了，别急", QMessageBox::Ok);
}

/**
  * @brief 登录按钮点击槽函数
  */
void LoginDialog::OnLoginClicked()
{
    if (isProcessing)
    {
        ShowBusyMessage();
        return;
    }

    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text();

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "错误", "用户名或密码不能为空");
        return;
    }

    isProcessing = true;
    ui->loginButton->setEnabled(false);
    ui->registerButton->setEnabled(false);

    timeoutTimer->start(5000);

    userManager->LoginUser(username, password);
}

/**
  * @brief 注册按钮点击槽函数
  */
void LoginDialog::OnRegisterClicked()
{
    if (isProcessing)
    {
        ShowBusyMessage();
        return;
    }

    done(1);
}

/**
  * @brief 登录超时槽函数
  */
void LoginDialog::OnLoginTimeout()
{
    timeoutTimer->stop();
    isProcessing = false;
    ui->loginButton->setEnabled(true);
    ui->registerButton->setEnabled(true);
    QMessageBox::warning(this, "错误", "请求超时");
}

/**
  * @brief 登录成功槽函数
  */
void LoginDialog::OnLoginSuccess()
{
    timeoutTimer->stop();
    isProcessing = false;
    accept();
}

/**
  * @brief 登录失败槽函数
  * @param errorMsg 错误信息
  */
void LoginDialog::OnLoginFailed(const QString& errorMsg)
{
    timeoutTimer->stop();
    isProcessing = false;
    ui->loginButton->setEnabled(true);
    ui->registerButton->setEnabled(true);

    if (errorMsg == "该用户不存在")
    {
        QMessageBox::warning(this, "错误", "该用户不存在");
    }
    else if (errorMsg == "用户名或密码错误")
    {
        QMessageBox::warning(this, "错误", "用户名或密码错误");
    }
    else if (errorMsg == "该账号已在其他地方登录")
    {
        QMessageBox::warning(this, "错误", "该账号已在其他地方登录");
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