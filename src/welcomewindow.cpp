#include "welcomewindow.h"
#include "ui_welcomewindow.h"
#include <filesystem>
#include "src/ConstantDefination.h"
#include <QMessageBox>
#include <thread>
#include <chrono>
#include <src/ConstantDefination.h>

WelcomeWindow::WelcomeWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WelcomeWindow)
{
    ui->setupUi(this);

}

WelcomeWindow::~WelcomeWindow()
{
    delete ui;
}

bool WelcomeWindow::checkIfRequirementSatisfied()
{
    // check if model file exists
    if(mConfigManager.readConfigFile(defaultConfigFilePath,mConfig)){
        if(!std::filesystem::exists(mConfig.RVMModelFilePath)){
            QMessageBox msgBox;

            msgBox.setWindowTitle(QString::fromLocal8Bit("错误！"));
            msgBox.setText(QString::fromLocal8Bit("环境检查失败！配置文件指定的模型文件缺失，即将用默认模型文件替换配置项！"));
            msgBox.exec();

            mConfig.RVMModelFilePath = defaultRVMModelFilePath;

            if(!mConfigManager.writeConfigFile(defaultConfigFilePath,mConfig)){
                msgBox.setWindowTitle(QString::fromLocal8Bit("错误！"));
                msgBox.setText(QString::fromLocal8Bit("环境检查失败！配置文件指定的模型文件缺失，并且默认模型文件替换配置项失败！"));
                msgBox.exec();
                return false;
            }

            if(!std::filesystem::exists(defaultRVMModelFilePath)){
                QMessageBox msgBox;
                msgBox.setWindowTitle(QString::fromLocal8Bit("错误！"));
                msgBox.setText(QString::fromLocal8Bit("环境检查失败！默认模型文件不存在，请重新安装此程序！"));
                msgBox.exec();
                return false;
            }

            return false;
        }
    }

    for( int i = 0;i<=100;i++){
        ui->progressBar->setValue(i);
        qApp->processEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    return true;
}
