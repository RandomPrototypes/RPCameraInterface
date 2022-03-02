#include "CameraInterfaceOpenCV.h"
#include <opencv2/opencv.hpp>

namespace RPCameraInterface
{

CameraInterfaceOpenCV::CameraInterfaceOpenCV()
{
    cap = NULL;
}

CameraInterfaceOpenCV::~CameraInterfaceOpenCV()
{
    if(cap != NULL) {
        delete cap;
        cap = NULL;
    }
}

bool CameraInterfaceOpenCV::open(std::string params)
{
    cap = new cv::VideoCapture(std::stoi(params), cv::CAP_DSHOW);
    if(!cap->isOpened()) {
        delete cap;
        cap = NULL;
        return false;
    }

    default_fourcc = cvRound(cap->get(cv::CAP_PROP_FOURCC));
    mjpg_fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');//0x47504A4D;
    imageFormat.width = cvRound(cap->get(cv::CAP_PROP_FRAME_WIDTH));
    imageFormat.height = cvRound(cap->get(cv::CAP_PROP_FRAME_HEIGHT));
    imageFormat.type = ImageType::BGR24;
    return true;
}

bool CameraInterfaceOpenCV::testResolution(int width, int height, bool use_mjpg)
{
    int fourcc = use_mjpg ? mjpg_fourcc : default_fourcc;
    if(use_mjpg) {
        cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('m', 'j', 'p', 'g'));
        cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
        //cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    } else {
        cap->set(cv::CAP_PROP_FOURCC, default_fourcc);
    }
    cap->set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap->set(cv::CAP_PROP_FRAME_HEIGHT, height);
    int result_fourcc = cvRound(cap->get(cv::CAP_PROP_FOURCC));
    int result_width = cvRound(cap->get(cv::CAP_PROP_FRAME_WIDTH));
    int result_height = cvRound(cap->get(cv::CAP_PROP_FRAME_HEIGHT));
    printf("%dx%d %d => %dx%d %d\n", width, height, fourcc, result_width, result_height, result_fourcc);
    return result_fourcc == fourcc && result_width == width && result_height == height;
}
bool CameraInterfaceOpenCV::close()
{
    delete cap;
    cap = NULL;
    return true;
}
std::vector<ImageFormat> CameraInterfaceOpenCV::getAvailableFormats()
{
    std::vector<cv::Size> listSizeToTest = {cv::Size(320,240), cv::Size(640,480), cv::Size(1280,720), cv::Size(1920,1080),
                                            cv::Size(3840,2160), cv::Size(4096,2160)};
    std::vector<ImageFormat> list;
    for(const cv::Size& size : listSizeToTest) {
        ImageFormat format;
        format.width = size.width;
        format.height = size.height;
        format.type = ImageType::BGR24;
        if(testResolution(size.width, size.height, false))
            list.push_back(format);
        format.type = ImageType::JPG;
        if(testResolution(size.width, size.height, true))
            list.push_back(format);
    }
    return list;
}
void CameraInterfaceOpenCV::selectFormat(int formatId)
{
    if(listFormats.size() == 0)
        listFormats = getAvailableFormats();
    if(formatId < listFormats.size())
        imageFormat = listFormats[formatId];
}
void CameraInterfaceOpenCV::selectFormat(ImageFormat format)
{
    imageFormat = format;
}
std::shared_ptr<ImageData> CameraInterfaceOpenCV::getNewFrame(bool skipOldFrames)
{
    if(cap != NULL)
    {
        *cap >> frame;
        std::shared_ptr<ImageData> data = std::make_shared<ImageData>();
        data->releaseDataWhenDestroy = false;
        data->timestamp = getTimestampMs();
        data->imageFormat.type = ImageType::BGR24;
        data->imageFormat.width = frame.cols;
        data->imageFormat.height = frame.rows;
        data->data = const_cast<unsigned char*>(frame.data);
        data->dataSize = frame.rows*frame.cols*3;
        return data;
    }
    return std::shared_ptr<ImageData>();
}
std::string CameraInterfaceOpenCV::getErrorMsg()
{
    return errorMsg;
}

bool CameraInterfaceOpenCV::startCapturing()
{
    if(cap != NULL) {
        if(imageFormat.type == ImageType::JPG) {
            cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('m', 'j', 'p', 'g'));
            cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
        } else {
            cap->set(cv::CAP_PROP_FOURCC, default_fourcc);
        }
        //cap->set(cv::CAP_PROP_FOURCC, imageFormat.type == ImageType::MJPG ? mjpg_fourcc : default_fourcc);
        cap->set(cv::CAP_PROP_FRAME_WIDTH, imageFormat.width);
        cap->set(cv::CAP_PROP_FRAME_HEIGHT, imageFormat.height);
    }
    return true;
}
bool CameraInterfaceOpenCV::stopCapturing()
{
    return true;
}

CameraInterfaceFactoryOpenCV::CameraInterfaceFactoryOpenCV()
{
}

CameraInterfaceFactoryOpenCV::~CameraInterfaceFactoryOpenCV()
{
}

CameraInterface *CameraInterfaceFactoryOpenCV::createInterface()
{
    return new CameraInterfaceOpenCV();
}

}
