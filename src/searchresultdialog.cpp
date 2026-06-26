#include "searchresultdialog.h"
#include "friendmanager.h"
#include "clientlogger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDebug>

/**
 * @brief SearchResultDialog构造函数，用于初始化类内私有属性+初始化UI+连接信号槽
 * @param friendManager 好友管理器指针
 * @param parent 父窗口
 */
SearchResultDialog::SearchResultDialog(FriendManager* friendManager, QWidget *parent)
    : QDialog(parent)
    , friendManager(friendManager)
    , titleLabel(nullptr)
    , usernameLabel(nullptr)
    , statusLabel(nullptr)
    , statusIndicator(nullptr)
    , addFriendButton(nullptr)
    , closeButton(nullptr)
    , isProcessing(false)
    , timeoutTimer(new QTimer(this))
{
    SetupUI();

    // 添加好友按钮被点击后，自动触发自带的信号，自动调用槽函数
    connect(addFriendButton, &QPushButton::clicked, this, &SearchResultDialog::OnAddFriendClicked);
    // 关闭按钮被点击后，自动触发自带的信号，自动调用槽函数
    connect(closeButton, &QPushButton::clicked, this, &SearchResultDialog::OnCloseClicked);
    // 时间定时器超时后，自动触发自带的信号，自动调用槽函数
    connect(timeoutTimer, &QTimer::timeout, this, &SearchResultDialog::OnAddFriendTimeout);
    // 添加好友成功后，手动触发自定义信号，自动调用槽函数
    connect(friendManager, &FriendManager::FriendAdded, this, &SearchResultDialog::OnFriendAdded);
    // 添加好友失败后，手动触发自定义信号，自动调用槽函数
    connect(friendManager, &FriendManager::FriendAddFailed, this, &SearchResultDialog::OnFriendAddFailed);
}

/**
 * @brief SearchResultDialog析构函数，用于释放动态分配的资源
 */
SearchResultDialog::~SearchResultDialog(){}

/**
 * @brief 设置搜索结果信息
 * @param username 用户名
 * @param status 用户状态
 */
void SearchResultDialog::SetResultInfo(const QString& username, UserStatus status){
    queriedUsername = username;
    usernameLabel->setText("用户名：" + username);  // 设置用户名标签的文本
    if(status == UserStatus::Online){
        statusIndicator->setStyleSheet("background-color: #52C41A; border-radius: 6px; border: none;");  // 设置状态指示器的样式
        statusLabel->setText("状态：在线");
    }
    else{
        statusIndicator->setStyleSheet("background-color: #BFBFBF; border-radius: 6px; border: none;");
        statusLabel->setText("状态：离线");
    }
}

/**
 * @brief 设置是否为仅查看模式（隐藏添加好友按钮）
 * @param viewOnly 是否仅查看
 */
void SearchResultDialog::SetViewOnlyMode(bool viewOnly){
    if(viewOnly){
        addFriendButton->hide();  // 隐藏添加好友按钮
        setWindowTitle("好友资料");  // 设置窗口标题
    }
}

/**
 * @brief 设置搜索结果对话框（UI界面）样式
 */
