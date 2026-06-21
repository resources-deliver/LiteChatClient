#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "networkmanager.h"
#include "usermanager.h"
#include "friendmanager.h"
#include "messagemanager.h"
#include "userinfodialog.h"
#include "searchresultdialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QMessageBox>
#include <QRegularExpression>
#include <QDebug>
#include <QDateTime>
#include <QIcon>
#include <QKeyEvent>
#include <QTimer>

/**
 * @brief MainWindow构造函数，用于初始化类内私有属性+初始化UI+显示基本窗口信息+连接信号槽
 * @param networkManager 网络管理器指针
 * @param userManager 用户管理器指针
 * @param friendManager 好友管理器指针
 * @param messageManager 消息管理器指针
 * @param parent 父窗口
 */
MainWindow::MainWindow(NetworkManager* networkManager, UserManager* userManager, FriendManager* friendManager, MessageManager* messageManager, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(networkManager)
    , userManager(userManager)
    , friendManager(friendManager)
    , messageManager(messageManager)
    , avatarLabel(nullptr)
    , usernameLabel(nullptr)
    , statusIndicator(nullptr)
    , searchLineEdit(nullptr)
    , friendListWidget(nullptr)
    , deleteFriendButton(nullptr)
    , refreshListButton(nullptr)
    , welcomeLabel(nullptr)
    , contentStack(nullptr)
    , chatPage(nullptr)
    , chatFriendLabel(nullptr)
    , messageListWidget(nullptr)
    , messageInput(nullptr)
    , sendButton(nullptr)
    , trayIcon(nullptr)
    , notificationBubble(nullptr)
    , isDeletingFriend(false)
    , isViewingFriendInfo(false)
    , isSendingMessage(false)
{
    ui->setupUi(this);  // 初始化主窗口（UI界面）

    setWindowTitle("LiteChat - " + userManager->GetCurrentUsername());  // 设置窗口标题
    ui->statusbar->showMessage("已登录: " + userManager->GetCurrentUsername());  // 状态栏显示登录用户名
    resize(800, 600);  // 设置窗口大小

    QWidget* centralWidget = new QWidget(this);  // 创建中央窗口部件
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);  // 创建垂直布局
    mainLayout->setContentsMargins(0, 0, 0, 0);  // 设置布局边距为0
    mainLayout->setSpacing(0);  // 设置布局间距为0

    SetupTopBar(mainLayout);  // 设置顶部栏

    QHBoxLayout* centralLayout = new QHBoxLayout();  // 创建中央区域水平布局
    centralLayout->setContentsMargins(0, 0, 0, 0);  // 设置布局边距为0
    centralLayout->setSpacing(0);  // 设置布局间距为0

    SetupLeftPanel(centralLayout);  // 设置左侧面板
    SetupContentArea(centralLayout);  // 设置内容区域

    mainLayout->addLayout(centralLayout);  // 添加中央区域布局

    SetupBottomBar(mainLayout);  // 设置底部操作栏

    setCentralWidget(centralWidget);  // 设置中央窗口部件

    // 连接断开后，自动触发自带的信号，自动调用槽函数
    connect(networkManager, &NetworkManager::Disconnected, this, &MainWindow::OnDisconnected);
    // 状态改变后，手动触发自定义信号，自动调用槽函数
    connect(userManager, &UserManager::StatusChanged, this, &MainWindow::OnStatusChanged);
    // 好友列表接收后，手动触发自定义信号，自动调用槽函数
    connect(friendManager, &FriendManager::FriendListReceived, this, &MainWindow::OnFriendListReceived);
    // 好友列表接收失败后，手动触发自定义信号，自动调用槽函数
    connect(friendManager, &FriendManager::FriendListFailed, this, &MainWindow::OnFriendListFailed);
    // 添加好友后，手动触发自定义信号，自动调用槽函数
    connect(friendManager, &FriendManager::FriendAdded, this, &MainWindow::OnFriendAdded);
    // 删除好友后，手动触发自定义信号，自动调用槽函数
    connect(friendManager, &FriendManager::FriendDeleted, this, &MainWindow::OnFriendDeleted);
    // 删除好友失败后，手动触发自定义信号，自动调用槽函数
    connect(friendManager, &FriendManager::FriendDeleteFailed, this, &MainWindow::OnFriendDeleteFailed);
    // 好友状态改变后，手动触发自定义信号，自动调用槽函数
    connect(friendManager, &FriendManager::FriendStatusChanged, this, &MainWindow::OnFriendStatusChanged);
    // 查询好友结果后，手动触发自定义信号，自动调用槽函数
    connect(friendManager, &FriendManager::QueryFriendResult, this, &MainWindow::OnQueryFriendResult);
    // 查询好友失败后，手动触发自定义信号，自动调用槽函数
    connect(friendManager, &FriendManager::QueryFriendFailed, this, &MainWindow::OnQueryFriendFailed);
    // 好友列表项被点击后，自动触发自带的信号，自动调用槽函数
    connect(friendListWidget, &QListWidget::itemClicked, this, &MainWindow::OnFriendListItemClicked);
    // 收到消息后，手动触发自定义信号，自动调用槽函数
    connect(messageManager, &MessageManager::MessageReceived, this, &MainWindow::OnMessageReceived);
    // 历史消息接收后，手动触发自定义信号，自动调用槽函数
    connect(messageManager, &MessageManager::HistoryReceived, this, &MainWindow::OnHistoryReceived);
    // 消息发送成功后，手动触发自定义信号，自动调用槽函数
    connect(messageManager, &MessageManager::MessageSendSuccess, this, &MainWindow::OnMessageSendSuccess);
    // 消息发送失败后，手动触发自定义信号，自动调用槽函数
    connect(messageManager, &MessageManager::MessageSendFailed, this, &MainWindow::OnMessageSendFailed);
    // 历史消息接收失败后，手动触发自定义信号，自动调用槽函数
    connect(messageManager, &MessageManager::HistoryFailed, this, &MainWindow::OnHistoryFailed);
    // 消息通知后，手动触发自定义信号，自动调用槽函数
    connect(messageManager, &MessageManager::MessageNotify, this, &MainWindow::OnMessageNotify);

    userManager->QueryUserStatus();  // 查询用户状态
    friendManager->GetFriendList();  // 获取好友列表

    // 初始化系统托盘图标
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/icons/app.png"));
    trayIcon->setToolTip("LiteChat");
    trayIcon->show();
}

