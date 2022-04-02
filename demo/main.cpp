#include <RPCameraInterface/CameraInterface.h>
#include <RPCameraInterface/ImageFormatConverter.h>
#include <opencv2/opencv.hpp>

using namespace RPCameraInterface;

int main()
{
	int cameraId = 0;
	
	//Obtain a camera enumerator using default backend
    std::shared_ptr<CameraEnumerator> camEnum = getCameraEnumerator();
    camEnum->detectCameras();
    printf("%d cameras detected\n", camEnum->count());
    for(int i = 0; i < camEnum->count(); i++) {
        printf("%s: %s\n", camEnum->getCameraId(i), camEnum->getCameraName(i));
    }
    
    if(cameraId < 0 || cameraId >= camEnum->count())
    	return 0;

	//Obtain a camera interface using the same backend as the enumerator
    std::shared_ptr<CameraInterface> cam = getCameraInterface(camEnum->getBackend());
    //Open the camera using the id from the enumerator
    cam->open(camEnum->getCameraId(cameraId));
    //Get the list of available formats
    std::vector<ImageFormat> listFormats = cam->getListAvailableFormat();
    for(ImageFormat& format : listFormats) {
        printf("%dx%d (%s)\n", format.width, format.height, toString(format.type).c_str());
    }
    //Select the format
    cam->selectFormat(ImageFormat(ImageType::JPG, 1920, 1080));
    if(!cam->startCapturing())
        printf("error in startCapturing()\n");
    //Choose the conversion format and initialize the converter
    ImageFormat dstFormat(ImageType::BGR24, 720, 480);
    ImageFormatConverter converter(ImageFormat(ImageType::JPG, 1920, 1080), dstFormat);

    std::shared_ptr<ImageData> imgData2 = createImageData();

    while(true){
    	//Obtain the frame
        std::shared_ptr<ImageData> imgData = cam->getNewFrame(true);
        //Conver to the output format (BGR 720x480)
        converter.convertImage(imgData, imgData2);
        //Create OpenCV Mat for visualization
        cv::Mat resultImg2(imgData2->getImageFormat().height, imgData2->getImageFormat().width, CV_8UC3, imgData2->getDataPtr());
        cv::imshow("img", resultImg2);
        cv::waitKey(10);
    }

    return 0;
}