void SearchResultDialog::SetupUI(){
    setWindowTitle("用户信息");
    setFixedSize(360, 240);
    setMinimumSize(360, 240);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    titleLabel = new QLabel("用户信息", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333;");
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(8);

    QHBoxLayout* usernameLayout = new QHBoxLayout();
    usernameLabel = new QLabel("用户名：", this);
    usernameLabel->setStyleSheet("font-size: 12px; color: #333333;");
    usernameLayout->addWidget(usernameLabel);
    usernameLayout->addStretch();
    mainLayout->addLayout(usernameLayout);

    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusIndicator = new QLabel(this);
    statusIndicator->setFixedSize(12, 12);
    statusIndicator->setStyleSheet("background-color: #BFBFBF; border-radius: 6px; border: none;");
    statusLayout->addWidget(statusIndicator);
    statusLabel = new QLabel("状态：", this);
    statusLabel->setStyleSheet("font-size: 12px; color: #333333;");
    statusLayout->addWidget(statusLabel);
    statusLayout->addStretch();
    mainLayout->addLayout(statusLayout);

    mainLayout->addSpacing(12);

    addFriendButton = new QPushButton("添加好友", this);
    addFriendButton->setFixedSize(200, 36);
    addFriendButton->setStyleSheet(
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
    QHBoxLayout* addBtnLayout = new QHBoxLayout();
    addBtnLayout->addStretch();
    addBtnLayout->addWidget(addFriendButton);
    addBtnLayout->addStretch();
    mainLayout->addLayout(addBtnLayout);

    closeButton = new QPushButton("关闭", this);
    closeButton->setFixedSize(200, 36);
    closeButton->setStyleSheet(
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
    QHBoxLayout* closeBtnLayout = new QHBoxLayout();
    closeBtnLayout->addStretch();
    closeBtnLayout->addWidget(closeButton);
    closeBtnLayout->addStretch();
    mainLayout->addLayout(closeBtnLayout);
}

/**
 * @brief 信息弹窗
 */
void SearchResultDialog::ShowBusyMessage(){
    QMessageBox::information(this, "提示", "在加班了，别急", QMessageBox::Ok);
}

/**
 * @brief 槽函数，用于响应添加好友按钮被点击后自动触发自带的信号
 */
void SearchResultDialog::OnAddFriendClicked(){
    if(isProcessing){
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "SearchResultDialog", "正在验证输入信息格式, 请稍后");
        ShowBusyMessage();
        return;
    }
    isProcessing = true;
    addFriendButton->setEnabled(false);  // 禁用添加好友按钮
    closeButton->setEnabled(false);  // 禁用关闭按钮
    timeoutTimer->start(15000);  // 启动15秒时间定时器
    bool addResult = friendManager->AddFriend(queriedUsername);
    if(addResult){
        ClientLogger::GetInstance().WriteLog(LogLevel::INFO, "SearchResultDialog", "发送添加请求成功");
    }
    else{
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "SearchResultDialog", "发送添加请求失败");
    }
}

/**
 * @brief 槽函数，用于响应关闭按钮被点击后自动触发自带的信号
 */
void SearchResultDialog::OnCloseClicked(){
    if(isProcessing){
        ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "SearchResultDialog", "正在验证输入信息格式, 请稍后");
        ShowBusyMessage();
        return;
    }
    ClientLogger::GetInstance().WriteLog(LogLevel::INFO, "SearchResultDialog", "用户点击了关闭按钮");
    reject();  // 关闭对话框
}

/**
 * @brief 关闭事件重写，防止在处理中通过X按钮关闭对话框
 * @param event 关闭事件
 */
void SearchResultDialog::closeEvent(QCloseEvent* event){
    if(isProcessing){
        event->ignore();  // 忽略关闭事件
        return;
    }
    QDialog::closeEvent(event);  // 处理关闭事件
}

/**
 * @brief 槽函数，用于响应时间定时器超时后自动触发自带的信号
 */
void SearchResultDialog::OnAddFriendTimeout(){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;
    addFriendButton->setEnabled(true);  // 启用添加好友按钮
    closeButton->setEnabled(true);  // 启用关闭按钮
    QMessageBox::warning(this, "错误", "请求超时");  // 错误弹窗
    ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "SearchResultDialog", "时间定时器超时后自动调用槽函数");
}

/**
 * @brief 槽函数，用于响应添加好友成功后手动触发的自定义信号
 * @param username 添加成功的用户名
 */
void SearchResultDialog::OnFriendAdded(const QString& username){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;
    addFriendButton->setEnabled(true);  // 启用添加好友按钮
    closeButton->setEnabled(true);  // 启用关闭按钮
    QMessageBox::information(this, "成功", "添加好友成功");  // 信息弹窗
    ClientLogger::GetInstance().WriteLog(LogLevel::INFO, "SearchResultDialog", "添加成功");
    accept();  // 接受添加成功信号
}

/**
 * @brief 槽函数，用于响应添加好友失败后手动触发的自定义信号
 * @param errorMsg 错误信息
 */
void SearchResultDialog::OnFriendAddFailed(const QString& errorMsg){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;
    addFriendButton->setEnabled(true);  // 启用添加好友按钮
    closeButton->setEnabled(true);  // 启用关闭按钮
    QMessageBox::warning(this, "错误", errorMsg);  // 错误弹窗
    ClientLogger::GetInstance().WriteLog(LogLevel::ERROR, "SearchResultDialog", "添加失败, " + errorMsg);
}