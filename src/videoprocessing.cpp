#include "videoprocessing.h"
#include <windows.h>
#include <strmif.h>
#include <dshow.h>

#pragma comment(lib,"Strmiids.lib")

VideoProcessingBase::VideoProcessingBase()
{

}

int VideoProcessingBase::enumCameraIndex(std::vector<std::string>& cameraIndexList)
{
    ICreateDevEnum *pDevEnum = NULL;
    IEnumMoniker *pEnum = NULL;
    int deviceCounter = 0;
    CoInitialize(NULL);
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
                                  CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
                                  reinterpret_cast<void**>(&pDevEnum));


    if (SUCCEEDED(hr))
    {
        // Create an enumerator for the video capture category.
        hr = pDevEnum->CreateClassEnumerator(
                 CLSID_VideoInputDeviceCategory,
                 &pEnum, 0);

        if (hr == S_OK){

            //if (!silent)printf("SETUP: Looking For Capture Devices\n");
            IMoniker *pMoniker = NULL;

            while (pEnum->Next(1, &pMoniker, NULL) == S_OK){

                IPropertyBag *pPropBag;
                hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
                                             (void**)(&pPropBag));

                if (FAILED(hr)){
                    pMoniker->Release();
                    continue;  // Skip this one, maybe the next one will work.
                }


                // Find the description or friendly name.
                VARIANT varName;
                VariantInit(&varName);
                hr = pPropBag->Read(L"Description", &varName, 0);

                if (FAILED(hr)) hr = pPropBag->Read(L"FriendlyName", &varName, 0);

                if (SUCCEEDED(hr)){

                    hr = pPropBag->Read(L"FriendlyName", &varName, 0);

                    int count = 0;
                    char tmp[255] = {0};
                    //int maxLen = sizeof(deviceNames[0]) / sizeof(deviceNames[0][0]) - 2;
                    while (varName.bstrVal[count] != 0x00 && count < 255) {
                        tmp[count] = (char)varName.bstrVal[count];
                        count++;
                    }
                    cameraIndexList.push_back(tmp);
                    //deviceNames[deviceCounter][count] = 0;

                    //if (!silent)printf("SETUP: %i) %s \n", deviceCounter, deviceNames[deviceCounter]);
                }

                pPropBag->Release();
                pPropBag = NULL;

                pMoniker->Release();
                pMoniker = NULL;

                deviceCounter++;
            }

            pDevEnum->Release();
            pDevEnum = NULL;

            pEnum->Release();
            pEnum = NULL;
        }
    }

    return deviceCounter;
}

VideoProcessingFile::VideoProcessingFile()
{

}

VideoProcessingFile::~VideoProcessingFile()
{
    mVideoCapture.release();
}

bool VideoProcessingFile::openFile(std::string fileName)
{
    return mVideoCapture.open(fileName);
}

bool VideoProcessingFile::getFrame(cv::Mat& frame){
    if(!mVideoCapture.isOpened()){
        return false;
    }

    mVideoCapture.read(frame);

    if (frame.empty())
    {
        return false;
    }

    return true;
}

cv::Size VideoProcessingFile::getFrameSize(){
    const unsigned int width = mVideoCapture.get(cv::CAP_PROP_FRAME_WIDTH);
    const unsigned int height = mVideoCapture.get(cv::CAP_PROP_FRAME_HEIGHT);
    return cv::Size(width,height);
}

VideoProcessingCamera::VideoProcessingCamera()
{

}

VideoProcessingCamera::~VideoProcessingCamera()
{
    mVideoCapture.release();
}

bool VideoProcessingCamera::openCamera(int cameraIndex)
{
    return mVideoCapture.open(cameraIndex);
}

bool VideoProcessingCamera::getFrame(cv::Mat& frame){
    if(!mVideoCapture.isOpened()){
        return false;
    }

    mVideoCapture.read(frame);

    if (frame.empty())
    {
        return false;
    }

    return true;
}

cv::Size VideoProcessingCamera::getFrameSize(){
    const unsigned int width = mVideoCapture.get(cv::CAP_PROP_FRAME_WIDTH);
    const unsigned int height = mVideoCapture.get(cv::CAP_PROP_FRAME_HEIGHT);
    return cv::Size(width,height);
}
