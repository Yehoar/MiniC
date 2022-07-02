#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <QTextBrowser>
#include "minic.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_PBTN_OpenFile_clicked();

    void on_PBTN_Compile_clicked();

    void on_PBTN_Token_clicked();

    void on_PBTN_SyntaxTree_clicked();

    void on_PBTN_Symbol_clicked();

    void on_PBTN_IR_clicked();

    void on_PBTN_Run_clicked();

private:
    Ui::MainWindow *ui;

private:
    QString output[4];
    Minic minic;
    bool isOk{false};
};
#endif // MAINWINDOW_H