/**
 * @brief MainWindow析构函数，用于释放动态分配的资源
 */
MainWindow::~MainWindow(){
    delete ui;  // 释放主窗口（UI界面）的指针
}

/**
 * @brief 设置顶部栏
 * @param mainLayout 主布局指针
 */
void MainWindow::SetupTopBar(QVBoxLayout* mainLayout){
    QWidget* topBar = new QWidget();  // 创建顶部栏
    topBar->setFixedHeight(50);  // 设置顶部栏固定高度50px
    topBar->setStyleSheet("background-color: #FFFFFF; border-bottom: 1px solid #E8E8E8;");  // 设置顶部栏样式

    QHBoxLayout* topLayout = new QHBoxLayout(topBar);  // 创建顶部栏水平布局
    topLayout->setContentsMargins(16, 7, 16, 7);  // 设置布局边距

    avatarLabel = new QLabel();  // 创建头像标签
    avatarLabel->setFixedSize(36, 36);  // 设置头像标签固定大小36×36px
    QString username = userManager->GetCurrentUsername();  // 获取当前用户名
    QString avatarText = username.isEmpty() ? "U" : username.left(1).toUpper();  // 获取头像文字
    avatarLabel->setText(avatarText);  // 设置头像标签文本
    avatarLabel->setAlignment(Qt::AlignCenter);  // 设置头像标签居中
    avatarLabel->setStyleSheet(
        "QLabel {"
        "   background-color: #12B7F5;"
        "   border-radius: 18px;"
        "   color: #FFFFFF;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "}"
        "QLabel:hover {"
        "   background-color: #0EA5D9;"
        "}"
    );  // 设置头像标签样式（圆形蓝色背景）
    avatarLabel->installEventFilter(this);  // 安装事件过滤器

    usernameLabel = new QLabel(username);  // 创建用户名标签
    usernameLabel->setStyleSheet("font-size: 14px; color: #333333; border: none;");  // 设置用户名标签样式

    statusIndicator = new QLabel();  // 创建在线状态指示灯
    statusIndicator->setFixedSize(12, 12);  // 设置状态指示灯固定大小12×12px
    statusIndicator->setStyleSheet(
        "background-color: #52C41A; border-radius: 6px; border: none;"
    );  // 设置状态指示灯样式（绿色圆形）

    searchLineEdit = new QLineEdit();  // 创建搜索框
    searchLineEdit->setPlaceholderText("搜索好友");  // 设置搜索框占位符文本
    searchLineEdit->setFixedSize(200, 32);  // 设置搜索框固定大小200×32px
    QAction* searchAction = searchLineEdit->addAction(QIcon(":/icons/search.png"), QLineEdit::LeadingPosition);  // 添加放大镜图标
    searchLineEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 16px;"
        "   padding: 4px 12px;"
        "   font-size: 12px;"
        "   background-color: #F5F5F5;"
        "}"
        "QLineEdit:focus {"
        "   border: 1px solid #12B7F5;"
        "   background-color: #FFFFFF;"
        "}"
    );  // 设置搜索框样式
    // 回车键被按下后，自动触发自带的信号，自动调用槽函数
    connect(searchLineEdit, &QLineEdit::returnPressed, this, &MainWindow::OnSearchReturnPressed);

    topLayout->addWidget(avatarLabel);  // 添加头像标签
    topLayout->addWidget(usernameLabel);  // 添加用户名标签
    topLayout->addWidget(statusIndicator);  // 添加在线状态指示灯
    topLayout->addStretch();  // 添加弹性空间
    topLayout->addWidget(searchLineEdit);  // 添加搜索框

    mainLayout->addWidget(topBar);  // 添加顶部栏
}

/**
 * @brief 设置左侧面板（好友列表区域）
 * @param centralLayout 中央区域布局指针
 */
void MainWindow::SetupLeftPanel(QHBoxLayout* centralLayout){
    QWidget* leftPanel = new QWidget();  // 创建左侧面板
    leftPanel->setFixedWidth(250);  // 设置左侧面板固定宽度250px
    leftPanel->setStyleSheet("background-color: #FAFAFA; border-right: 1px solid #E8E8E8;");  // 设置左侧面板样式

    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);  // 创建左侧面板垂直布局
    leftLayout->setContentsMargins(0, 0, 0, 0);  // 设置布局边距为0
    leftLayout->setSpacing(0);  // 设置布局间距为0

    friendListWidget = new QListWidget();  // 创建好友列表控件
    friendListWidget->setStyleSheet(
        "QListWidget {"
        "   border: none;"
        "   background-color: #FAFAFA;"
        "   outline: none;"
        "}"
        "QListWidget::item {"
        "   border-bottom: 1px solid #F0F0F0;"
        "   padding: 8px 12px;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: #E6F7FF;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #BAE7FF;"
        "   color: #333333;"
        "}"
    );  // 设置好友列表样式
    friendListWidget->setContextMenuPolicy(Qt::CustomContextMenu);  // 设置右键菜单策略
    // 右键菜单请求后，自动触发自带的信号，自动调用槽函数
    connect(friendListWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::OnFriendListContextMenu);

    leftLayout->addWidget(friendListWidget);  // 添加好友列表控件

    centralLayout->addWidget(leftPanel);  // 添加左侧面板
}

