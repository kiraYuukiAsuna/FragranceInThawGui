#include "configmanager.h"
#include "src/ConstantDefination.h"
#include <fstream>
#include "Thirdparty/json/json.hpp"
#include <filesystem>
#include <QMessageBox>
#include <QString>

using namespace nlohmann;

ConfigManager::ConfigManager()
{

}

bool ConfigManager::createDefaultConfigFile(){
    std::fstream fstream;

    fstream.open(defaultConfigFilePath,std::ios::out|std::ios::ate);

    if(!fstream.is_open()){
        return false;
    }

    ordered_json j;

    j[jnode_RVMModelFilePath]=defaultRVMModelFilePath;
    j[jnode_Num_Thread]=defaultNumThread;
    j[jnode_VideoOutputFPS]=defaultVideoOutputFPS;
    j[jnode_Downsample_ratio]=defaultDownsample_ratio;
    j[jnode_BackgroundColor_R]=defaultBackgroundColor.val[0];
    j[jnode_BackgroundColor_G]=defaultBackgroundColor.val[1];
    j[jnode_BackgroundColor_B]=defaultBackgroundColor.val[2];
    j[jnode_NetInputImgSize_Width]=defaultNetInputImgSize.width;
    j[jnode_NetInputImgSize_Height]=defaultNetInputImgSize.height;

    fstream<<std::setw(4)<<j;

    fstream.close();

    return true;
}

bool ConfigManager::readConfigFile(const std::string configFilePath, ConfigFile& config){
    if(std::filesystem::exists(std::filesystem::path(defaultConfigFilePath))){
        std::fstream fstream;

        fstream.open(defaultConfigFilePath,std::ios::in);

        if(!fstream.is_open()){
            return false;
        }

        std::string s1;
        int i1,i2,i3,i4,i5,i6,i7;
        float f1;

        ordered_json j;

        fstream>>j;

        try{
            s1=j[jnode_RVMModelFilePath].get<std::string>();
            i1=j[jnode_Num_Thread].get<int>();
            i2=j[jnode_VideoOutputFPS].get<int>();
            f1=j[jnode_Downsample_ratio].get<float>();
            i3=j[jnode_BackgroundColor_R].get<int>();
            i4=j[jnode_BackgroundColor_G].get<int>();
            i5=j[jnode_BackgroundColor_B].get<int>();
            i6=j[jnode_NetInputImgSize_Width].get<int>();
            i7=j[jnode_NetInputImgSize_Height].get<int>();
        }catch(...){
            config.RVMModelFilePath=defaultConfigFilePath;
            config.VideoOutputFPS=defaultVideoOutputFPS;
            config.Downsample_ratio=defaultDownsample_ratio;
            config.BackgroundColor=cv::Scalar(defaultBackgroundColor.val[0],defaultBackgroundColor.val[1],defaultBackgroundColor.val[2]);
            config.NetInputImgSize=cv::Size(defaultNetInputImgSize.width,defaultNetInputImgSize.height);
            config.num_threads=defaultNumThread;

            if(!createDefaultConfigFile()){
                QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误："),QString::fromLocal8Bit("创建默认配置文件失败！"),QMessageBox::Ok,NULL);
                message.exec();
            }

            return false;
        }
        config.RVMModelFilePath=s1;
        config.VideoOutputFPS=i2;
        config.Downsample_ratio=f1;
        config.BackgroundColor=cv::Scalar(i3,i4,i5);
        config.NetInputImgSize=cv::Size(i6,i7);
        config.num_threads=i1;

        return true;
    }else{
        if(!createDefaultConfigFile()){
            QMessageBox message(QMessageBox::Warning,QString::fromLocal8Bit("错误："),QString::fromLocal8Bit("创建默认配置文件失败！"),QMessageBox::Ok,NULL);
            message.exec();
            return false;
        }
        config.RVMModelFilePath=defaultConfigFilePath;
        config.VideoOutputFPS=defaultVideoOutputFPS;
        config.Downsample_ratio=defaultDownsample_ratio;
        config.BackgroundColor=cv::Scalar(defaultBackgroundColor.val[0],defaultBackgroundColor.val[1],defaultBackgroundColor.val[2]);
        config.NetInputImgSize=cv::Size(defaultNetInputImgSize.width,defaultNetInputImgSize.height);
        config.num_threads=defaultNumThread;
        return true;
    }
}

bool ConfigManager::writeConfigFile(const std::string configFilePath, ConfigFile& config){
    if(std::filesystem::exists(std::filesystem::path(defaultConfigFilePath))){
        std::fstream fstream;

        fstream.open(defaultConfigFilePath,std::ios::out);

        if(!fstream.is_open()){
            return false;
        }

        ordered_json j;

        j[jnode_RVMModelFilePath]=config.RVMModelFilePath;
        j[jnode_Num_Thread]=config.num_threads;
        j[jnode_VideoOutputFPS]=config.VideoOutputFPS;
        j[jnode_Downsample_ratio]=config.Downsample_ratio;
        j[jnode_BackgroundColor_R]=config.BackgroundColor.val[0];
        j[jnode_BackgroundColor_G]=config.BackgroundColor.val[1];
        j[jnode_BackgroundColor_B]=config.BackgroundColor.val[2];
        j[jnode_NetInputImgSize_Width]=config.NetInputImgSize.width;
        j[jnode_NetInputImgSize_Height]=config.NetInputImgSize.height;

        fstream<<std::setw(4)<<j;

        fstream.close();

        return true;
    }else{
        return false;
    }
}
