#include "mainwindow.h"
#include "connectdialog.h"
#include "logindialog.h"
#include "registerdialog.h"
#include "networkmanager.h"
#include "usermanager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    NetworkManager networkManager;
    UserManager userManager(&networkManager);

    ConnectDialog connectDlg(&networkManager);

    if (connectDlg.exec() == QDialog::Accepted)
    {
        while (true)
        {
            LoginDialog loginDlg(&userManager);
            int loginResult = loginDlg.exec();

            if (loginResult == QDialog::Accepted)
            {
                MainWindow w;
                w.show();
                return QApplication::exec();
            }
            else if (loginResult == 1)
            {
                RegisterDialog registerDlg(&userManager);
                int registerResult = registerDlg.exec();

                if (registerResult == 0)
                {
                    loginDlg.SetUsername(registerDlg.GetRegisteredUsername());
                }
            }
            else
            {
                break;
            }
        }
    }

    return 0;
}