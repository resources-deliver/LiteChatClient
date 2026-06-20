#include "connectdialog.h"
#include "ui_connectdialog.h"
#include "networkmanager.h"

#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDebug>

/**
 * @brief ConnectDialog构造函数，用于初始化类内私有属性+初始化UI+连接信号槽
 * @param networkManager 网络管理器指针
 * @param parent 父窗口
 */
ConnectDialog::ConnectDialog(NetworkManager* networkManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConnectDialog)
    , networkManager(networkManager)
    , isProcessing(false)
    , timeoutTimer(new QTimer(this))
{
    ui->setupUi(this);  // 初始化连接对话框（UI界面）
    SetupUI();  // 设置连接对话框（UI界面）样式

    // 连接按钮被点击后，自动触发自带的信号，自动调用槽函数
    connect(ui->connectButton, &QPushButton::clicked, this, &ConnectDialog::OnConnectClicked);
    // 跳过按钮被点击后，自动触发自带的信号，自动调用槽函数
    connect(ui->skipButton, &QPushButton::clicked, this, &ConnectDialog::OnSkipClicked);
    // 时间定时器超时后，自动触发自带的信号，自动调用槽函数
    connect(timeoutTimer, &QTimer::timeout, this, &ConnectDialog::OnConnectionTimeout);
    // 连接成功后，手动触发自定义信号，自动调用槽函数
    connect(networkManager, &NetworkManager::Connected, this, &ConnectDialog::OnConnected);
    // 连接失败后，手动触发自定义信号，自动调用槽函数
    connect(networkManager, &NetworkManager::ErrorOccurred, this, &ConnectDialog::OnError);
}

/**
 * @brief ConnectDialog析构函数，用于释放动态分配的资源
 */
ConnectDialog::~ConnectDialog(){
    delete ui;  // 释放连接对话框（UI界面）的指针
}

/**
 * @brief 设置连接对话框（UI界面）样式
 */
void ConnectDialog::SetupUI(){
    setWindowTitle("连接服务器");  // 设置窗口标题
    setFixedSize(400, 300);  // 设置固定大小
    setMinimumSize(400, 300);  // 设置最小大小

    ui->titleLabel->setText("连接服务器");  // 设置标题文本
    ui->titleLabel->setAlignment(Qt::AlignCenter);  // 设置标题文本居中
    ui->titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333;");  // 设置标题文本样式

    ui->hintLabel->setText("请输入服务器IP地址");  // 设置提示文本
    ui->hintLabel->setStyleSheet("font-size: 12px; color: #666666;");  // 设置提示文本样式

    ui->ipLineEdit->setPlaceholderText("请输入服务器IP地址");  // 设置IP地址输入框占位符文本
    ui->ipLineEdit->setFixedHeight(36);  // 设置IP地址输入框固定高度
    ui->ipLineEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "}"
    );  // 设置IP地址输入框样式

    ui->skipButton->setText("跳过");  // 设置跳过按钮文本
    ui->skipButton->setFixedSize(120, 36);  // 设置跳过按钮固定大小
    ui->skipButton->setStyleSheet(
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
    );  // 设置跳过按钮样式

    ui->connectButton->setText("连接");  // 设置连接按钮文本
    ui->connectButton->setFixedSize(120, 36);  // 设置连接按钮固定大小
    ui->connectButton->setStyleSheet(
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
    );  // 设置连接按钮样式
}

/**
 * @brief 验证IP地址格式
 * @param ip IP地址字符串
 * @return 是否合法
 */
bool ConnectDialog::ValidateIP(const QString& ip){
    QRegularExpression regex("^((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]|2[0-4]\\d|[01]?\\d\\d?)$");
    return regex.match(ip).hasMatch();
}

/**
 * @brief 信息弹窗
 */
void ConnectDialog::ShowBusyMessage(){
    QMessageBox::information(this, "提示", "在加班了，别急", QMessageBox::Ok);
}

/**
 * @brief 执行连接请求操作
 * @param ip 服务器IP地址
 */
