#include "connectdialog.h"
#include "ui_connectdialog.h"
#include "networkmanager.h"
#include "clientlogger.h"

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
    SetupUI();

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
    delete ui;
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
    QRegularExpression regex(
        "^((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]|2[0-4]\\d|[01]?\\d\\d?)$"
    );  // 编译正则表达式
    bool result = regex.match(ip).hasMatch();  // 匹配IP地址字符串
    return result;
}

/**
 * @brief 信息弹窗
 */
void ConnectDialog::ShowBusyMessage(){
    QMessageBox::information(this, "提示", "在加班了，别急", QMessageBox::Ok);
}

/**
 * @brief 执行连接服务器操作
 * @param ip 服务器IP地址
 */
void ConnectDialog::PerformConnection(const QString& ip){
    if(!ValidateIP(ip)){
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "ConnectDialog", "连接失败, IP地址格式不符合要求");
        QMessageBox::warning(this, "错误", "IP地址不符合格式");  // 错误弹窗
        return;
    }
    isProcessing = true;
    ui->connectButton->setEnabled(false);  // 禁用连接按钮
    ui->skipButton->setEnabled(false);  // 禁用跳过按钮
    timeoutTimer->start(5000);  // 启动5秒时间定时器
    bool result = networkManager->ConnectToServer(ip, 8886);
    if(result){
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "ConnectDialog", "发送连接请求成功");
    }
    else{
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "ConnectDialog", "发送连接请求失败");
    }
}

/**
 * @brief 槽函数，用于响应连接按钮被点击后自动触发自带的信号
 */
void ConnectDialog::OnConnectClicked(){
    if(isProcessing){
        ClientLogger::GetInstance().WriteLog(LogLevel::INFO, "ConnectDialog", "正在验证输入信息格式, 请稍后");
        ShowBusyMessage();
        return;
    }
    QString ip = ui->ipLineEdit->text().trimmed();  // 获取IP地址输入框中的文本并去掉首尾空格
    if(ip.isEmpty()){  // 如果输入框IP地址为空
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "ConnectDialog", "连接失败, 输入框IP地址为空");
        QMessageBox::warning(this, "错误", "请输入服务器IP地址");  // 错误弹窗
        return;
    }
    PerformConnection(ip);
}

/**
 * @brief 槽函数，用于响应跳过按钮被点击后自动触发自带的信号
 */
void ConnectDialog::OnSkipClicked(){
    if(isProcessing){
        ClientLogger::GetInstance().WriteLog(LogLevel::INFO, "ConnectDialog", "正在验证输入信息格式, 请稍后");
        ShowBusyMessage();
        return;
    }
    PerformConnection(DEFAULT_IP);
}

/**
 * @brief 槽函数，用于响应时间定时器超时后自动触发自带的信号
 */
void ConnectDialog::OnConnectionTimeout(){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;
    ui->connectButton->setEnabled(true);  // 启用连接按钮
    ui->skipButton->setEnabled(true);  // 启用跳过按钮
    QMessageBox::warning(this, "错误", "请求超时");  // 错误弹窗
    ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "ConnectDialog", "时间定时器超时后自动调用槽函数");
}

/**
 * @brief 槽函数，用于响应连接成功后手动触发的自定义信号
 */
void ConnectDialog::OnConnected(){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;
    ui->connectButton->setEnabled(true);  // 启用连接按钮
    ui->skipButton->setEnabled(true);  // 启用跳过按钮
    ClientLogger::GetInstance().WriteLog(LogLevel::INFO, "ConnectDialog", "连接成功");
    accept();  // 接受连接成功信号
}

/**
 * @brief 槽函数，用于响应连接失败后手动触发的自定义信号
 * @param errorMsg 错误信息
 */
void ConnectDialog::OnError(const QString& errorMsg){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;
    ui->connectButton->setEnabled(true);  // 启用连接按钮
    ui->skipButton->setEnabled(true);  // 启用跳过按钮
    QMessageBox::warning(this, "错误", errorMsg);  // 错误弹窗
    ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "ConnectDialog", "连接失败, " + errorMsg);
}