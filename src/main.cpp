#include "mainwindow.h"
#include "connectdialog.h"
#include "logindialog.h"
#include "registerdialog.h"
#include "networkmanager.h"
#include "usermanager.h"
#include "friendmanager.h"
#include "messagemanager.h"
#include "clientlogger.h"

#include <QApplication>
#include <QDir>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg){
    QByteArray localMsg = msg.toUtf8();
    fprintf(stderr, "%s\n", localMsg.constData());
    fflush(stderr);
}

int main(int argc, char *argv[]){
#ifdef Q_OS_WIN
    SetConsoleOutputCP(CP_UTF8);
#endif
    QApplication a(argc, argv);
    qInstallMessageHandler(customMessageHandler);

    QString logDir = QCoreApplication::applicationDirPath() + "/logs";
    ClientLogger::GetInstance().InitLogger(logDir);
    ClientLogger::GetInstance().WriteLog(LogLevel::INFO, "Main", "LiteChat客户端启动");

    NetworkManager networkManager;
    UserManager userManager(&networkManager);
    FriendManager friendManager(&networkManager);
    MessageManager messageManager(&networkManager, &friendManager);
    ConnectDialog connectDlg(&networkManager);

    if(connectDlg.exec() == QDialog::Accepted){
        while(true){
            LoginDialog loginDlg(&userManager);
            int loginResult = loginDlg.exec();
            if(loginResult == QDialog::Accepted){
                messageManager.SetCurrentUsername(userManager.GetCurrentUsername());
                MainWindow* w = new MainWindow(&networkManager, &userManager, &friendManager, &messageManager);
                w->show();
                return QApplication::exec();
            }
            else if(loginResult == 2){
                RegisterDialog registerDlg(&userManager);
                int registerResult = registerDlg.exec();
                if(registerResult == 0){
                    loginDlg.SetUsername(registerDlg.GetRegisteredUsername());
                }
            }
            else{
                break;
            }
        }
    }
    return 0;
}