void ConnectDialog::PerformConnection(const QString& ip){
    if(isProcessing){  // 如果正在处理连接请求
        qDebug() << "[ConnectDialog::PerformConnection]已经正在处理连接请求";
        ShowBusyMessage();  // 信息弹窗
        return;
    }
    if(!ValidateIP(ip)){  // 如果IP地址格式不符合要求
        qDebug() << "[ConnectDialog::PerformConnection]IP地址格式不符合要求";
        QMessageBox::warning(this, "错误", "IP地址不符合格式");  // 错误弹窗
        return;
    }
    isProcessing = true;  // 设置连接请求状态
    ui->connectButton->setEnabled(false);  // 禁用连接按钮
    ui->skipButton->setEnabled(false);  // 禁用跳过按钮
    timeoutTimer->start(5000);  // 启动5秒时间定时器
    bool result = networkManager->ConnectToServer(ip, 8886);  // 连接到服务器
    if(!result){  // 如果连接失败
        timeoutTimer->stop();  // 停止时间定时器
        isProcessing = false;  // 设置连接请求状态
        ui->connectButton->setEnabled(true);  // 启用连接按钮
        ui->skipButton->setEnabled(true);  // 启用跳过按钮
        qDebug() << "[ConnectDialog::PerformConnection]连接服务器请求失败";
        return;
    }
    qDebug() << "[ConnectDialog::PerformConnection]连接服务器请求成功";
}

/**
 * @brief 连接按钮被点击后，自动触发自带的信号，自动调用槽函数
 */
void ConnectDialog::OnConnectClicked(){
    QString ip = ui->ipLineEdit->text().trimmed();  // 获取IP地址输入框中的文本并去掉首尾空格
    if(ip.isEmpty()){  // 如果输入框IP地址为空
        qDebug() << "[ConnectDialog::OnConnectClicked]输入框IP地址为空";
        QMessageBox::warning(this, "错误", "请输入服务器IP地址");  // 错误弹窗
        return;
    }
    qDebug() << "[ConnectDialog::OnConnectClicked]客户端发送连接请求到服务器";
    PerformConnection(ip);  // 执行连接请求操作
}

/**
 * @brief 跳过按钮被点击后，自动触发自带的信号，自动调用槽函数
 */
void ConnectDialog::OnSkipClicked(){
    qDebug() << "[ConnectDialog::OnSkipClicked]客户端发送连接请求到服务器";
    PerformConnection(DEFAULT_IP);  // 执行连接请求操作
}

/**
 * @brief 时间定时器超时后，自动触发自带的信号，自动调用槽函数
 */
void ConnectDialog::OnConnectionTimeout(){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;  // 设置连接请求状态
    ui->connectButton->setEnabled(true);  // 启用连接按钮
    ui->skipButton->setEnabled(true);  // 启用跳过按钮
    QMessageBox::warning(this, "错误", "请求超时");  // 错误弹窗
    qDebug() << "[ConnectDialog::OnConnectionTimeout]时间定时器超时后自动调用槽函数";  // Debug输出
}

/**
 * @brief 连接成功后，手动触发自定义信号，自动调用槽函数
 */
void ConnectDialog::OnConnected(){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;  // 设置连接请求状态
    qDebug() << "[ConnectDialog::OnConnected]客户端接收连接成功响应";
    accept();  // 接受连接成功信号
}

/**
 * @brief 连接失败后，手动触发自定义信号，自动调用槽函数
 * @param errorMsg 错误信息
 */
void ConnectDialog::OnError(const QString& errorMsg){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;  // 设置连接请求状态
    ui->connectButton->setEnabled(true);  // 启用连接按钮
    ui->skipButton->setEnabled(true);  // 启用跳过按钮

    if(errorMsg == "服务器下机了"){
        QMessageBox::warning(this, "错误", "服务器下机了");
    }
    else if(errorMsg.contains("接收请求失败")){
        QMessageBox::warning(this, "错误", "服务器接收请求失败");
    }
    else if(errorMsg.contains("内部错误")){
        QMessageBox::warning(this, "错误", "服务器罢工了...");
    }
    else{
        QMessageBox::warning(this, "错误", errorMsg);
    }
    qDebug() << "[ConnectDialog::OnError]客户端接收连接失败响应, 错误信息: " << errorMsg;
}