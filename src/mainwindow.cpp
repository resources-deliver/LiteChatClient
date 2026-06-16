#include "mainwindow.h"
#include "ui_mainwindow.h"

/**
 * @brief MainWindow构造函数，用于初始化类内私有属性+初始化UI
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);  // 初始化主窗口（UI界面）
}

/**
 * @brief MainWindow析构函数，用于释放动态分配的资源
 */
MainWindow::~MainWindow(){
    delete ui;  // 释放主窗口（UI界面）的指针
}
