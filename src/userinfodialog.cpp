#include "userinfodialog.h"
#include "ui_userinfodialog.h"
#include "usermanager.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>

/**
 * @brief UserInfoDialog构造函数，用于初始化类内私有属性+初始化UI+连接信号槽
 * @param userManager 用户管理器指针
 * @param parent 父窗口
 */
UserInfoDialog::UserInfoDialog(UserManager* userManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UserInfoDialog)
    , userManager(userManager)
    , isProcessing(false)
    , timeoutTimer(new QTimer(this))
{
    ui->setupUi(this);  // 初始化个人信息对话框（UI界面）
    SetupUI();  // 设置个人信息对话框（UI界面）样式

    // 保存按钮被点击后，自动触发自带的信号，自动调用槽函数
    connect(ui->saveButton, &QPushButton::clicked, this, &UserInfoDialog::OnSaveClicked);
    // 取消按钮被点击后，自动触发自带的信号，自动调用槽函数
    connect(ui->cancelButton, &QPushButton::clicked, this, &UserInfoDialog::OnCancelClicked);
    // 时间定时器超时后，自动触发自带的信号，自动调用槽函数
    connect(timeoutTimer, &QTimer::timeout, this, &UserInfoDialog::OnUpdateTimeout);
    // 更新成功后，手动触发自定义信号，自动调用槽函数
    connect(userManager, &UserManager::UpdateSuccess, this, &UserInfoDialog::OnUpdateSuccess);
    // 更新失败后，手动触发自定义信号，自动调用槽函数
    connect(userManager, &UserManager::UpdateFailed, this, &UserInfoDialog::OnUpdateFailed);

    ui->currentUsernameLabel->setText("当前用户名: " + userManager->GetCurrentUsername());  // 设置当前用户名标签的文本
}

/**
 * @brief UserInfoDialog析构函数，用于释放动态分配的资源
 */
UserInfoDialog::~UserInfoDialog(){
    delete ui;  // 释放个人信息对话框（UI界面）的指针
}

/**
 * @brief 设置个人信息对话框（UI界面）样式
 */
void UserInfoDialog::SetupUI(){
    setWindowTitle("个人信息设置");  // 设置窗口标题
    setFixedSize(400, 450);  // 设置固定大小
    setMinimumSize(400, 450);  // 设置最小大小

    ui->titleLabel->setText("个人信息设置");  // 设置标题文本
    ui->titleLabel->setAlignment(Qt::AlignCenter);  // 设置标题文本居中
    ui->titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333333;");  // 设置标题文本样式

    ui->currentUsernameLabel->setStyleSheet("font-size: 12px; color: #666666;");  // 设置当前用户名标签样式

    ui->newUsernameLabel->setText("新用户名（可选）");  // 设置新用户名标签文本
    ui->newUsernameLabel->setStyleSheet("font-size: 12px; color: #666666;");  // 设置新用户名标签样式

    ui->newUsernameLineEdit->setPlaceholderText("请输入新用户名（3-20位）");  // 设置新用户名文本框占位符文本
    ui->newUsernameLineEdit->setFixedHeight(36);  // 设置新用户名文本框固定高度
    ui->newUsernameLineEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "}"
    );  // 设置新用户名文本框样式

    ui->newPasswordLabel->setText("新密码（可选）");  // 设置新密码标签文本
    ui->newPasswordLabel->setStyleSheet("font-size: 12px; color: #666666;");  // 设置新密码标签样式

    ui->newPasswordLineEdit->setPlaceholderText("请输入新密码（至少6位）");  // 设置新密码文本框占位符文本
    ui->newPasswordLineEdit->setEchoMode(QLineEdit::Password);  // 设置新密码文本框为密码模式
    ui->newPasswordLineEdit->setFixedHeight(36);  // 设置新密码文本框固定高度
    ui->newPasswordLineEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "}"
    );  // 设置新密码文本框样式

    ui->confirmPasswordLabel->setText("确认新密码");  // 设置确认新密码标签文本
    ui->confirmPasswordLabel->setStyleSheet("font-size: 12px; color: #666666;");  // 设置确认新密码标签样式

    ui->confirmPasswordLineEdit->setPlaceholderText("请再次输入新密码");  // 设置确认新密码文本框占位符文本
    ui->confirmPasswordLineEdit->setEchoMode(QLineEdit::Password);  // 设置确认新密码文本框为密码模式
    ui->confirmPasswordLineEdit->setFixedHeight(36);  // 设置确认新密码文本框固定高度
    ui->confirmPasswordLineEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 1px solid #D9D9D9;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   font-size: 12px;"
        "}"
    );  // 设置确认新密码文本框样式

    ui->saveButton->setText("保存修改");  // 设置保存修改按钮文本
    ui->saveButton->setFixedSize(200, 36);  // 设置保存修改按钮固定大小
    ui->saveButton->setStyleSheet(
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
    );  // 设置保存修改按钮样式

    ui->cancelButton->setText("取消");  // 设置取消按钮文本
    ui->cancelButton->setFixedSize(200, 36);  // 设置取消按钮固定大小
    ui->cancelButton->setStyleSheet(
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
    );  // 设置取消按钮样式
}

/**
 * @brief 信息弹窗
 */
