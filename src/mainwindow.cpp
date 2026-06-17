#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "networkmanager.h"
#include "usermanager.h"
#include "userinfodialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QDebug>

/**
 * @brief MainWindow构造函数，用于初始化类内私有属性+初始化UI+显示基本窗口信息
 * @param networkManager 网络管理器指针
 * @param userManager 用户管理器指针
 * @param parent 父窗口
 */
MainWindow::MainWindow(NetworkManager* networkManager, UserManager* userManager, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(networkManager)
    , userManager(userManager)
    , avatarLabel(nullptr)
    , usernameLabel(nullptr)
    , statusIndicator(nullptr)
    , searchLineEdit(nullptr)
    , welcomeLabel(nullptr)
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
    SetupContentArea(mainLayout);  // 设置内容区域

    setCentralWidget(centralWidget);  // 设置中央窗口部件

    // 连接断开后，自动触发自带的信号，自动调用槽函数
    connect(networkManager, &NetworkManager::Disconnected, this, &MainWindow::OnDisconnected);
    // 状态改变后，手动触发自定义信号，自动调用槽函数
    connect(userManager, &UserManager::StatusChanged, this, &MainWindow::OnStatusChanged);

    userManager->QueryUserStatus();  // 查询用户状态
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

    topLayout->addWidget(avatarLabel);  // 添加头像标签
    topLayout->addWidget(usernameLabel);  // 添加用户名标签
    topLayout->addWidget(statusIndicator);  // 添加在线状态指示灯
    topLayout->addStretch();  // 添加弹性空间
    topLayout->addWidget(searchLineEdit);  // 添加搜索框

    mainLayout->addWidget(topBar);  // 添加顶部栏
}

/**
 * @brief 设置内容区域
 * @param mainLayout 主布局指针
 */
void MainWindow::SetupContentArea(QVBoxLayout* mainLayout){
    QWidget* contentArea = new QWidget();  // 创建内容区域
    QVBoxLayout* contentLayout = new QVBoxLayout(contentArea);  // 创建内容区域垂直布局
    contentLayout->setAlignment(Qt::AlignCenter);  // 设置内容区域布局居中

    welcomeLabel = new QLabel("欢迎使用 LiteChat", contentArea);  // 创建欢迎标签
    welcomeLabel->setAlignment(Qt::AlignCenter);  // 设置欢迎标签居中
    welcomeLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #333333;");  // 设置欢迎标签样式

    contentLayout->addWidget(welcomeLabel);  // 添加欢迎标签

    mainLayout->addWidget(contentArea);  // 添加内容区域
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