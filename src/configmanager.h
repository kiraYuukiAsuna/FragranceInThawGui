#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <string>
#include <opencv.hpp>

struct ConfigFile{
    std::string RVMModelFilePath;

    unsigned int num_threads;

    int VideoOutputFPS;

    float Downsample_ratio;

    cv::Scalar BackgroundColor;

    cv::Size NetInputImgSize;

};

class ConfigManager
{
public:
    ConfigManager();

public:
    bool createDefaultConfigFile();

    bool readConfigFile(const std::string configFilePath, ConfigFile& config);

    bool writeConfigFile(const std::string configFilePath, ConfigFile& config);

};

#endif // CONFIGMANAGER_H
