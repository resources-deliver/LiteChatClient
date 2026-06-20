#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "networkmanager.h"
#include "usermanager.h"
#include "friendmanager.h"
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

/**
 * @brief MainWindow构造函数，用于初始化类内私有属性+初始化UI+显示基本窗口信息+连接信号槽
 * @param networkManager 网络管理器指针
 * @param userManager 用户管理器指针
 * @param friendManager 好友管理器指针
 * @param parent 父窗口
 */
MainWindow::MainWindow(NetworkManager* networkManager, UserManager* userManager, FriendManager* friendManager, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(networkManager)
    , userManager(userManager)
    , friendManager(friendManager)
    , avatarLabel(nullptr)
    , usernameLabel(nullptr)
    , statusIndicator(nullptr)
    , searchLineEdit(nullptr)
    , friendListWidget(nullptr)
    , deleteFriendButton(nullptr)
    , refreshListButton(nullptr)
    , welcomeLabel(nullptr)
    , isDeletingFriend(false)
    , isViewingFriendInfo(false)
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

    userManager->QueryUserStatus();  // 查询用户状态
    friendManager->GetFriendList();  // 获取好友列表
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
 * @brief 设置内容区域（右侧面板）
 * @param centralLayout 中央区域布局指针
 */
void MainWindow::SetupContentArea(QHBoxLayout* centralLayout){
    QWidget* contentArea = new QWidget();  // 创建内容区域
    QVBoxLayout* contentLayout = new QVBoxLayout(contentArea);  // 创建内容区域垂直布局
    contentLayout->setAlignment(Qt::AlignCenter);  // 设置内容区域布局居中

    welcomeLabel = new QLabel("欢迎使用 LiteChat", contentArea);  // 创建欢迎标签
    welcomeLabel->setAlignment(Qt::AlignCenter);  // 设置欢迎标签居中
    welcomeLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #333333;");  // 设置欢迎标签样式

    contentLayout->addWidget(welcomeLabel);  // 添加欢迎标签

    centralLayout->addWidget(contentArea);  // 添加内容区域
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
 * @brief 事件过滤器，用于处理头像点击事件
 * @param obj 事件对象
 * @param event 事件
 * @return 是否处理事件
 */
bool MainWindow::eventFilter(QObject* obj, QEvent* event){
    if(obj == avatarLabel && event->type() == QEvent::MouseButtonPress){
        OnAvatarClicked();
        return true;
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
    isDeletingFriend = false;
    QMessageBox::information(this, "成功", "删除好友成功");
    friendManager->GetFriendList();  // 刷新好友列表
    qDebug() << "[MainWindow::OnFriendDeleted]删除好友后自动调用槽函数" << username;  // Debug输出
}

/**
 * @brief 删除好友失败后，手动触发自定义信号，自动调用槽函数
 * @param errorMsg 错误信息
 */
void MainWindow::OnFriendDeleteFailed(const QString& errorMsg){
    isDeletingFriend = false;
    QMessageBox::warning(this, "错误", "删除好友失败：" + errorMsg);
    qDebug() << "[MainWindow::OnFriendDeleteFailed]删除好友失败后自动调用槽函数" << errorMsg;  // Debug输出
}

/**
 * @brief 好友状态改变后，手动触发自定义信号，自动调用槽函数
 * @param username 好友用户名
 * @param status 好友状态
 */
void MainWindow::OnFriendStatusChanged(const QString& username, UserStatus status){
    UpdateFriendListStatus(username, status);  // 更新好友列表中的状态指示器
    qDebug() << "[MainWindow::OnFriendStatusChanged]好友状态改变后自动调用槽函数" << username;  // Debug输出
}

/**
 * @brief 查询好友结果后，手动触发自定义信号，自动调用槽函数
 * @param info 好友信息
 */
void MainWindow::OnQueryFriendResult(const FriendInfo& info){
    SearchResultDialog searchResultDlg(friendManager, this);  // 创建搜索结果对话框
    searchResultDlg.SetResultInfo(info.username, info.status);  // 设置搜索结果信息
    if(isViewingFriendInfo){
        searchResultDlg.SetViewOnlyMode(true);  // 设置仅查看模式
        isViewingFriendInfo = false;  // 重置标志
    }
    searchResultDlg.exec();  // 显示搜索结果对话框
    qDebug() << "[MainWindow::OnQueryFriendResult]查询好友结果后自动调用槽函数";  // Debug输出
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
        QMessageBox::information(this, "提示", "在加班了，别急", QMessageBox::Ok);
        return;
    }
    QListWidgetItem* currentItem = friendListWidget->currentItem();  // 获取当前选中项
    if(!currentItem){
        QMessageBox::warning(this, "错误", "请先选择要删除的好友");
        return;
    }
    QString username = currentItem->data(Qt::UserRole).toString();  // 获取用户名
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除好友", "确定要删除好友 \"" + username + "\" 吗？\n删除后，双方的历史聊天记录将清空。",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No
    );  // 确认删除好友弹窗
    if(reply == QMessageBox::Yes){
        isDeletingFriend = true;
        friendManager->DeleteFriend(username);  // 发送删除好友请求
    }
    qDebug() << "[MainWindow::OnDeleteFriendClicked]删除好友按钮被点击后自动调用槽函数";  // Debug输出
}

/**
 * @brief 刷新列表按钮被点击后，自动触发自带的信号，自动调用槽函数
 */
void MainWindow::OnRefreshListClicked(){
    friendManager->GetFriendList();  // 发送获取好友列表请求
    ui->statusbar->showMessage("好友列表已刷新", 1500);  // 状态栏显示刷新提示，1.5秒自动消失
    qDebug() << "[MainWindow::OnRefreshListClicked]刷新列表按钮被点击后自动调用槽函数";  // Debug输出
}

/**
 * @brief 好友列表右键菜单请求后，自动触发自带的信号，自动调用槽函数
 * @param pos 右键点击位置
 */
void MainWindow::OnFriendListContextMenu(const QPoint& pos){
    QListWidgetItem* item = friendListWidget->itemAt(pos);  // 获取右键点击位置对应的列表项
    if(!item) return;
    QMenu contextMenu(this);  // 创建右键菜单
    QAction* infoAction = contextMenu.addAction("查看资料");  // 添加查看资料菜单项
    QAction* deleteAction = contextMenu.addAction("删除好友");  // 添加删除好友菜单项
    QAction* selectedAction = contextMenu.exec(friendListWidget->mapToGlobal(pos));  // 显示右键菜单
    if(selectedAction == infoAction){
        QString username = item->data(Qt::UserRole).toString();  // 获取用户名
        isViewingFriendInfo = true;  // 设置查看资料模式
        friendManager->QueryFriendInfo(username);  // 发送查询好友请求
    }
    else if(selectedAction == deleteAction){
        QString username = item->data(Qt::UserRole).toString();  // 获取用户名
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "确认删除好友", "确定要删除好友 \"" + username + "\" 吗？\n删除后，双方的历史聊天记录将清空。",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No
        );  // 确认删除好友弹窗
        if(reply == QMessageBox::Yes){
            friendManager->DeleteFriend(username);  // 发送删除好友请求
        }
    }
    qDebug() << "[MainWindow::OnFriendListContextMenu]好友列表右键菜单请求后自动调用槽函数";  // Debug输出
}