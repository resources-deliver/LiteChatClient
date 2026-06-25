#include "logindialog.h"
#include "ui_logindialog.h"
#include "usermanager.h"

#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>

/**
 * @brief LoginDialog构造函数，用于初始化类内私有属性+初始化UI+连接信号槽
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
    ui->setupUi(this);  // 初始化登录对话框（UI界面）
    SetupUI();

    // 登录按钮被点击后，自动触发自带的信号，自动调用槽函数
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::OnLoginClicked);
    // 注册按钮被点击后，自动触发自带的信号，自动调用槽函数
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginDialog::OnRegisterClicked);
    // 时间定时器超时后，自动触发自带的信号，自动调用槽函数
    connect(timeoutTimer, &QTimer::timeout, this, &LoginDialog::OnLoginTimeout);
    // 登录成功后，手动触发自定义信号，自动调用槽函数
    connect(userManager, &UserManager::LoginSuccess, this, &LoginDialog::OnLoginSuccess);
    // 登录失败后，手动触发自定义信号，自动调用槽函数
    connect(userManager, &UserManager::LoginFailed, this, &LoginDialog::OnLoginFailed);
}

/**
 * @brief LoginDialog析构函数，用于释放动态分配的资源
 */
LoginDialog::~LoginDialog(){
    delete ui;
}

/**
 * @brief 设置用户名（用于注册后自动填充）
 * @param username 用户名
 */
void LoginDialog::SetUsername(const QString& username){
    ui->usernameLineEdit->setText(username);  // 设置用户名文本框的内容为传入的用户名
}

/**
 * @brief 设置登录对话框（UI界面）样式
 */
void LoginDialog::SetupUI(){
    setWindowTitle("登录");  // 设置窗口标题
    setFixedSize(400, 350);  // 设置固定大小
    setMinimumSize(400, 350);  // 设置最小大小

    ui->titleLabel->setText("登录");  // 设置标题文本
    ui->titleLabel->setAlignment(Qt::AlignCenter);  // 设置标题文本居中
    ui->titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333;");  // 设置标题文本样式

    ui->usernameLabel->setText("用户名");  // 设置用户名标签文本
    ui->usernameLabel->setStyleSheet("font-size: 12px; color: #666666;");  // 设置用户名标签文本样式

    ui->usernameLineEdit->setPlaceholderText("请输入用户名");  // 设置用户名文本框占位符文本
    ui->usernameLineEdit->setFixedHeight(36);  // 设置用户名文本框固定高度
    ui->usernameLineEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "}"
    );  // 设置用户名文本框样式

    ui->passwordLabel->setText("密码");  // 设置密码标签文本
    ui->passwordLabel->setStyleSheet("font-size: 12px; color: #666666;");  // 设置密码标签文本样式

    ui->passwordLineEdit->setPlaceholderText("请输入密码");  // 设置密码文本框占位符文本
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);  // 设置密码文本框为密码模式
    ui->passwordLineEdit->setFixedHeight(36);  // 设置密码文本框固定高度
    ui->passwordLineEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "}"
    );  // 设置密码文本框样式

    ui->loginButton->setText("登录");  // 设置登录按钮文本
    ui->loginButton->setFixedSize(200, 36);  // 设置登录按钮固定大小
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
    );  // 设置登录按钮样式

    ui->registerButton->setText("注册");  // 设置注册按钮文本
    ui->registerButton->setFixedSize(200, 36);  // 设置注册按钮固定大小
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
    );  // 设置注册按钮样式
}

/**
 * @brief 信息弹窗
 */
void LoginDialog::ShowBusyMessage(){
    QMessageBox::information(this, "提示", "在加班了，别急", QMessageBox::Ok);
}

/**
 * @brief 按键事件处理，实现Enter键导航
 * @param event 按键事件
 */
