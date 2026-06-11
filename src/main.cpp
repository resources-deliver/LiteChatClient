#include "mainwindow.h"
#include "connectdialog.h"
#include "networkmanager.h"

#include <QApplication>

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    NetworkManager networkManager;
    ConnectDialog connectDlg(&networkManager);

    if(connectDlg.exec() == QDialog::Accepted){
        MainWindow w;
        w.show();
        return QApplication::exec();
    }
    return 0;
}