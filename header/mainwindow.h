#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSystemTrayIcon>
#include <QTimer>
#include "usermanager.h"
#include "friendmanager.h"
#include "messagemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class NetworkManager;

/**
 * @brief 主窗口类，负责显示聊天界面和好友管理
 */
class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    explicit MainWindow(NetworkManager* networkManager, UserManager* userManager, FriendManager* friendManager, MessageManager* messageManager, QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void OnDisconnected();
    void OnStatusChanged(UserStatus status);
    void OnAvatarClicked();
    void OnSearchReturnPressed();
    void OnFriendListReceived(const QList<FriendInfo>& friendList);
    void OnFriendListFailed(const QString& errorMsg);
    void OnFriendAdded(const QString& username);
    void OnFriendDeleted(const QString& username);
    void OnFriendDeleteFailed(const QString& errorMsg);
    void OnFriendStatusChanged(const QString& username, UserStatus status);
    void OnQueryFriendResult(const FriendInfo& info);
    void OnQueryFriendFailed(const QString& errorMsg);
    void OnDeleteFriendClicked();
    void OnRefreshListClicked();
    void OnFriendListContextMenu(const QPoint& pos);
    void OnFriendListItemClicked(QListWidgetItem* item);
    void OnSendMessageClicked();
    void OnMessageInputReturnPressed();
    void OnMessageReceived(const Message& msg);
    void OnHistoryReceived(const QString& friendUsername, const QList<Message>& messages);
    void OnMessageSendSuccess(const Message& msg);
    void OnMessageSendFailed(const QString& errorMsg);
    void OnHistoryFailed(const QString& errorMsg);
    void OnMessageNotify(const QString& sender, int count);

private:
    void SetupTopBar(QVBoxLayout* mainLayout);
    void SetupLeftPanel(QHBoxLayout* centralLayout);
    void SetupContentArea(QHBoxLayout* centralLayout);
    void SetupBottomBar(QVBoxLayout* mainLayout);
    void SetupChatPage();
    void ClearFriendList();
    void UpdateFriendListStatus(const QString& username, UserStatus status);
    bool eventFilter(QObject* obj, QEvent* event) override;
    void SwitchToChatView(const QString& friendUsername);
    void DisplayMessage(const Message& msg);
    void DisplayMessages(const QList<Message>& messages);
    void ClearMessageList();
    void ScrollMessageListToBottom();
    QString FormatTimestamp(const QString& timestamp) const;
    void ShowNotificationBubble(const QString& sender, const QString& summary);

private:
    Ui::MainWindow *ui;  // 主窗口的（UI界面）的指针
    NetworkManager* networkManager;  // 网络管理器的指针
    UserManager* userManager;  // 用户管理器的指针
    FriendManager* friendManager;  // 好友管理器的指针
    MessageManager* messageManager;  // 消息管理器的指针
    QLabel* avatarLabel;  // 头像标签的指针
    QLabel* usernameLabel;  // 用户名标签的指针
    QLabel* statusIndicator;  // 状态指示器标签的指针
    QLineEdit* searchLineEdit;  // 搜索输入框的指针
    QListWidget* friendListWidget;  // 好友列表控件的指针
    QPushButton* deleteFriendButton;  // 删除好友按钮的指针
    QPushButton* refreshListButton;  // 刷新列表按钮的指针
    QLabel* welcomeLabel;  // 欢迎标签的指针
    QStackedWidget* contentStack;  // 内容区域堆栈控件的指针
    QWidget* chatPage;  // 聊天页面控件的指针
    QLabel* chatFriendLabel;  // 聊天好友名称标签的指针
    QListWidget* messageListWidget;  // 消息列表控件的指针
    QPlainTextEdit* messageInput;  // 消息输入框的指针
    QPushButton* sendButton;  // 发送按钮的指针
    QSystemTrayIcon* trayIcon;  // 系统托盘图标指针
    QLabel* notificationBubble;  // 通知气泡标签指针
    QString currentChatFriend;  // 当前聊天好友用户名
    bool isDeletingFriend;  // 删除好友处理状态
    bool isViewingFriendInfo;  // 查看好友资料标志
    bool isSendingMessage;  // 发送消息处理状态
};

#endif // MAINWINDOW_H