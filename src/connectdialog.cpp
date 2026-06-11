#include "connectdialog.h"
#include "ui_connectdialog.h"
#include "networkmanager.h"

#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

/**
 * @brief ConnectDialog构造函数
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
    ui->setupUi(this);
    SetupUI();

    connect(ui->connectButton, &QPushButton::clicked, this, &ConnectDialog::OnConnectClicked);
    connect(ui->skipButton, &QPushButton::clicked, this, &ConnectDialog::OnSkipClicked);
    connect(timeoutTimer, &QTimer::timeout, this, &ConnectDialog::OnConnectionTimeout);
    connect(networkManager, &NetworkManager::Connected, this, &ConnectDialog::OnConnected);
    connect(networkManager, &NetworkManager::ErrorOccurred, this, &ConnectDialog::OnError);
}

/**
 * @brief ConnectDialog析构函数
 */
ConnectDialog::~ConnectDialog(){
    delete ui;
}

/**
 * @brief 设置UI界面样式
 */
void ConnectDialog::SetupUI(){
    setWindowTitle("连接服务器");
    setFixedSize(400, 300);
    setMinimumSize(400, 300);

    ui->titleLabel->setText("连接服务器");
    ui->titleLabel->setAlignment(Qt::AlignCenter);
    ui->titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333;");

    ui->hintLabel->setText("请输入服务器IP地址");
    ui->hintLabel->setStyleSheet("font-size: 12px; color: #666666;");

    ui->ipLineEdit->setPlaceholderText("请输入服务器IP地址");
    ui->ipLineEdit->setFixedHeight(36);
    ui->ipLineEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "}"
    );

    ui->skipButton->setText("跳过");
    ui->skipButton->setFixedSize(120, 36);
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
    );

    ui->connectButton->setText("连接");
    ui->connectButton->setFixedSize(120, 36);
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
    );
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
 * @brief 显示忙碌提示
 */
void ConnectDialog::ShowBusyMessage(){
    QMessageBox::information(this, "提示", "在加班了，别急", QMessageBox::Ok);
}

/**
 * @brief 执行连接操作
 * @param ip 服务器IP地址
 */
void ConnectDialog::PerformConnection(const QString& ip){
    if(isProcessing){
        ShowBusyMessage();
        return;
    }

    if(!ValidateIP(ip)){
        QMessageBox::warning(this, "错误", "IP地址不符合格式");
        return;
    }

    isProcessing = true;
    ui->connectButton->setEnabled(false);
    ui->skipButton->setEnabled(false);
    timeoutTimer->start(5000);

    if(!networkManager->ConnectToServer(ip, 8886)){
        timeoutTimer->stop();
        isProcessing = false;
        ui->connectButton->setEnabled(true);
        ui->skipButton->setEnabled(true);
    }
}

/**
 * @brief 连接按钮点击槽函数
 */
void ConnectDialog::OnConnectClicked(){
    QString ip = ui->ipLineEdit->text().trimmed();

    if(ip.isEmpty()){
        QMessageBox::warning(this, "错误", "请输入服务器IP地址");
        return;
    }

    PerformConnection(ip);
}

/**
 * @brief 跳过按钮点击槽函数
 */
void ConnectDialog::OnSkipClicked(){
    PerformConnection(DEFAULT_IP);
}

/**
 * @brief 连接超时槽函数
 */
void ConnectDialog::OnConnectionTimeout(){
    timeoutTimer->stop();
    isProcessing = false;
    ui->connectButton->setEnabled(true);
    ui->skipButton->setEnabled(true);
    QMessageBox::warning(this, "错误", "请求超时");
}

/**
 * @brief 连接成功槽函数
 */
void ConnectDialog::OnConnected(){
    timeoutTimer->stop();
    isProcessing = false;
    accept();
}

/**
 * @brief 错误发生槽函数
 * @param errorMsg 错误信息
 */
void ConnectDialog::OnError(const QString& errorMsg){
    timeoutTimer->stop();
    isProcessing = false;
    ui->connectButton->setEnabled(true);
    ui->skipButton->setEnabled(true);

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
}