/**
 * @brief 设置内容区域（右侧面板，包含欢迎页和聊天页）
 * @param centralLayout 中央区域布局指针
 */
void MainWindow::SetupContentArea(QHBoxLayout* centralLayout){
    contentStack = new QStackedWidget();  // 创建内容区域堆栈控件

    // 欢迎页
    QWidget* welcomePage = new QWidget();  // 创建欢迎页控件
    QVBoxLayout* welcomeLayout = new QVBoxLayout(welcomePage);  // 创建欢迎页垂直布局
    welcomeLayout->setAlignment(Qt::AlignCenter);  // 设置欢迎页布局居中
    welcomeLabel = new QLabel("欢迎使用 LiteChat", welcomePage);  // 创建欢迎标签
    welcomeLabel->setAlignment(Qt::AlignCenter);  // 设置欢迎标签居中
    welcomeLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #333333;");  // 设置欢迎标签样式
    welcomeLayout->addWidget(welcomeLabel);  // 添加欢迎标签
    contentStack->addWidget(welcomePage);  // 添加欢迎页

    // 聊天页
    SetupChatPage();  // 设置聊天页
    contentStack->addWidget(chatPage);  // 添加聊天页

    contentStack->setCurrentIndex(0);  // 默认显示欢迎页

    centralLayout->addWidget(contentStack);  // 添加内容区域
}

/**
 * @brief 设置聊天页
 */
void MainWindow::SetupChatPage(){
    chatPage = new QWidget();  // 创建聊天页控件
    chatPage->setStyleSheet("background-color: #F5F5F5;");  // 设置聊天页背景样式

    QVBoxLayout* chatLayout = new QVBoxLayout(chatPage);  // 创建聊天页垂直布局
    chatLayout->setContentsMargins(0, 0, 0, 0);  // 设置布局边距为0
    chatLayout->setSpacing(0);  // 设置布局间距为0

    // 聊天顶部栏（显示好友名称）
    QWidget* chatHeader = new QWidget();  // 创建聊天顶部栏
    chatHeader->setFixedHeight(44);  // 设置聊天顶部栏固定高度44px
    chatHeader->setStyleSheet("background-color: #FFFFFF; border-bottom: 1px solid #E8E8E8;");  // 设置聊天顶部栏样式

    QHBoxLayout* chatHeaderLayout = new QHBoxLayout(chatHeader);  // 创建聊天顶部栏水平布局
    chatHeaderLayout->setContentsMargins(16, 10, 16, 10);  // 设置布局边距

    chatFriendLabel = new QLabel();  // 创建聊天好友名称标签
    chatFriendLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333; border: none;");  // 设置聊天好友名称标签样式
    chatHeaderLayout->addWidget(chatFriendLabel);  // 添加聊天好友名称标签
    chatHeaderLayout->addStretch();  // 添加弹性空间

    chatLayout->addWidget(chatHeader);  // 添加聊天顶部栏

    // 消息列表
    messageListWidget = new QListWidget();  // 创建消息列表控件
    messageListWidget->setStyleSheet(
        "QListWidget {"
        "   border: none;"
        "   background-color: #F5F5F5;"
        "   outline: none;"
        "}"
        "QListWidget::item {"
        "   border: none;"
        "   padding: 4px 0px;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: transparent;"
        "}"
    );  // 设置消息列表样式
    messageListWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);  // 设置滚动模式
    messageListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  // 隐藏水平滚动条
    messageListWidget->setSelectionMode(QAbstractItemView::NoSelection);  // 禁用选中

    chatLayout->addWidget(messageListWidget);  // 添加消息列表

    // 输入区域
    QWidget* inputArea = new QWidget();  // 创建输入区域控件
    inputArea->setFixedHeight(52);  // 设置输入区域固定高度52px
    inputArea->setStyleSheet("background-color: #FFFFFF; border-top: 1px solid #E8E8E8;");  // 设置输入区域样式

    QHBoxLayout* inputLayout = new QHBoxLayout(inputArea);  // 创建输入区域水平布局
    inputLayout->setContentsMargins(12, 10, 12, 10);  // 设置布局边距

    messageInput = new QPlainTextEdit();  // 创建消息输入框（支持多行）
    messageInput->setPlaceholderText("输入消息...");  // 设置消息输入框占位符文本
    messageInput->setMaximumHeight(100);  // 设置最大高度（约4行）
    messageInput->setStyleSheet(
        "QPlainTextEdit {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 6px 10px;"
        "   font-size: 13px;"
        "   background-color: #FFFFFF;"
        "}"
        "QPlainTextEdit:focus {"
        "   border: 1px solid #12B7F5;"
        "}"
    );  // 设置消息输入框样式
    messageInput->installEventFilter(this);  // 安装事件过滤器（处理Enter发送/Shift+Enter换行）

    sendButton = new QPushButton("发送");  // 创建发送按钮
    sendButton->setFixedSize(60, 32);  // 设置发送按钮固定大小60×32px
    sendButton->setStyleSheet(
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
        "QPushButton:pressed {"
        "   background-color: #0B98C2;"
        "}"
    );  // 设置发送按钮样式
    // 发送按钮被点击后，自动触发自带的信号，自动调用槽函数
    connect(sendButton, &QPushButton::clicked, this, &MainWindow::OnSendMessageClicked);

    inputLayout->addWidget(messageInput);  // 添加消息输入框
    inputLayout->addWidget(sendButton);  // 添加发送按钮

    chatLayout->addWidget(inputArea);  // 添加输入区域
}

/**
 * @brief 设置底部操作栏
 * @param mainLayout 主布局指针
 */
