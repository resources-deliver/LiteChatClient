#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>

#include "usermanager.h"

QT_BEGIN_NAMESPACE
namespace Ui{
class MainWindow;
}
QT_END_NAMESPACE

class NetworkManager;

/**
 * @brief 主窗口类，负责显示聊天界面和连接服务器的对话框
 */
class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    explicit MainWindow(NetworkManager* networkManager, UserManager* userManager, QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void OnDisconnected();
    void OnStatusChanged(UserStatus status);
    void OnAvatarClicked();

private:
    void SetupTopBar(QVBoxLayout* mainLayout);
    void SetupContentArea(QVBoxLayout* mainLayout);
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    Ui::MainWindow *ui;  // 主窗口的（UI界面）的指针
    NetworkManager* networkManager;  // 网络管理器的指针
    UserManager* userManager;  // 用户管理器的指针
    QLabel* avatarLabel;  // 头像标签的指针
    QLabel* usernameLabel;  // 用户名标签的指针
    QLabel* statusIndicator;  // 状态指示器标签的指针
    QLineEdit* searchLineEdit;  // 搜索输入框的指针
    QLabel* welcomeLabel;  // 欢迎标签的指针
};
#endif // MAINWINDOW_H