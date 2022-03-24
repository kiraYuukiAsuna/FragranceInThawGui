#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <opencv.hpp>
#include "src/RVMInvoke/RVMInvoke.h"
#include "src/configmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Settings{
    bool savePictureContents = false;
    bool saveCameraContents = false;
    bool saveVideoContents = false;

    bool currentCameraProcessStatus = false;
    bool currentVideoProcessStatus = false;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void refreshCameraDevice();

    RobustVideoMatting* safeRVMInvokeAccess();

    void setUISettings();
    void getUISettings();

private:
    RobustVideoMatting* mRobustVideoMatting;

    MattingContent mPictureContents;
    std::vector<MattingContent> mCameraContents;
    std::vector<MattingContent> mVideoContents;

    Settings mSettings;

    ConfigFile mConfig;

    ConfigManager configManager;

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_actionExit_triggered();

    void on_pushButton_10_clicked();

    void on_pushButton_9_clicked();

    void on_actionOpen_File_triggered();

    void on_action_3_triggered();

    void on_action_2_triggered();

    void on_actionHelp_triggered();

    void on_pushButton_11_clicked();

    void on_pushButton_12_clicked();

private:
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW_H
