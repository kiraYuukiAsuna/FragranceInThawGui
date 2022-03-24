#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QDebug>
#include <filesystem>
#include <src/welcomewindow.h>
#include "src/videoprocessing.h"
#include <QMessageBox>
#include <QFileDialog>
#include "src/ConstantDefination.h"
#include "src/RVMInvoke/RVMInvoke.h"
#include <json.hpp>
#include "developmodewindow.h"
#include <fstream>
#include "aboutdialog.h"
#include "helpdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->hide();

    WelcomeWindow welcomeWindow;
    welcomeWindow.show();

    if(welcomeWindow.checkIfRequirementSatisfied()){
        welcomeWindow.accept();
    }else{
        welcomeWindow.reject();
    }

    if(!configManager.readConfigFile(defaultConfigFilePath, mConfig)){
        QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误："),QString::fromLocal8Bit("读取配置文件失败！"),QMessageBox::Ok,this);
        message.exec();
    }

    setUISettings();

    mRobustVideoMatting = new RobustVideoMatting(mConfig.RVMModelFilePath, defaultNumThread);

    refreshCameraDevice();

    this->show();

}

MainWindow::~MainWindow()
{
    delete mRobustVideoMatting;
    delete ui;
}

void MainWindow::setUISettings(){
    ui->spinBox_6->setValue(mConfig.VideoOutputFPS);

    ui->doubleSpinBox->setValue(mConfig.Downsample_ratio);

    ui->spinBox_10->setValue(mConfig.NetInputImgSize.width);
    ui->spinBox_11->setValue(mConfig.NetInputImgSize.height);

    ui->spinBox_7->setValue(mConfig.BackgroundColor.val[0]);
    ui->spinBox_8->setValue(mConfig.BackgroundColor.val[1]);
    ui->spinBox_9->setValue(mConfig.BackgroundColor.val[2]);
}

void MainWindow::getUISettings(){
    mConfig.VideoOutputFPS = ui->spinBox_6->value();

    mConfig.Downsample_ratio = ui->doubleSpinBox->value();

    mConfig.NetInputImgSize.width = ui->spinBox_10->value();
    mConfig.NetInputImgSize.height = ui->spinBox_11->value();

    mConfig.BackgroundColor.val[0] = ui->spinBox_7->value();
    mConfig.BackgroundColor.val[1] = ui->spinBox_8->value();
    mConfig.BackgroundColor.val[2] = ui->spinBox_9->value();
}