void MainWindow::SetupBottomBar(QVBoxLayout* mainLayout){
    QWidget* bottomBar = new QWidget();  // 创建底部操作栏
    bottomBar->setFixedHeight(40);  // 设置底部操作栏固定高度40px
    bottomBar->setStyleSheet("background-color: #FFFFFF; border-top: 1px solid #E8E8E8;");  // 设置底部操作栏样式

    QHBoxLayout* bottomLayout = new QHBoxLayout(bottomBar);  // 创建底部操作栏水平布局
    bottomLayout->setContentsMargins(8, 4, 8, 4);  // 设置布局边距

    deleteFriendButton = new QPushButton("删除好友");  // 创建删除好友按钮
    deleteFriendButton->setStyleSheet(
        "QPushButton {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 4px 8px;"
        "   font-size: 12px;"
        "   color: #333333;"
        "   background-color: #FFFFFF;"
        "}"
        "QPushButton:hover {"
        "   border: 1px solid #FF4D4F;"
        "   color: #FF4D4F;"
        "}"
    );  // 设置删除好友按钮样式
    // 删除好友按钮被点击后，自动触发自带的信号，自动调用槽函数
    connect(deleteFriendButton, &QPushButton::clicked, this, &MainWindow::OnDeleteFriendClicked);

    refreshListButton = new QPushButton("刷新列表");  // 创建刷新列表按钮
    refreshListButton->setStyleSheet(
        "QPushButton {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 4px 8px;"
        "   font-size: 12px;"
        "   color: #333333;"
        "   background-color: #FFFFFF;"
        "}"
        "QPushButton:hover {"
        "   border: 1px solid #12B7F5;"
        "   color: #12B7F5;"
        "}"
    );  // 设置刷新列表按钮样式
    // 刷新列表按钮被点击后，自动触发自带的信号，自动调用槽函数
    connect(refreshListButton, &QPushButton::clicked, this, &MainWindow::OnRefreshListClicked);

    bottomLayout->addStretch();  // 添加弹性空间
    bottomLayout->addWidget(deleteFriendButton);  // 添加删除好友按钮
    bottomLayout->addWidget(refreshListButton);  // 添加刷新列表按钮

    mainLayout->addWidget(bottomBar);  // 添加底部操作栏
}

/**
 * @brief 清空好友列表控件
 */
void MainWindow::ClearFriendList(){
    friendListWidget->clear();  // 清空好友列表
}

/**
 * @brief 更新好友列表中的状态指示器
 * @param username 好友用户名
 * @param status 好友状态
 */
void MainWindow::UpdateFriendListStatus(const QString& username, UserStatus status){
    for(int i = 0; i < friendListWidget->count(); ++i){
        QListWidgetItem* item = friendListWidget->item(i);
        QWidget* widget = friendListWidget->itemWidget(item);
        if(!widget) continue;
        QLabel* indicator = widget->findChild<QLabel*>("statusIndicator");
        if(!indicator) continue;
        QLabel* nameLabel = widget->findChild<QLabel*>("usernameLabel");
        if(!nameLabel) continue;
        if(nameLabel->text() == username){
            if(status == UserStatus::Online){
                indicator->setStyleSheet("background-color: #52C41A; border-radius: 6px; border: none; min-width: 12px;");
            }
            else{
                indicator->setStyleSheet("background-color: #BFBFBF; border-radius: 6px; border: none; min-width: 12px;");
            }
        }
    }
}

/**
 * @brief 连接断开后，自动触发自带的信号，自动调用槽函数
 */
void MainWindow::OnDisconnected(){
    statusIndicator->setStyleSheet(
        "background-color: #BFBFBF; border-radius: 6px; border: none;"
    );  // 设置状态指示灯样式（灰色圆形，离线）
    ui->statusbar->showMessage("连接已断开");  // 状态栏显示连接断开
    qDebug() << "[MainWindow::OnDisconnected]连接断开后自动调用槽函数";  // Debug输出
}

/**
 * @brief 事件过滤器，用于处理头像点击和消息输入框按键事件
 * @param obj 事件对象
 * @param event 事件
 * @return 是否处理事件
 */
