#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->TB_Output->setLineWrapMode(QTextBrowser::LineWrapMode::NoWrap);
    ui->TB_SourceCode->setLineWrapMode(QTextBrowser::LineWrapMode::NoWrap);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_PBTN_OpenFile_clicked()
{
    // 选择文件
    QString filePath = QFileDialog::getOpenFileName(NULL,"选择文件",".","*.mc");
    ui->LE_filepath->setText(filePath);
    if(filePath.isEmpty())
    {
        this->isOk=false;
        return;
    }
    // 读取文件
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->TB_SourceCode->append("文件打开失败");
        return;
    }
    QString content = file.readAll();
    ui->TB_SourceCode->setText(content);
}


void MainWindow::on_PBTN_Compile_clicked()
{
    QString filename = ui->LE_filepath->text();
    if(!filename.isEmpty())
    {
        int ret = minic.Compile(filename);
        output[0] = filename+".token";
        output[1] = filename+".ast";
        output[2] = filename+".sym";
        output[3] = filename+".ir";
        this->on_PBTN_Token_clicked();
        QString tips;
        switch (ret) {
        case 0:
        {
            this->isOk = true;
            QMessageBox::information(this,tr("提示"),tr("编译成功"));
            return;
        }
        case -2:
        {
            tips = "词法分析失败";
            break;
        }
        case -3:
        {
            tips = "语法分析失败";
            break;
        }
        case -4:
        {
            tips = "符号表建立/类型检查失败";
            break;
        }
        default:
        {
            break;
        }
        }
        if(!tips.isEmpty())
        {
            QMessageBox::warning(this,tr("提示"),tips);
        }
    }
}

void MainWindow::on_PBTN_Token_clicked()
{
    if(output[0].isEmpty())
    {
        return;
    }
    QFile file(output[0]);
    if(!file.open(QFile::ReadOnly))
    {
        ui->TB_Output->setText(output[0]);
        ui->TB_Output->append("文件打开失败！请重新编译！\n");
        return;
    }
    QString content = file.readAll();
    ui->TB_Output->setText(content);
}


void MainWindow::on_PBTN_SyntaxTree_clicked()
{
    if(output[1].isEmpty())
    {
        return;
    }
    QFile file(output[1]);
    if(!file.open(QFile::ReadOnly))
    {
        ui->TB_Output->setText(output[1]);
        ui->TB_Output->append("文件打开失败！请重新编译！\n");
        return;
    }
    QString content = file.readAll();
    ui->TB_Output->setText(content);
}


void MainWindow::on_PBTN_Symbol_clicked()
{
    if(output[2].isEmpty())
    {
        return;
    }
    QFile file(output[2]);
    if(!file.open(QFile::ReadOnly))
    {
        ui->TB_Output->setText(output[2]);
        ui->TB_Output->append("文件打开失败！请重新编译！\n");
        return;
    }
    QString content = file.readAll();
    ui->TB_Output->setText(content);
}


void MainWindow::on_PBTN_IR_clicked()
{
    if(output[3].isEmpty())
    {
        return;
    }
    QFile file(output[3]);
    if(!file.open(QFile::ReadOnly))
    {
        ui->TB_Output->setText(output[3]);
        ui->TB_Output->append("文件打开失败！请重新编译！\n");
        return;
    }
    QString content = file.readAll();
    ui->TB_Output->setText(content);
}

void MainWindow::on_PBTN_Run_clicked()
{
    if(this->isOk)
    {
        int ret = minic.Run(QApplication::applicationDirPath(), output[3]);
        if(ret < 0)
        {
            QMessageBox::warning(this,"提示","运行失败，请尝试在命令行中执行");
        }
    }
}

