#include "developmodewindow.h"
#include "ui_developmodewindow.h"
#include "src/ConstantDefination.h"
#include <QMessageBox>
#include <QString>
#include <filesystem>
#include <QFileDialog>

DevelopModeWindow::DevelopModeWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DevelopModeWindow)
{
    ui->setupUi(this);


    if(!configManager.readConfigFile(defaultConfigFilePath, mConfig)){
        QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误："),QString::fromLocal8Bit("读取配置文件失败！"),QMessageBox::Ok,this);
        message.exec();
    }

    setUISettings();

}

DevelopModeWindow::~DevelopModeWindow()
{
    delete ui;
}

void DevelopModeWindow::setUISettings(){
    ui->spinBox_7->setValue(mConfig.num_threads);

    ui->lineEdit->setText(QString::fromStdString(mConfig.RVMModelFilePath));
}

void DevelopModeWindow::getUISettings(){
    mConfig.num_threads = ui->spinBox_7->value();

    mConfig.RVMModelFilePath = ui->lineEdit->text().toStdString();
}

void DevelopModeWindow::on_pushButton_2_clicked()
{
    getUISettings();

    if(!std::filesystem::exists(std::filesystem::path(mConfig.RVMModelFilePath))){
        QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误："),QString::fromLocal8Bit("路径下的模型文件不存在！"),QMessageBox::Ok,this);
        message.exec();
        return;
    }

    if(configManager.writeConfigFile(defaultConfigFilePath, mConfig)){
        QMessageBox message(QMessageBox::Information,QString::fromLocal8Bit("提示："),QString::fromLocal8Bit("写入配置文件成功！"),QMessageBox::Ok,this);
        message.exec();
    }else{
        QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误："),QString::fromLocal8Bit("写入配置文件失败！"),QMessageBox::Ok,this);
        message.exec();
    }
}


void DevelopModeWindow::on_pushButton_clicked()
{
    QFileDialog fileDialog;
    fileDialog.setWindowTitle(tr("Open Model File"));
    fileDialog.setDirectory(".");
    fileDialog.setNameFilter(tr("Model (*.*)"));
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setViewMode(QFileDialog::Detail);

    if (fileDialog.exec())
    {
        QString fileName;
        fileName=fileDialog.selectedFiles()[0];

        ui->lineEdit->setText(fileName);

    }
}

