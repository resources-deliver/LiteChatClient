#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "usermanager.h"
#include "clientlogger.h"

#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>

/**
 * @brief RegisterDialog构造函数，用于初始化类内私有属性+初始化UI+连接信号槽
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
    ui->setupUi(this);  // 初始化注册对话框（UI界面）
    SetupUI();

    // 注册按钮被点击后，自动触发自带的信号，自动调用槽函数
    connect(ui->registerButton, &QPushButton::clicked, this, &RegisterDialog::OnRegisterClicked);
    // 返回按钮被点击后，自动触发自带的信号，自动调用槽函数
    connect(ui->backButton, &QPushButton::clicked, this, &RegisterDialog::OnBackClicked);
    // 时间定时器超时后，自动触发自带的信号，自动调用槽函数
    connect(timeoutTimer, &QTimer::timeout, this, &RegisterDialog::OnRegisterTimeout);
    // 注册成功后，手动触发自定义信号，自动调用槽函数
    connect(userManager, &UserManager::RegisterSuccess, this, &RegisterDialog::OnRegisterSuccess);
    // 注册失败后，手动触发自定义信号，自动调用槽函数
    connect(userManager, &UserManager::RegisterFailed, this, &RegisterDialog::OnRegisterFailed);
}

/**
 * @brief RegisterDialog析构函数，用于释放动态分配的资源
 */
RegisterDialog::~RegisterDialog(){
    delete ui;
}

/**
 * @brief 获取注册的用户名
 * @return 注册成功的用户名
 */
QString RegisterDialog::GetRegisteredUsername() const{
    return registeredUsername;
}

/**
 * @brief 设置注册对话框（UI界面）样式
 */
void RegisterDialog::SetupUI(){
    setWindowTitle("注册");  // 设置窗口标题
    setFixedSize(400, 400);  // 设置固定大小
    setMinimumSize(400, 400);  // 设置最小大小

    ui->titleLabel->setText("注册");  // 设置标题文本
    ui->titleLabel->setAlignment(Qt::AlignCenter);  // 设置标题文本居中
    ui->titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333;");  // 设置标题文本样式

    ui->usernameLabel->setText("用户名");  // 设置用户名标签文本
    ui->usernameLabel->setStyleSheet("font-size: 12px; color: #666666;");  // 设置用户名标签文本样式

    ui->usernameLineEdit->setPlaceholderText("请输入用户名（3-20位）");  // 设置用户名文本框占位符文本
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

    ui->passwordLineEdit->setPlaceholderText("请输入密码（至少6位）");  // 设置密码文本框占位符文本
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

    ui->confirmPasswordLabel->setText("确认密码");  // 设置确认密码标签文本
    ui->confirmPasswordLabel->setStyleSheet("font-size: 12px; color: #666666;");  // 设置确认密码标签文本样式

    ui->confirmPasswordLineEdit->setPlaceholderText("请再次输入密码");  // 设置确认密码文本框占位符文本
    ui->confirmPasswordLineEdit->setEchoMode(QLineEdit::Password);  // 设置确认密码文本框为密码模式
    ui->confirmPasswordLineEdit->setFixedHeight(36);  // 设置确认密码文本框固定高度
    ui->confirmPasswordLineEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "}"
    );  // 设置确认密码文本框样式

    ui->registerButton->setText("注册");  // 设置注册按钮文本
    ui->registerButton->setFixedSize(200, 36);  // 设置注册按钮固定大小
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
    );  // 设置注册按钮样式

    ui->backButton->setText("返回登录");  // 设置返回按钮文本
    ui->backButton->setFixedSize(200, 36);  // 设置返回按钮固定大小
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
    );  // 设置返回按钮样式
}

/**
 * @brief 信息弹窗
 */
void RegisterDialog::ShowBusyMessage(){
    QMessageBox::information(this, "提示", "在加班了，别急", QMessageBox::Ok);
}

/**
 * @brief 按键事件处理，实现Enter键导航
 * @param event 按键事件
 */
