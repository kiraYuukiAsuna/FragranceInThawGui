#pragma once

#include <opencv.hpp>
#include <vector>

class VideoProcessingBase
{
public:
    VideoProcessingBase();

    int enumCameraIndex(std::vector<std::string>& cameraIndexList);

};

class VideoProcessingFile : public VideoProcessingBase
{
private:
    cv::VideoCapture mVideoCapture;

public:

private:

public:
    VideoProcessingFile();
    ~VideoProcessingFile();

    bool openFile(std::string fileName);

    bool getFrame(cv::Mat& frame);

    cv::Size getFrameSize();

};

class VideoProcessingCamera : public VideoProcessingBase
{
private:
    cv::VideoCapture mVideoCapture;

public:

private:

public:
    VideoProcessingCamera();
    ~VideoProcessingCamera();

    bool openCamera(int cameraIndex);

    bool getFrame(cv::Mat& frame);

    cv::Size getFrameSize();

};
