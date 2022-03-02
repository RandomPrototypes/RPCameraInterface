#include "CameraInterface.h"
#include "ImageFormatConverter.h"
#include "CameraInterfaceDShow.h"
#include <opencv2/opencv.hpp>

using namespace RPCameraInterface;

int main()
{
    CameraEnumeratorDShow camEnum;
    camEnum.detectCameras();
    printf("%d cameras detected\n", camEnum.count());
    for(int i = 0; i < camEnum.count(); i++) {
        printf("%s: %s\n", camEnum.getCameraId(i).c_str(), camEnum.getCameraName(i).c_str());
    }

    CameraInterfaceDShow cam;
    cam.open("2");
    std::vector<ImageFormat> listFormats = cam.getAvailableFormats();
    for(ImageFormat& format : listFormats) {
        printf("%dx%d (%s)\n", format.width, format.height, toString(format.type).c_str());
    }
    cam.selectFormat(ImageFormat(ImageType::JPG, 1920, 1080));
    if(!cam.startCapturing())
        printf("error in startCapturing()\n");
    ImageFormat dstFormat(ImageType::BGR24, 720, 480);
    ImageFormatConverter converter(ImageFormat(ImageType::JPG, 1920, 1080), dstFormat);

    std::shared_ptr<ImageData> imgData2 = std::make_shared<ImageData>();

    while(true){
        std::shared_ptr<ImageData> imgData = cam.getNewFrame(true);
        converter.convertImage(imgData, imgData2);
        cv::Mat resultImg2(imgData2->imageFormat.height, imgData2->imageFormat.width, CV_8UC3, imgData2->data);
        cv::imshow("img", resultImg2);
        cv::waitKey(10);
    }

    /*CameraInterfaceOpenCV cam;
    if(!cam.open("2"))
        printf("can not open cam 0\n");
    std::vector<ImageFormat> listFormats = cam.getAvailableFormats();
    for(const ImageFormat& format : listFormats) {
        printf("%dx%d %s\n", format.width, format.height, format.type == ImageType::MJPG ? "mjpg" : "bgr");
    }
    cam.selectFormat(listFormats[listFormats.size()-1]);
    cam.startCapturing();
    while(true){
        std::shared_ptr<ImageData> imgData = cam.getNewFrame(true);
        cv::Mat img(imgData->imageFormat.height, imgData->imageFormat.width, CV_8UC3, imgData->data);
        cv::imshow("img", img);
        cv::waitKey(10);
    }*/
    return 0;
}