void RegisterDialog::keyPressEvent(QKeyEvent* event){
    if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter){  // 如果按下Enter键
        if(ui->usernameLineEdit->hasFocus()){  // 如果用户名文本框有焦点
            ui->passwordLineEdit->setFocus();  // 转换焦点到密码文本框
            return;
        }
        if(ui->passwordLineEdit->hasFocus()){
            ui->confirmPasswordLineEdit->setFocus();
            return;
        }
        if(ui->confirmPasswordLineEdit->hasFocus()){
            OnRegisterClicked();
            return;
        }
    }
    QDialog::keyPressEvent(event);  // 调用父类的按键事件处理函数
}

/**
 * @brief 槽函数，用于响应注册按钮被点击后自动触发自带的信号
 */
void RegisterDialog::OnRegisterClicked(){
    if(isProcessing){
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "RegisterDialog", "正在验证输入信息格式, 请稍后");
        ShowBusyMessage();
        return;
    }
    QString username = ui->usernameLineEdit->text().trimmed();  // 获取用户名文本框内容并去掉首尾空格
    QString password = ui->passwordLineEdit->text();  // 获取密码文本框内容
    QString confirmPassword = ui->confirmPasswordLineEdit->text();  // 获取确认密码文本框内容
    if(username.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()){  // 如果用户名或密码或确认密码为空
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "RegisterDialog", "注册失败, 用户名或密码或确认密码为空");
        QMessageBox::warning(this, "错误", "用户名或密码不能为空");  // 错误弹窗
        return;
    }
    if(password != confirmPassword){
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "RegisterDialog", "注册失败, 两次输入的密码不一致");
        QMessageBox::warning(this, "错误", "两次输入的密码不一致");
        return;
    }
    isProcessing = true;
    ui->registerButton->setEnabled(false);  // 禁用注册按钮
    ui->backButton->setEnabled(false);  // 禁用返回按钮
    timeoutTimer->start(5000);  // 启动5秒时间定时器
    bool registerResult = userManager->RegisterUser(username, password);
    if(registerResult){
        ClientLogger::GetInstance().WriteLog(LogLevel::INFO, "RegisterDialog", "发送注册请求成功");
    }
    else{
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "RegisterDialog", "发送注册请求失败");
    }
}

/**
 * @brief 槽函数，用于响应返回按钮被点击后自动触发自带的信号
 */
void RegisterDialog::OnBackClicked(){
    if(isProcessing){
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "RegisterDialog", "正在验证输入信息格式, 请稍后");
        ShowBusyMessage();
        return;
    }
    ClientLogger::GetInstance().WriteLog(LogLevel::INFO, "RegisterDialog", "返回登录页面");
    done(1);  // 关闭注册对话框并返回登录页面
}

/**
 * @brief 槽函数，用于响应时间定时器超时后自动触发自带的信号
 */
void RegisterDialog::OnRegisterTimeout(){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;
    ui->registerButton->setEnabled(true);  // 启用注册按钮
    ui->backButton->setEnabled(true);  // 启用返回按钮
    QMessageBox::warning(this, "错误", "请求超时");  // 错误弹窗
    ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "RegisterDialog", "时间定时器超时后自动调用槽函数");
}

/**
 * @brief 槽函数，用于响应注册成功后手动触发的自定义信号
 */
void RegisterDialog::OnRegisterSuccess(){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;
    ui->registerButton->setEnabled(true);  // 启用注册按钮
    ui->backButton->setEnabled(true);  // 启用返回按钮
    registeredUsername = ui->usernameLineEdit->text().trimmed();  // 获取注册成功的用户名
    QMessageBox::information(this, "成功", "注册成功");  // 成功弹窗
    ClientLogger::GetInstance().WriteLog(LogLevel::INFO, "RegisterDialog", "注册成功");
    done(0);  // 关闭注册对话框并返回登录页面
}

/**
 * @brief 槽函数，用于响应注册失败后手动触发的自定义信号
 * @param errorMsg 错误信息
 */
void RegisterDialog::OnRegisterFailed(const QString& errorMsg){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;
    ui->registerButton->setEnabled(true);  // 启用注册按钮
    ui->backButton->setEnabled(true);  // 启用返回按钮
    QMessageBox::warning(this, "错误", errorMsg);  // 错误弹窗
    ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "RegisterDialog", "注册失败, " + errorMsg);
}