void MainWindow::refreshCameraDevice()
{
    VideoProcessingBase vp;

    std::vector<std::string> cameraDevice;
    int num = vp.enumCameraIndex(cameraDevice);

    ui->label_4->setText(QString::fromLocal8Bit("摄像头数量：")+QString::fromStdString(std::to_string(num)));

    ui->comboBox->clear();

    for(auto& i : cameraDevice){
        ui->comboBox->addItem(QString::fromStdString(i));
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    if(mSettings.currentVideoProcessStatus){
        return;
    }

    mSettings.currentVideoProcessStatus = true;

    VideoProcessingFile vpf;


    std::string fileName = ui->lineEdit->text().toStdString();
    std::string fileSavePath = ui->lineEdit_3->text().toStdString();

    std::filesystem::path stdFilePath;
    std::filesystem::path stdFileSavePath;
    int writer_fps;
    float downsample_ratio;

    cv::Scalar backgroundColor;
    cv::Size netInputSize(ui->spinBox_10->value(),ui->spinBox_11->value());

    try{
        stdFilePath=fileName;
        stdFileSavePath=fileSavePath;
    }catch(std::exception& e){
        QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误："),"Exception: "+QString::fromStdString(e.what()),QMessageBox::Ok,this);
        message.exec();
    }

    if(ui->checkBox->isChecked()){
        writer_fps=0;
    }else{
        writer_fps = ui->spinBox_6->value();
    }

    downsample_ratio=ui->doubleSpinBox->value();

    backgroundColor=cv::Scalar(ui->spinBox_7->value(),ui->spinBox_8->value(),ui->spinBox_9->value());

    if(std::filesystem::exists(stdFilePath) && std::filesystem::exists(stdFileSavePath)){
        if(vpf.openFile(fileName)){
            std::string outputFileName = stdFileSavePath.string()+"/output_"+stdFilePath.filename().string();
            // 0. init video capture
            cv::VideoCapture video_capture(fileName);
            const unsigned int width = video_capture.get(cv::CAP_PROP_FRAME_WIDTH);
            const unsigned int height = video_capture.get(cv::CAP_PROP_FRAME_HEIGHT);
            const unsigned int frame_count = video_capture.get(cv::CAP_PROP_FRAME_COUNT);
            if (writer_fps == 0)// not setting fps, we consider it as the input video fps
            {
                writer_fps = video_capture.get(cv::CAP_PROP_FPS);
            }
            if (!video_capture.isOpened())
            {
                QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误："),"Exception: "+QString::fromStdString("不能打开视频文件：")+QString::fromStdString(fileName),QMessageBox::Ok,this);
                message.exec();
                return;
            }
            // 1. init video writer
            cv::VideoWriter video_writer(outputFileName, cv::VideoWriter::fourcc('m', 'p', '4', 'v'),
                                         writer_fps, cv::Size(width, height));
            if (!video_writer.isOpened())
            {
                QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误："),"Exception: "+QString::fromStdString("不能创建处理后视频的输出文件：")+QString::fromStdString(outputFileName),QMessageBox::Ok,this);
                message.exec();
                return;
            }

            if(width<netInputSize.width && height<netInputSize.height){
                netInputSize=cv::Size(width,height);
            }

            // 2. matting loop
            auto totalFrameCount = QString::fromStdString(std::to_string(frame_count));
            unsigned int i = 0;
            cv::Mat mat;
            while (video_capture.read(mat))
            {
                if(!mSettings.currentVideoProcessStatus){
                    QMessageBox message(QMessageBox::Information,QString::fromLocal8Bit("提示："),QString::fromLocal8Bit("停止处理成功！"),QMessageBox::Ok,this);
                    message.exec();
                    std::filesystem::remove(std::filesystem::path(outputFileName));
                    ui->label_17->setText("");
                    return;
                }

                auto ct1 = std::chrono::high_resolution_clock::now();
                auto t1 = std::chrono::time_point_cast<std::chrono::milliseconds>(ct1).time_since_epoch()
                          .count();
                i += 1;

                cv::Mat resizedImg;
                cv::resize(mat, resizedImg, netInputSize);

                MattingContent content;
                mRobustVideoMatting->detect(resizedImg, content, downsample_ratio, backgroundColor, true); // video_mode true


                cv::resize(content.merge_mat, mat, cv::Size(width, height));


                // 3. save contents and writing out.
                if (content.flag)
                {
                    if (mSettings.saveVideoContents) {
                        mVideoContents.push_back(content);
                    }
                    if (!content.merge_mat.empty())
                    {
                        video_writer.write(mat);
                    }
                }
                // 4. check context states.
                if (!mRobustVideoMatting->context_is_update) break;

                auto ct2 = std::chrono::high_resolution_clock::now();
                auto t2 = std::chrono::time_point_cast<std::chrono::milliseconds>(ct2).time_since_epoch()
                          .count();

                ui->label_17->setText(QString::fromLocal8Bit("已处理帧数/总帧数：")+QString::fromStdString(std::to_string(i))+"/"+totalFrameCount);

                //std::cout << "time consume is : " << t2 - t1 << "\n";

                //std::cout << i << "/" << frame_count << " done!" << "\n";

                qApp->processEvents();
            }

            // 5. release
            video_capture.release();
            video_writer.release();

            mSettings.currentVideoProcessStatus = false;

            QMessageBox message(QMessageBox::Information,QString::fromLocal8Bit("提示："),QString::fromLocal8Bit("视频处理完成，输出的处理后的视频文件已存放到指定文件夹下！"),QMessageBox::Ok,this);
            message.exec();

            ui->label_17->setText("");

            return;
        }else{
            QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误："),QString::fromLocal8Bit("打开指定的视频文件失败！"),QMessageBox::Ok,this);
            message.exec();
        }
    }else{
        QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误："),QString::fromLocal8Bit("没有找到指定的视频文件！"),QMessageBox::Ok,this);
        message.exec();
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    refreshCameraDevice();
}

RobustVideoMatting* MainWindow::safeRVMInvokeAccess()
{
    if(mRobustVideoMatting!=nullptr){
        return mRobustVideoMatting;
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    if(mSettings.currentCameraProcessStatus){
        return;
    }

    mSettings.currentCameraProcessStatus = true;

    int cameraIndex = ui->comboBox->currentIndex();
    qDebug()<<cameraIndex;

    VideoProcessingCamera vpc;

    std::string fileName = ui->lineEdit->text().toStdString();
    std::string fileSavePath = ui->lineEdit_3->text().toStdString();

    std::filesystem::path stdFilePath;
    std::filesystem::path stdFileSavePath;
    int writer_fps;
    float downsample_ratio;

    cv::Scalar backgroundColor;
    cv::Size netInputSize(ui->spinBox_10->value(),ui->spinBox_11->value());

    if(ui->checkBox->isChecked()){
        writer_fps=0;
    }else{
        writer_fps = ui->spinBox_6->value();
    }

    downsample_ratio=ui->doubleSpinBox->value();

    backgroundColor=cv::Scalar(ui->spinBox_7->value(),ui->spinBox_8->value(),ui->spinBox_9->value());

    if(!vpc.openCamera(cameraIndex)){
        QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误："),QString::fromLocal8Bit("打开指定的摄像头设备失败！"),QMessageBox::Ok,this);
        message.exec();
    }else{
        cv::Mat frame;
        cv::Size frameSize = vpc.getFrameSize();

        if(frameSize.width<netInputSize.width && frameSize.height<netInputSize.height){
            netInputSize=cv::Size(frameSize.width,frameSize.height);
        }

        while(true){
            if(!mSettings.currentCameraProcessStatus){
                QMessageBox message(QMessageBox::Information,QString::fromLocal8Bit("提示："),QString::fromLocal8Bit("停止处理成功！"),QMessageBox::Ok,this);
                message.exec();
                break;
            }

            auto ret = vpc.getFrame(frame);
            if(ret){
                cv::imshow("origin", frame);

                // 2. matting loop


                auto ct1 = std::chrono::high_resolution_clock::now();
                auto t1 = std::chrono::time_point_cast<std::chrono::milliseconds>(ct1).time_since_epoch()
                          .count();

                cv::Mat resizedImg;
                cv::resize(frame, resizedImg, netInputSize);

                MattingContent content;
                mRobustVideoMatting->detect(resizedImg, content, downsample_ratio, backgroundColor, true); // video_mode true


                cv::resize(content.merge_mat, frame, frameSize);


                // 3. save contents and writing out.
                if (content.flag)
                {
                    if (mSettings.saveVideoContents) {
                        mVideoContents.push_back(content);
                    }
                }
                // 4. check context states.
                if (!mRobustVideoMatting->context_is_update) break;

                auto ct2 = std::chrono::high_resolution_clock::now();
                auto t2 = std::chrono::time_point_cast<std::chrono::milliseconds>(ct2).time_since_epoch()
                          .count();
                //std::cout << "time consume is : " << t2 - t1 << "\n";

                cv::imshow("ouput", frame);
                cv::waitKey(1);
            }
            qApp->processEvents();
        }

    }

}

void MainWindow::on_pushButton_clicked()
{
    QFileDialog fileDialog;
    fileDialog.setWindowTitle(tr("Open Video File"));
    fileDialog.setDirectory(".");
    fileDialog.setNameFilter(tr("Video (*.*)"));
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setViewMode(QFileDialog::Detail);

    if (fileDialog.exec())
    {
        QString fileName;
        fileName=fileDialog.selectedFiles()[0];

        ui->lineEdit->setText(fileName);

    }
}

void MainWindow::on_pushButton_6_clicked()
{
    QFileDialog fileDialog;
    fileDialog.setWindowTitle(tr("Open Picture File"));
    fileDialog.setDirectory(".");
    fileDialog.setNameFilter(tr("Picture (*.*)"));
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setViewMode(QFileDialog::Detail);

    if (fileDialog.exec())
    {
        QString fileName;
        fileName=fileDialog.selectedFiles()[0];

        ui->lineEdit_2->setText(fileName);

    }
}

void MainWindow::on_pushButton_5_clicked()
{
    std::filesystem::path imgPath;

    try{
        imgPath = ui->lineEdit_2->text().toStdString();
    }catch(std::exception& e){
        qDebug()<<"Error:"<<QString::fromStdString(e.what());
    }

    if(!std::filesystem::exists(imgPath)){
        QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误："),QString::fromLocal8Bit("打开指定的图片文件失败！"),QMessageBox::Ok,this);
        message.exec();
        return;
    }

    int writer_fps;
    float downsample_ratio;

    cv::Scalar backgroundColor;
    cv::Size netInputSize(ui->spinBox_10->value(),ui->spinBox_11->value());

    if(ui->checkBox->isChecked()){
        writer_fps=0;
    }else{
        writer_fps = ui->spinBox_6->value();
    }

    downsample_ratio=ui->doubleSpinBox->value();

    backgroundColor=cv::Scalar(ui->spinBox_7->value(),ui->spinBox_8->value(),ui->spinBox_9->value());

    auto originImg = cv::imread(imgPath.string());

    mRobustVideoMatting->detect(originImg,mPictureContents,downsample_ratio,backgroundColor,false);

    cv::imwrite(imgPath.string()+".output.jpg",mPictureContents.merge_mat);

    QMessageBox message(QMessageBox::Information,QString::fromLocal8Bit("提示："),QString::fromLocal8Bit("处理完成！处理后的图片已写入原始图片相同目录下！"),QMessageBox::Ok,this);
    message.exec();

}

void MainWindow::on_pushButton_7_clicked()
{
    QString filePath;
    filePath=QFileDialog::getExistingDirectory(NULL,"Save Path",".");
    ui->lineEdit_3->setText(filePath);
}

void MainWindow::on_pushButton_8_clicked()
{
    QString filePath;
    filePath=QFileDialog::getExistingDirectory(NULL,"Save Path",".");
    ui->lineEdit_4->setText(filePath);
}

void MainWindow::on_actionExit_triggered()
{
    qApp->quit();
}

void MainWindow::on_pushButton_10_clicked()
{
    DevelopModeWindow developMode(this);

    developMode.exec();
}

void MainWindow::on_pushButton_9_clicked()
{
    getUISettings();

    if(configManager.writeConfigFile(defaultConfigFilePath, mConfig)){
        QMessageBox message(QMessageBox::Information,QString::fromLocal8Bit("提示："),QString::fromLocal8Bit("写入配置文件成功！"),QMessageBox::Ok,this);
        message.exec();
    }else{
        QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误："),QString::fromLocal8Bit("写入配置文件失败！"),QMessageBox::Ok,this);
        message.exec();
    }
}

void MainWindow::on_actionOpen_File_triggered()
{
    on_pushButton_clicked();
}

void MainWindow::on_action_3_triggered()
{
    on_pushButton_6_clicked();
}


void MainWindow::on_action_2_triggered()
{
    AboutDialog aboutDialog(this);

    aboutDialog.exec();
}


void MainWindow::on_actionHelp_triggered()
{
    HelpDialog helpDialog(this);

    helpDialog.exec();
}


void MainWindow::on_pushButton_11_clicked()
{
    mSettings.currentCameraProcessStatus = false;
}


void MainWindow::on_pushButton_12_clicked()
{
    mSettings.currentVideoProcessStatus = false;
}