void LoginDialog::keyPressEvent(QKeyEvent* event){
    if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter){  // 如果按下Enter键
        if(ui->usernameLineEdit->hasFocus()){  // 如果用户名文本框有焦点
            ui->passwordLineEdit->setFocus();  // 转换焦点到密码文本框
            return;
        }
        if(ui->passwordLineEdit->hasFocus()){
            OnLoginClicked();
            return;
        }
    }
    QDialog::keyPressEvent(event);  // 调用父类的按键事件处理函数
}

/**
 * @brief 槽函数，用于响应登录按钮被点击后自动触发自带的信号
 */
void LoginDialog::OnLoginClicked(){
    if(isProcessing){
        qDebug() << "[LoginDialog::OnLoginClicked]正在处理登录请求, 请稍后";
        ShowBusyMessage();
        return;
    }
    QString username = ui->usernameLineEdit->text().trimmed();  // 获取用户名文本框的内容并去掉首尾空格
    QString password = ui->passwordLineEdit->text();  // 获取密码文本框的内容
    if(username.isEmpty() || password.isEmpty()){  // 如果用户名或密码为空
        qDebug() << "[LoginDialog::OnLoginClicked]用户名或密码不能为空";
        QMessageBox::warning(this, "错误", "用户名或密码不能为空");  // 错误弹窗
        return;
    }
    isProcessing = true;
    ui->loginButton->setEnabled(false);  // 禁用登录按钮
    ui->registerButton->setEnabled(false);  // 禁用注册按钮
    timeoutTimer->start(5000);  // 启动5秒时间定时器
    qDebug() << "[LoginDialog::OnLoginClicked]客户端发送登录请求到服务器";
    userManager->LoginUser(username, password);
}

/**
 * @brief 槽函数，用于响应注册按钮被点击后自动触发自带的信号
 */
void LoginDialog::OnRegisterClicked(){
    if(isProcessing){
        qDebug() << "[LoginDialog::OnRegisterClicked]正在处理登录请求, 请稍后";
        ShowBusyMessage();
        return;
    }
    qDebug() << "[LoginDialog::OnRegisterClicked]进入注册页面";
    done(2);  // 关闭登录对话框并返回注册页面
}

/**
 * @brief 槽函数，用于响应时间定时器超时后自动触发自带的信号
 */
void LoginDialog::OnLoginTimeout(){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;
    ui->loginButton->setEnabled(true);  // 启用登录按钮
    ui->registerButton->setEnabled(true);  // 启用注册按钮
    QMessageBox::warning(this, "错误", "请求超时");  // 错误弹窗
    qDebug() << "[LoginDialog::OnLoginTimeout]时间定时器超时后自动调用槽函数";
}

/**
 * @brief 槽函数，用于响应登录成功后手动触发的自定义信号
 */
void LoginDialog::OnLoginSuccess(){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;
    qDebug() << "[LoginDialog::OnLoginSuccess]客户端接收登录成功响应,登录成功";
    accept();  // 接受登录成功信号
}

/**
 * @brief 槽函数，用于响应登录失败后手动触发的自定义信号
 * @param errorMsg 错误信息
 */
void LoginDialog::OnLoginFailed(const QString& errorMsg){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;
    ui->loginButton->setEnabled(true);  // 启用登录按钮
    ui->registerButton->setEnabled(true);  // 启用注册按钮
    if(errorMsg == "该用户不存在"){
        QMessageBox::warning(this, "错误", "该用户不存在");  // 错误弹窗
    }
    else if(errorMsg == "用户名或密码错误"){
        QMessageBox::warning(this, "错误", "用户名或密码错误");
    }
    else if(errorMsg == "该账号已在其他地方登录"){
        QMessageBox::warning(this, "错误", "该账号已在其他地方登录");
    }
    else if(errorMsg == "服务器接收请求失败"){
        QMessageBox::warning(this, "错误", "服务器接收请求失败");
    }
    else if(errorMsg == "服务器罢工了..."){
        QMessageBox::warning(this, "错误", "服务器罢工了...");
    }
    else{
        QMessageBox::warning(this, "错误", errorMsg);
    }
    qDebug() << "[LoginDialog::OnLoginFailed]客户端接收登录失败响应, 错误信息: " << errorMsg;
}