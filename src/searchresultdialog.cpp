#include "searchresultdialog.h"
#include "friendmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
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

    connect(addFriendButton, &QPushButton::clicked, this, &SearchResultDialog::OnAddFriendClicked);
    connect(closeButton, &QPushButton::clicked, this, &SearchResultDialog::OnCloseClicked);
    connect(timeoutTimer, &QTimer::timeout, this, &SearchResultDialog::OnAddFriendTimeout);
    connect(friendManager, &FriendManager::FriendAdded, this, &SearchResultDialog::OnFriendAdded);
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
    usernameLabel->setText("用户名：" + username);
    if(status == UserStatus::Online){
        statusIndicator->setStyleSheet("background-color: #52C41A; border-radius: 6px; border: none;");
        statusLabel->setText("状态：在线");
    }
    else{
        statusIndicator->setStyleSheet("background-color: #BFBFBF; border-radius: 6px; border: none;");
        statusLabel->setText("状态：离线");
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
 * @brief 添加好友按钮被点击后，自动触发自带的信号，自动调用槽函数
 */
void SearchResultDialog::OnAddFriendClicked(){
    if(isProcessing){
        ShowBusyMessage();
        return;
    }
    isProcessing = true;
    addFriendButton->setEnabled(false);
    closeButton->setEnabled(false);
    timeoutTimer->start(5000);
    qDebug() << "[SearchResultDialog::OnAddFriendClicked]添加好友按钮被点击后自动调用槽函数";
    friendManager->AddFriend(queriedUsername);
}

/**
 * @brief 关闭按钮被点击后，自动触发自带的信号，自动调用槽函数
 */
void SearchResultDialog::OnCloseClicked(){
    if(isProcessing){
        ShowBusyMessage();
        return;
    }
    qDebug() << "[SearchResultDialog::OnCloseClicked]关闭按钮被点击后自动调用槽函数";
    reject();
}

/**
 * @brief 时间定时器超时后，自动触发自带的信号，自动调用槽函数
 */
void SearchResultDialog::OnAddFriendTimeout(){
    timeoutTimer->stop();
    isProcessing = false;
    addFriendButton->setEnabled(true);
    closeButton->setEnabled(true);
    QMessageBox::warning(this, "错误", "请求超时");
    qDebug() << "[SearchResultDialog::OnAddFriendTimeout]时间定时器超时后自动调用槽函数";
}

/**
 * @brief 添加好友成功后，手动触发自定义信号，自动调用槽函数
 * @param username 添加成功的用户名
 */
void SearchResultDialog::OnFriendAdded(const QString& username){
    timeoutTimer->stop();
    isProcessing = false;
    QMessageBox::information(this, "成功", "添加好友成功");
    qDebug() << "[SearchResultDialog::OnFriendAdded]添加好友成功后自动调用槽函数";
    accept();
}

/**
 * @brief 添加好友失败后，手动触发自定义信号，自动调用槽函数
 * @param errorMsg 错误信息
 */
void SearchResultDialog::OnFriendAddFailed(const QString& errorMsg){
    timeoutTimer->stop();
    isProcessing = false;
    addFriendButton->setEnabled(true);
    closeButton->setEnabled(true);

    if(errorMsg == "该用户不存在"){
        QMessageBox::warning(this, "错误", "该用户不存在");
    }
    else if(errorMsg == "对方已是您的好友"){
        QMessageBox::information(this, "提示", "对方已是您的好友");
    }
    else if(errorMsg == "怎么能添加自己呢"){
        QMessageBox::warning(this, "错误", "怎么能添加自己呢");
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
    qDebug() << "[SearchResultDialog::OnFriendAddFailed]添加好友失败后自动调用槽函数" << errorMsg;
}