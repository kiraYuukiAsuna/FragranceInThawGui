#pragma once

#include <string>
#include <opencv.hpp>

const std::string defaultConfigFilePath = "./config.json";

const std::string defaultRVMModelFilePath = "./Resource/Model/rvm_mobilenetv3_fp32.onnx";

constexpr int defaultNumThread = 12;

constexpr int defaultVideoOutputFPS = 20;

constexpr float defaultDownsample_ratio = 0.25;

const cv::Scalar defaultBackgroundColor(0,0,0);

const cv::Size defaultNetInputImgSize(960,540);


const std::string jnode_RVMModelFilePath = "RVMModelFilePath";
const std::string jnode_Num_Thread = "Num_Thread";
const std::string jnode_VideoOutputFPS = "VideoOutputFPS";
const std::string jnode_Downsample_ratio = "Downsample_ratio";
const std::string jnode_BackgroundColor_R = "BackgroundColor_R";
const std::string jnode_BackgroundColor_G = "BackgroundColor_G";
const std::string jnode_BackgroundColor_B = "BackgroundColor_B";
const std::string jnode_NetInputImgSize_Width = "NetInputImgSize_Width";
const std::string jnode_NetInputImgSize_Height = "NetInputImgSize_Height";
