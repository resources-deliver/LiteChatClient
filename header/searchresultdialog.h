#ifndef SEARCHRESULTDIALOG_H
#define SEARCHRESULTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

#include "usermanager.h"

class FriendManager;

/**
 * @brief 搜索结果对话框，用于显示查询到的用户信息并提供添加好友功能
 */
class SearchResultDialog : public QDialog{
    Q_OBJECT

public:
    explicit SearchResultDialog(FriendManager* friendManager, QWidget *parent = nullptr);
    ~SearchResultDialog() override;
    void SetResultInfo(const QString& username, UserStatus status);
    void SetViewOnlyMode(bool viewOnly);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void OnAddFriendClicked();
    void OnCloseClicked();
    void OnAddFriendTimeout();
    void OnFriendAdded(const QString& username);
    void OnFriendAddFailed(const QString& errorMsg);

private:
    void SetupUI();
    void ShowBusyMessage();

private:
    FriendManager* friendManager;  // 好友管理器的指针
    QLabel* titleLabel;  // 标题标签的指针
    QLabel* usernameLabel;  // 用户名标签的指针
    QLabel* statusLabel;  // 状态标签的指针
    QLabel* statusIndicator;  // 状态指示器标签的指针
    QPushButton* addFriendButton;  // 添加好友按钮的指针
    QPushButton* closeButton;  // 关闭按钮的指针
    bool isProcessing;  // 添加好友请求状态
    bool ignoreLateResponse;  // 忽略延迟响应标志
    QTimer* timeoutTimer;  // 时间定时器的指针
    QString queriedUsername;  // 查询到的用户名
};

#endif // SEARCHRESULTDIALOG_H