bool MainWindow::eventFilter(QObject* obj, QEvent* event){
    if(obj == avatarLabel && event->type() == QEvent::MouseButtonPress){
        OnAvatarClicked();
        return true;
    }
    if(obj == messageInput && event->type() == QEvent::KeyPress){
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if(keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter){
            if(keyEvent->modifiers() & Qt::ShiftModifier){
                return false;  // Shift+Enter：允许换行（默认行为）
            }
            else{
                OnSendMessageClicked();  // Enter：发送消息
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

/**
 * @brief 头像被点击后，打开个人信息设置对话框
 */
void MainWindow::OnAvatarClicked(){
    UserInfoDialog userInfoDlg(userManager, this);  // 创建个人信息对话框
    userInfoDlg.exec();  // 显示个人信息对话框
    qDebug() << "[MainWindow::OnAvatarClicked]头像被点击后自动调用槽函数";  // Debug输出
}

/**
 * @brief 状态改变后，手动触发自定义信号，自动调用槽函数
 * @param status 用户状态
 */
void MainWindow::OnStatusChanged(UserStatus status){
    if(status == UserStatus::Online){
        statusIndicator->setStyleSheet(
            "background-color: #52C41A; border-radius: 6px; border: none;"
        );  // 设置状态指示灯样式（绿色圆形，在线）
        ui->statusbar->showMessage("在线");  // 状态栏显示在线
    }
    else{
        statusIndicator->setStyleSheet(
            "background-color: #BFBFBF; border-radius: 6px; border: none;"
        );  // 设置状态指示灯样式（灰色圆形，离线）
        ui->statusbar->showMessage("离线");  // 状态栏显示离线
    }
    qDebug() << "[MainWindow::OnStatusChanged]状态改变后自动调用槽函数";  // Debug输出
}

/**
 * @brief 搜索框回车键被按下后，自动触发自带的信号，自动调用槽函数
 */
void MainWindow::OnSearchReturnPressed(){
    QString searchText = searchLineEdit->text().trimmed();  // 获取搜索框文本并去掉首尾空格
    if(searchText.isEmpty()){
        QMessageBox::warning(this, "错误", "用户名不能为空");
        return;
    }
    QRegularExpression regex("^[a-zA-Z0-9_]{3,20}$");  // 用户名格式校验正则表达式
    if(!regex.match(searchText).hasMatch()){
        QMessageBox::warning(this, "错误", "用户名不符合格式");
        return;
    }
    friendManager->QueryFriendInfo(searchText);  // 发送查询好友请求
    isViewingFriendInfo = false;  // 设置搜索模式
    qDebug() << "[MainWindow::OnSearchReturnPressed]搜索框回车键被按下后自动调用槽函数";  // Debug输出
}

/**
 * @brief 好友列表接收后，手动触发自定义信号，自动调用槽函数
 * @param friendList 好友列表
 */
void MainWindow::OnFriendListReceived(const QList<FriendInfo>& friendList){
    ClearFriendList();  // 清空好友列表
    if(friendList.isEmpty()){
        QListWidgetItem* emptyItem = new QListWidgetItem();  // 创建空列表提示项
        emptyItem->setSizeHint(QSize(0, 60));  // 设置列表项大小
        emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsSelectable);  // 禁用选中
        QLabel* emptyLabel = new QLabel("暂无好友，快去添加好友吧");  // 创建空列表提示标签
        emptyLabel->setAlignment(Qt::AlignCenter);  // 设置标签居中
        emptyLabel->setStyleSheet("font-size: 12px; color: #BFBFBF; border: none;");  // 设置标签样式
        friendListWidget->addItem(emptyItem);  // 添加空列表提示项
        friendListWidget->setItemWidget(emptyItem, emptyLabel);  // 设置列表项控件
        return;
    }
    for(const FriendInfo& info : friendList){
        QWidget* itemWidget = new QWidget();  // 创建好友项控件
        QHBoxLayout* itemLayout = new QHBoxLayout(itemWidget);  // 创建好友项水平布局
        itemLayout->setContentsMargins(12, 8, 12, 8);  // 设置布局边距

        QLabel* indicator = new QLabel(itemWidget);  // 创建状态指示器
        indicator->setObjectName("statusIndicator");  // 设置对象名
        indicator->setFixedSize(12, 12);  // 设置状态指示器固定大小12×12px
        if(info.status == UserStatus::Online){
            indicator->setStyleSheet("background-color: #52C41A; border-radius: 6px; border: none; min-width: 12px;");
        }
        else{
            indicator->setStyleSheet("background-color: #BFBFBF; border-radius: 6px; border: none; min-width: 12px;");
        }

        QLabel* nameLabel = new QLabel(info.username, itemWidget);  // 创建用户名标签
        nameLabel->setObjectName("usernameLabel");  // 设置对象名
        nameLabel->setStyleSheet("font-size: 13px; color: #333333; border: none;");  // 设置用户名标签样式

        itemLayout->addWidget(indicator);  // 添加状态指示器
        itemLayout->addWidget(nameLabel);  // 添加用户名标签
        itemLayout->addStretch();  // 添加弹性空间

        QListWidgetItem* item = new QListWidgetItem();  // 创建列表项
        item->setSizeHint(QSize(0, 60));  // 设置列表项大小
        item->setData(Qt::UserRole, info.username);  // 设置列表项数据（用户名）
        friendListWidget->addItem(item);  // 添加列表项
        friendListWidget->setItemWidget(item, itemWidget);  // 设置列表项控件
    }
    qDebug() << "[MainWindow::OnFriendListReceived]好友列表接收后自动调用槽函数";  // Debug输出
}

/**
 * @brief 好友列表接收失败后，手动触发自定义信号，自动调用槽函数
 * @param errorMsg 错误信息
 */
void MainWindow::OnFriendListFailed(const QString& errorMsg){
    QMessageBox::warning(this, "错误", "获取好友列表失败：" + errorMsg);
    qDebug() << "[MainWindow::OnFriendListFailed]好友列表接收失败后自动调用槽函数" << errorMsg;  // Debug输出
}

/**
 * @brief 添加好友后，手动触发自定义信号，自动调用槽函数
 * @param username 添加成功的用户名
 */
void MainWindow::OnFriendAdded(const QString& username){
    friendManager->GetFriendList();  // 刷新好友列表
    qDebug() << "[MainWindow::OnFriendAdded]添加好友后自动调用槽函数" << username;  // Debug输出
}

/**
 * @brief 删除好友后，手动触发自定义信号，自动调用槽函数
 * @param username 删除成功的用户名
 */
void MainWindow::OnFriendDeleted(const QString& username){
    isDeletingFriend = false;  // 重置删除好友处理状态
    if(currentChatFriend == username){
        currentChatFriend.clear();  // 清空当前聊天好友
        contentStack->setCurrentIndex(0);  // 切回欢迎页
    }
    friendManager->GetFriendList();  // 刷新好友列表
    QMessageBox::information(this, "提示", "已删除好友：" + username);
    qDebug() << "[MainWindow::OnFriendDeleted]删除好友后自动调用槽函数" << username;  // Debug输出
}

/**
 * @brief 删除好友失败后，手动触发自定义信号，自动调用槽函数
 * @param errorMsg 错误信息
 */
void MainWindow::OnFriendDeleteFailed(const QString& errorMsg){
    isDeletingFriend = false;  // 重置删除好友处理状态
    QMessageBox::warning(this, "错误", "删除好友失败：" + errorMsg);
    qDebug() << "[MainWindow::OnFriendDeleteFailed]删除好友失败后自动调用槽函数" << errorMsg;  // Debug输出
}

/**
 * @brief 好友状态改变后，手动触发自定义信号，自动调用槽函数
 * @param username 好友用户名
 * @param status 好友状态
 */
void MainWindow::OnFriendStatusChanged(const QString& username, UserStatus status){
    UpdateFriendListStatus(username, status);  // 更新好友列表状态指示器
    qDebug() << "[MainWindow::OnFriendStatusChanged]好友状态改变后自动调用槽函数" << username;  // Debug输出
}

/**
 * @brief 查询好友结果后，手动触发自定义信号，自动调用槽函数
 * @param info 好友信息
 */
void MainWindow::OnQueryFriendResult(const FriendInfo& info){
    SearchResultDialog dlg(friendManager, this);  // 创建搜索结果对话框
    dlg.SetResultInfo(info.username, info.status);  // 设置搜索结果信息
    if(isViewingFriendInfo){
        dlg.SetViewOnlyMode(true);  // 设置为仅查看模式
    }
    dlg.exec();  // 显示搜索结果对话框
    qDebug() << "[MainWindow::OnQueryFriendResult]查询好友结果后自动调用槽函数" << info.username;  // Debug输出
}

/**
 * @brief 查询好友失败后，手动触发自定义信号，自动调用槽函数
 * @param errorMsg 错误信息
 */
void MainWindow::OnQueryFriendFailed(const QString& errorMsg){
    QMessageBox::warning(this, "错误", "查询好友失败：" + errorMsg);
    qDebug() << "[MainWindow::OnQueryFriendFailed]查询好友失败后自动调用槽函数" << errorMsg;  // Debug输出
}

/**
 * @brief 删除好友按钮被点击后，自动触发自带的信号，自动调用槽函数
 */
void MainWindow::OnDeleteFriendClicked(){
    if(isDeletingFriend){
        return;  // 正在处理删除好友请求，忽略重复点击
    }
    isDeletingFriend = true;  // 设置删除好友处理状态
    if(currentChatFriend.isEmpty()){
        QMessageBox::warning(this, "错误", "请先在好友列表中点击一个好友");
        isDeletingFriend = false;  // 重置删除好友处理状态
        return;
    }
    friendManager->DeleteFriend(currentChatFriend);  // 发送删除好友请求
    qDebug() << "[MainWindow::OnDeleteFriendClicked]删除好友按钮被点击后自动调用槽函数";  // Debug输出
}

/**
 * @brief 刷新列表按钮被点击后，自动触发自带的信号，自动调用槽函数
 */
void MainWindow::OnRefreshListClicked(){
    friendManager->GetFriendList();  // 发送获取好友列表请求
    qDebug() << "[MainWindow::OnRefreshListClicked]刷新列表按钮被点击后自动调用槽函数";  // Debug输出
}

/**
 * @brief 好友列表右键菜单请求后，自动触发自带的信号，自动调用槽函数
 * @param pos 右键点击位置
 */
void MainWindow::OnFriendListContextMenu(const QPoint& pos){
    QListWidgetItem* item = friendListWidget->itemAt(pos);  // 获取右键点击的列表项
    if(!item) return;  // 如果没有选中项则返回
    QString username = item->data(Qt::UserRole).toString();  // 获取用户名
    if(username.isEmpty()) return;  // 如果用户名为空则返回

    currentChatFriend = username;  // 设置当前聊天好友

    QMenu menu(this);  // 创建右键菜单
    QAction* viewAction = menu.addAction("查看资料");  // 添加查看资料菜单项
    QAction* deleteAction = menu.addAction("删除好友");  // 添加删除好友菜单项

    QAction* selectedAction = menu.exec(friendListWidget->mapToGlobal(pos));  // 显示右键菜单
    if(selectedAction == viewAction){
        isViewingFriendInfo = true;  // 设置查看好友资料标志
        friendManager->QueryFriendInfo(username);  // 发送查询好友请求
    }
    else if(selectedAction == deleteAction){
        isDeletingFriend = true;  // 设置删除好友处理状态
        friendManager->DeleteFriend(username);  // 发送删除好友请求
    }
    qDebug() << "[MainWindow::OnFriendListContextMenu]好友列表右键菜单请求后自动调用槽函数";  // Debug输出
}

/**
 * @brief 好友列表项被点击后，自动触发自带的信号，自动调用槽函数
 * @param item 被点击的列表项
 */
void MainWindow::OnFriendListItemClicked(QListWidgetItem* item){
    QString username = item->data(Qt::UserRole).toString();  // 获取用户名
    if(username.isEmpty()) return;  // 如果用户名为空则返回
    SwitchToChatView(username);  // 切换到聊天视图
    qDebug() << "[MainWindow::OnFriendListItemClicked]好友列表项被点击后自动调用槽函数" << username;  // Debug输出
}

/**
 * @brief 发送按钮被点击后，自动触发自带的信号，自动调用槽函数
 */
void MainWindow::OnSendMessageClicked(){
    QString content = messageInput->toPlainText().trimmed();  // 获取消息输入框文本并去掉首尾空格
    if(content.isEmpty()){
        QMessageBox::warning(this, "错误", "消息内容不能为空");
        return;
    }
    if(currentChatFriend.isEmpty()){
        QMessageBox::warning(this, "错误", "请先选择一位好友");
        return;
    }
    if(isSendingMessage){
        return;  // 正在发送消息，忽略重复点击
    }
    isSendingMessage = true;  // 设置发送消息处理状态
    sendButton->setEnabled(false);  // 禁用发送按钮
    messageInput->clear();  // 清空消息输入框
    messageManager->SendMessage(currentChatFriend, content);  // 发送消息
    qDebug() << "[MainWindow::OnSendMessageClicked]发送按钮被点击后自动调用槽函数";  // Debug输出
}

/**
 * @brief 消息输入框回车键被按下后，自动触发自带的信号，自动调用槽函数
 */
void MainWindow::OnMessageInputReturnPressed(){
    OnSendMessageClicked();  // 调用发送消息方法
}

/**
 * @brief 收到消息后，手动触发自定义信号，自动调用槽函数
 * @param msg 消息对象
 */
void MainWindow::OnMessageReceived(const Message& msg){
    QString friendUsername = (msg.sender == userManager->GetCurrentUsername()) ? msg.receiver : msg.sender;
    if(friendUsername == currentChatFriend){
        DisplayMessage(msg);  // 如果当前正在与该好友聊天，则显示消息
    }
    qDebug() << "[MainWindow::OnMessageReceived]收到消息后自动调用槽函数" << msg.sender << ":" << msg.content;  // Debug输出
}

/**
 * @brief 历史消息接收后，手动触发自定义信号，自动调用槽函数
 * @param friendUsername 好友用户名
 * @param messages 历史消息列表
 */
void MainWindow::OnHistoryReceived(const QString& friendUsername, const QList<Message>& messages){
    if(friendUsername != currentChatFriend) return;  // 不是当前聊天好友，忽略
    ClearMessageList();  // 清空消息列表
    DisplayMessages(messages);  // 显示历史消息
    qDebug() << "[MainWindow::OnHistoryReceived]历史消息接收后自动调用槽函数" << friendUsername << messages.size() << "条";  // Debug输出
}

/**
 * @brief 消息发送成功后，手动触发自定义信号，自动调用槽函数
 * @param msg 消息对象
 */
void MainWindow::OnMessageSendSuccess(const Message& msg){
    isSendingMessage = false;  // 重置发送消息处理状态
    sendButton->setEnabled(true);  // 启用发送按钮
    if(msg.receiver == currentChatFriend){
        DisplayMessage(msg);  // 如果当前正在与该好友聊天，则显示消息
    }
    qDebug() << "[MainWindow::OnMessageSendSuccess]消息发送成功后自动调用槽函数";  // Debug输出
}

/**
 * @brief 消息发送失败后，手动触发自定义信号，自动调用槽函数
 * @param errorMsg 错误信息
 */
void MainWindow::OnMessageSendFailed(const QString& errorMsg){
    isSendingMessage = false;  // 重置发送消息处理状态
    sendButton->setEnabled(true);  // 启用发送按钮
    QMessageBox::warning(this, "错误", "消息发送失败：" + errorMsg);
    qDebug() << "[MainWindow::OnMessageSendFailed]消息发送失败后自动调用槽函数" << errorMsg;  // Debug输出
}

/**
 * @brief 历史消息接收失败后，手动触发自定义信号，自动调用槽函数
 * @param errorMsg 错误信息
 */
void MainWindow::OnHistoryFailed(const QString& errorMsg){
    QMessageBox::warning(this, "错误", "获取历史消息失败：" + errorMsg);
    qDebug() << "[MainWindow::OnHistoryFailed]历史消息接收失败后自动调用槽函数" << errorMsg;  // Debug输出
}

/**
 * @brief 切换到聊天视图
 * @param friendUsername 好友用户名
 */
void MainWindow::SwitchToChatView(const QString& friendUsername){
    if(friendUsername.isEmpty()) return;  // 如果好友用户名为空则返回
    currentChatFriend = friendUsername;  // 设置当前聊天好友
    chatFriendLabel->setText(friendUsername);  // 设置聊天好友名称标签
    ClearMessageList();  // 清空消息列表
    contentStack->setCurrentIndex(1);  // 切换到聊天页
    messageManager->GetHistoryMessages(friendUsername);  // 获取历史消息
    qDebug() << "[MainWindow::SwitchToChatView]切换到聊天视图" << friendUsername;  // Debug输出
}

/**
 * @brief 显示单条消息
 * @param msg 消息对象
 */
void MainWindow::DisplayMessage(const Message& msg){
    bool isMine = (msg.sender == userManager->GetCurrentUsername());  // 判断是否为自己发送的消息
    QString displayName = isMine ? "我" : msg.sender;  // 获取显示名称
    QString timeStr = FormatTimestamp(msg.timestamp);  // 格式化时间戳

    QWidget* messageWidget = new QWidget();  // 创建消息控件
    QHBoxLayout* mainLayout = new QHBoxLayout(messageWidget);  // 创建消息水平布局
    mainLayout->setContentsMargins(16, 4, 16, 4);  // 设置布局边距

    QWidget* bubbleWidget = new QWidget();  // 创建气泡控件
    QVBoxLayout* bubbleLayout = new QVBoxLayout(bubbleWidget);  // 创建气泡垂直布局
    bubbleLayout->setContentsMargins(0, 0, 0, 0);  // 设置布局边距为0
    bubbleLayout->setSpacing(2);  // 设置布局间距

    // 陌生人标签
    if(msg.isStranger && !isMine){
        QLabel* strangerLabel = new QLabel("[陌生人]", bubbleWidget);  // 创建陌生人标签
        strangerLabel->setStyleSheet(
            "font-size: 10px; color: #FA8C16; border: none; background: transparent;"
        );  // 设置陌生人标签样式（橙色）
        strangerLabel->setAlignment(Qt::AlignLeft);  // 左对齐
        bubbleLayout->addWidget(strangerLabel);  // 添加陌生人标签
    }

    // 发送者名称标签
    QLabel* senderLabel = new QLabel(displayName, bubbleWidget);  // 创建发送者名称标签
    senderLabel->setStyleSheet(
        "font-size: 11px; color: #999999; border: none; background: transparent;"
    );  // 设置发送者名称标签样式
    if(isMine){
        senderLabel->setAlignment(Qt::AlignRight);  // 自己的消息右对齐
    }
    bubbleLayout->addWidget(senderLabel);  // 添加发送者名称标签

    // 消息内容标签
    QLabel* contentLabel = new QLabel(msg.content, bubbleWidget);  // 创建消息内容标签
    contentLabel->setWordWrap(true);  // 设置自动换行
    contentLabel->setMaximumWidth(300);  // 设置最大宽度300px
    contentLabel->setStyleSheet(
        isMine ?
        "QLabel {"
        "   background-color: #12B7F5;"
        "   color: #FFFFFF;"
        "   border-radius: 8px;"
        "   padding: 8px 12px;"
        "   font-size: 13px;"
        "}" :
        "QLabel {"
        "   background-color: #FFFFFF;"
        "   color: #333333;"
        "   border-radius: 8px;"
        "   padding: 8px 12px;"
        "   font-size: 13px;"
        "   border: 1px solid #E8E8E8;"
        "}"
    );  // 设置消息内容标签样式
    bubbleLayout->addWidget(contentLabel);  // 添加消息内容标签

    // 时间标签
    QLabel* timeLabel = new QLabel(timeStr, bubbleWidget);  // 创建时间标签
    timeLabel->setStyleSheet(
        "font-size: 10px; color: #BFBFBF; border: none; background: transparent;"
    );  // 设置时间标签样式
    if(isMine){
        timeLabel->setAlignment(Qt::AlignRight);  // 自己的消息右对齐
    }
    bubbleLayout->addWidget(timeLabel);  // 添加时间标签

    if(isMine){
        mainLayout->addStretch();  // 添加弹性空间
        mainLayout->addWidget(bubbleWidget);  // 添加气泡控件
    }
    else{
        mainLayout->addWidget(bubbleWidget);  // 添加气泡控件
        mainLayout->addStretch();  // 添加弹性空间
    }

    QListWidgetItem* item = new QListWidgetItem();  // 创建列表项
    item->setSizeHint(messageWidget->sizeHint());  // 设置列表项大小
    messageListWidget->addItem(item);  // 添加列表项
    messageListWidget->setItemWidget(item, messageWidget);  // 设置列表项控件
    ScrollMessageListToBottom();  // 滚动到底部
}

/**
 * @brief 显示多条消息
 * @param messages 消息列表
 */
void MainWindow::DisplayMessages(const QList<Message>& messages){
    for(const Message& msg : messages){
        DisplayMessage(msg);  // 显示每条消息
    }
}

/**
 * @brief 清空消息列表
 */
void MainWindow::ClearMessageList(){
    messageListWidget->clear();  // 清空消息列表
}

/**
 * @brief 滚动消息列表到底部
 */
void MainWindow::ScrollMessageListToBottom(){
    messageListWidget->scrollToBottom();  // 滚动到底部
}

/**
 * @brief 格式化时间戳，提取时间部分（HH:mm）
 * @param timestamp 原始时间戳
 * @return 格式化后的时间字符串
 */
QString MainWindow::FormatTimestamp(const QString& timestamp) const{
    if(timestamp.isEmpty()){
        return QDateTime::currentDateTime().toString("HH:mm");  // 返回当前时间
    }
    QString timeStr = timestamp;  // 复制时间戳
    if(timeStr.length() >= 16){
        timeStr = timeStr.mid(11, 5);  // 提取HH:mm部分
    }
    return timeStr;
}

/**
 * @brief 消息通知后，手动触发自定义信号，自动调用槽函数
 * @param sender 发送者用户名
 * @param count 未读消息数量
 */
void MainWindow::OnMessageNotify(const QString& sender, int count){
    QString summary = QString("您有 %1 条来自 %2 的新消息").arg(count).arg(sender);
    if(isActiveWindow()){
        ShowNotificationBubble(sender, summary);  // 主窗口活跃时显示通知气泡
    }
    else{
        trayIcon->showMessage("新消息", summary, QSystemTrayIcon::Information, 5000);  // 主窗口不活跃时显示系统托盘通知
    }
    qDebug() << "[MainWindow::OnMessageNotify]消息通知后自动调用槽函数" << sender << count;  // Debug输出
}

/**
 * @brief 显示通知气泡（5秒后自动消失）
 * @param sender 发送者用户名
 * @param summary 消息摘要
 */
void MainWindow::ShowNotificationBubble(const QString& sender, const QString& summary){
    if(notificationBubble){
        notificationBubble->deleteLater();  // 如果已有通知气泡则先删除
    }
    notificationBubble = new QLabel(this);  // 创建通知气泡标签
    notificationBubble->setText(summary);  // 设置通知文本
    notificationBubble->setStyleSheet(
        "QLabel {"
        "   background-color: #12B7F5;"
        "   color: #FFFFFF;"
        "   border-radius: 8px;"
        "   padding: 10px 16px;"
        "   font-size: 13px;"
        "}"
    );  // 设置通知气泡样式
    notificationBubble->setAlignment(Qt::AlignCenter);  // 设置文本居中
    notificationBubble->setFixedHeight(40);  // 设置固定高度
    notificationBubble->adjustSize();  // 调整大小
    notificationBubble->move((width() - notificationBubble->width()) / 2, 60);  // 居中显示在顶部
    notificationBubble->raise();  // 置于顶层
    notificationBubble->show();  // 显示通知气泡
    QTimer::singleShot(5000, this, [this](){
        if(notificationBubble){
            notificationBubble->deleteLater();
            notificationBubble = nullptr;
        }
    });  // 5秒后自动删除通知气泡
}