#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QMenu>

#include "usermanager.h"
#include "friendmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui{
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
    explicit MainWindow(NetworkManager* networkManager, UserManager* userManager, FriendManager* friendManager, QWidget *parent = nullptr);
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

private:
    void SetupTopBar(QVBoxLayout* mainLayout);
    void SetupLeftPanel(QHBoxLayout* centralLayout);
    void SetupContentArea(QHBoxLayout* centralLayout);
    void SetupBottomBar(QVBoxLayout* leftLayout);
    void ClearFriendList();
    void UpdateFriendListStatus(const QString& username, UserStatus status);
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    Ui::MainWindow *ui;  // 主窗口的（UI界面）的指针
    NetworkManager* networkManager;  // 网络管理器的指针
    UserManager* userManager;  // 用户管理器的指针
    FriendManager* friendManager;  // 好友管理器的指针
    QLabel* avatarLabel;  // 头像标签的指针
    QLabel* usernameLabel;  // 用户名标签的指针
    QLabel* statusIndicator;  // 状态指示器标签的指针
    QLineEdit* searchLineEdit;  // 搜索输入框的指针
    QListWidget* friendListWidget;  // 好友列表控件的指针
    QPushButton* deleteFriendButton;  // 删除好友按钮的指针
    QPushButton* refreshListButton;  // 刷新列表按钮的指针
    QLabel* welcomeLabel;  // 欢迎标签的指针
};
#endif // MAINWINDOW_H