void UserInfoDialog::ShowBusyMessage(){
    QMessageBox::information(this, "提示", "在加班了，别急", QMessageBox::Ok);
}

/**
 * @brief 显示身份验证密码对话框
 */
void UserInfoDialog::ShowVerifyPasswordDialog(){
    bool ok;
    QString verifyPassword = QInputDialog::getText(
        this, 
        "身份验证",
        "请输入当前密码以验证身份：", 
        QLineEdit::Password, 
        "", 
        &ok
    );  // 通过身份验证密码对话框获取用户输入的密码
    if(ok && !verifyPassword.isEmpty()){  // 如果用户点击了确定按钮并且密码不为空
        isProcessing = true;  // 设置更新请求状态
        ui->saveButton->setEnabled(false);  // 禁用保存修改按钮
        ui->cancelButton->setEnabled(false);  // 禁用取消按钮
        timeoutTimer->start(5000);  // 启动5秒时间定时器
        userManager->UpdateUserInfo(pendingNewUsername, pendingNewPassword, verifyPassword);  // 更新
        qDebug() << "[UserInfoDialog::ShowVerifyPasswordDialog]更新用户信息请求成功";
    }
}

/**
 * @brief 保存按钮被点击后，自动触发自带的信号，自动调用槽函数
 */
void UserInfoDialog::OnSaveClicked(){
    if(isProcessing){  // 如果正在处理更新请求
        qDebug() << "[UserInfoDialog::OnSaveClicked]正在处理更新请求, 请稍后";
        ShowBusyMessage();  // 信息弹窗
        return;
    }
    pendingNewUsername = ui->newUsernameLineEdit->text().trimmed();  // 获取新用户名文本框中的文本并去掉首尾空格
    pendingNewPassword = ui->newPasswordLineEdit->text();  // 获取新密码文本框中的文本
    QString confirmPassword = ui->confirmPasswordLineEdit->text();  // 获取确认新密码文本框中的文本
    if(pendingNewUsername.isEmpty() && pendingNewPassword.isEmpty()){  // 如果用户名和密码都为空
        qDebug() << "[UserInfoDialog::OnSaveClicked]未修改任何信息";
        QMessageBox::warning(this, "错误", "未修改任何信息");  // 错误弹窗
        return;
    }
    if(!pendingNewPassword.isEmpty() && pendingNewPassword != confirmPassword){  // 如果新密码不为空且新密码和确认新密码不一致
        qDebug() << "[UserInfoDialog::OnSaveClicked]两次输入的密码不一致";
        QMessageBox::warning(this, "错误", "两次输入的密码不一致");  // 错误弹窗
        return;
    }
    qDebug() << "[UserInfoDialog::OnSaveClicked]客户端发送更新用户信息请求到服务器";
    ShowVerifyPasswordDialog();  // 显示身份验证密码对话框
}

/**
 * @brief 取消按钮被点击后，自动触发自带的信号，自动调用槽函数
 */
void UserInfoDialog::OnCancelClicked(){
    if(isProcessing){  // 如果正在处理更新请求
        qDebug() << "[UserInfoDialog::OnCancelClicked]正在处理更新请求, 请稍后";
        ShowBusyMessage();  // 信息弹窗
        return;
    }
    qDebug() << "[UserInfoDialog::OnCancelClicked]用户点击了取消按钮";
    reject();  // 关闭个人信息对话框
}

/**
 * @brief 时间定时器超时后，自动触发自带的信号，自动调用槽函数
 */
void UserInfoDialog::OnUpdateTimeout(){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;  // 设置更新请求状态
    ui->saveButton->setEnabled(true);  // 启用保存修改按钮
    ui->cancelButton->setEnabled(true);  // 启用取消按钮
    QMessageBox::warning(this, "错误", "请求超时");  // 错误弹窗
    qDebug() << "[UserInfoDialog::OnUpdateTimeout]时间定时器超时后自动调用槽函数";  // Debug输出
}

/**
 * @brief 更新成功后，手动触发自定义信号，自动调用槽函数
 */
void UserInfoDialog::OnUpdateSuccess(){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;  // 设置更新请求状态
    ui->saveButton->setEnabled(true);  // 启用保存修改按钮
    ui->cancelButton->setEnabled(true);  // 启用取消按钮
    QMessageBox::information(this, "成功", "修改成功");  // 成功弹窗
    qDebug() << "[UserInfoDialog::OnUpdateSuccess]客户端接收更新成功响应,更新成功";
    accept();  // 接受更新成功信号
}

/**
 * @brief 更新失败后，手动触发自定义信号，自动调用槽函数
 * @param errorMsg 错误信息
 */
void UserInfoDialog::OnUpdateFailed(const QString& errorMsg){
    timeoutTimer->stop();  // 停止时间定时器
    isProcessing = false;  // 设置更新请求状态
    ui->saveButton->setEnabled(true);  // 启用保存修改按钮
    ui->cancelButton->setEnabled(true);  // 启用取消按钮
    if(errorMsg == "验证密码错误"){
        QMessageBox::warning(this, "错误", "验证密码错误");
    }
    else if(errorMsg == "用户名已存在"){
        QMessageBox::warning(this, "错误", "用户名已存在");
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
    qDebug() << "[UserInfoDialog::OnUpdateFailed]客户端接收更新失败响应, 错误信息:" << errorMsg;
}