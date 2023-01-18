#include <RPCameraInterface/CameraInterface.h>
#include <RPCameraInterface/ImageFormatConverter.h>
#include <RPCameraInterface/VideoEncoder.h>
#include <opencv2/opencv.hpp>

using namespace RPCameraInterface;

int main()
{
	int backendId = 0, cameraId = 0;
	

    std::vector<CaptureBackend> listBackends = getAvailableCaptureBackends();
    printf("available backends:\n");
    for(size_t i = 0; i < listBackends.size(); i++)
        printf("%d : %s\n", (int)i, getCameraEnumerator(listBackends[i])->getCameraType());
    scanf("%d", &backendId);
    getchar();
    
    if(backendId < 0 || backendId >= listBackends.size())
    	return 0;
    
    std::string cameraParam;
    if(listBackends[backendId] == CaptureBackend::OpenCV) {
        printf("enter camera parameter:\n");
        std::getline(std::cin, cameraParam);
    } else {
        //Obtain a camera enumerator using default backend
        std::shared_ptr<CameraEnumerator> camEnum = getCameraEnumerator(listBackends[backendId]);//CaptureBackend::GStreamer
        camEnum->detectCameras();
        printf("%s, %d cameras detected\n", camEnum->getCameraType(), camEnum->count());
        for(int i = 0; i < camEnum->count(); i++) {
            printf("%d : %s: %s\n", i, camEnum->getCameraId(i), camEnum->getCameraName(i));
        }

        if(camEnum->count() == 0)
            return 0;

        printf("which camera?\n");
        if(camEnum->count() > 1)
            scanf("%d", &cameraId);
        
        if(cameraId < 0 || cameraId >= camEnum->count())
            return 0;
        cameraParam = camEnum->getCameraId(cameraId);
    }

	//Obtain a camera interface using the same backend as the enumerator
    std::shared_ptr<CameraInterface> cam = getCameraInterface(listBackends[backendId]);
    //Open the camera using the id from the enumerator
    if(!cam->open(cameraParam.c_str())) {
        printf("error in cam->open(\"%s\") : %s\n", cameraParam.c_str(), cam->getErrorMsg());
        return 0;
    }
    printf("querying the available formats...\n");
    //Get the list of available formats
    std::vector<ImageFormat> listFormats = cam->getListAvailableFormat();
    std::vector<ImageFormat> listResolutions = getListResolution(listFormats);
    int resId = -1, type_id = -1;
    while(resId < 0 || resId >= listResolutions.size()) {
        for(size_t i = 0; i < listResolutions.size(); i++) {
            printf("%d: %dx%d\n", (int)i, listResolutions[i].width, listResolutions[i].height);
        }
        printf("which resolution? (0~%d)\n", ((int)listResolutions.size())-1);
        scanf("%d", &resId);
    }
    ImageFormat inputFormat = listResolutions[resId];
    std::vector<ImageType> listImageType = getListImageType(listFormats, inputFormat.width, inputFormat.height);
    while(type_id < 0 || type_id >= listImageType.size()) {
        for(size_t i = 0; i < listImageType.size(); i++) {
            printf("%d: %s\n",  (int)i, toString(listImageType[i]).c_str());
        }
        printf("which image type? (0~%d)\n", ((int)listImageType.size())-1);
        scanf("%d", &type_id);
    }
    inputFormat.type = listImageType[type_id];
    //Select the format
    cam->selectFormat(inputFormat);

    getchar();
    
    char filename[255];
    printf("enter the recording name (empty for no recording):");
    if(fgets(filename, 255, stdin) == NULL) {
        printf("invalid filename\n");
        if(!cam->close())
            printf("error in cam->close() : %s\n", cam->getErrorMsg());
        return 0;
    }

    int filename_size = 0;
    while(filename_size < 255 && filename[filename_size] != '\0' && filename[filename_size] != '\n')
        filename_size++;
    filename[filename_size] = '\0';

    std::shared_ptr<VideoEncoder> videoEncoder;

    FILE *mjpgRecordFile = NULL;

    char preset[255];
    
    if(filename_size > 4 && !strcmp(filename + filename_size - 4, ".mp4")) {
        int bitrate = 2000000;
        printf("bitrate (in kbps, recommended 500~8000):\n");
        scanf("%d", &bitrate);
        bitrate *= 1000;
        printf("preset (ultrafast,superfast,veryfast,faster,fast,medium,slow,...):\n");
        scanf("%255s", preset);
        getchar();
        videoEncoder = RPCameraInterface::createVideoEncoder();
        videoEncoder->open(filename, 720, 1280, 30, "", bitrate, preset);
    } else if((filename_size > 5 && !strcmp(filename + filename_size - 5, ".mjpg")) 
           || (filename_size > 6 && !strcmp(filename + filename_size - 6, ".mjpeg"))) 
    {
        if(inputFormat.type != ImageType::JPG) {
            printf("mjpg recording only supported if the camera output type is mjpg\n");
        } else {
            mjpgRecordFile = fopen(filename, "w"); 
        }
    }
    
    if(filename_size > 0 && videoEncoder == NULL && mjpgRecordFile == NULL) {
        printf("can not record to %s\n", filename);
        if(!cam->close())
            printf("error in cam->close() : %s\n", cam->getErrorMsg());
        return 0;
    }
    
    if(!cam->startCapturing())
        printf("error in cam->startCapturing() : %s\n", cam->getErrorMsg());
    //Choose the conversion format and initialize the converter
    ImageFormat dstFormat(ImageType::BGR24, 1280, 720);

    ImageFormatConverter converter(inputFormat, dstFormat);

    std::shared_ptr<ImageData> imgData2 = createImageData();

    int count = 0;
    double average_t01 = 0, average_t12 = 0, average_t23 = 0, average_t34 = 0;

    while(true){
    	//Obtain the frame
        auto t0 = std::chrono::system_clock::now();
        std::shared_ptr<ImageData> imgData = cam->getNewFrame(true);
        if(imgData == NULL) {
            printf("error in cam->getNewFrame : %s\n", cam->getErrorMsg());
            break;
        }

        auto t1 = std::chrono::system_clock::now();
        //Convert to the output format (BGR 720x480)
        converter.convertImage(imgData, imgData2);
        auto t2 = std::chrono::system_clock::now();
        if(videoEncoder != NULL) {
            videoEncoder->write(imgData2);
        } else if(mjpgRecordFile != NULL) {
            fwrite(imgData->getDataPtr(), imgData->getDataSize(), 1, mjpgRecordFile);
        }
        auto t3 = std::chrono::system_clock::now();
        //Create OpenCV Mat for visualization
        cv::Mat resultImg2(imgData2->getImageFormat().height, imgData2->getImageFormat().width, CV_8UC3, imgData2->getDataPtr());
        //cv::Mat resultImg2(imgData->getImageFormat().height, imgData->getImageFormat().width, CV_8UC1, imgData->getDataPtr());
        cv::imshow("img", resultImg2);
        if(cv::waitKey(5) > 0)
            break;
        auto t4 = std::chrono::system_clock::now();
        std::chrono::duration<double> t01 = t1 - t0;
        std::chrono::duration<double> t12 = t2 - t1;
        std::chrono::duration<double> t23 = t3 - t2;
        std::chrono::duration<double> t34 = t4 - t3;
        average_t01 = (count*average_t01 + 1000*t01.count()) / (count+1);
        average_t12 = (count*average_t12 + 1000*t12.count()) / (count+1);
        average_t23 = (count*average_t23 + 1000*t23.count()) / (count+1);
        average_t34 = (count*average_t34 + 1000*t34.count()) / (count+1);
        count++;
        printf("average time:get frame: %lf ms, convert to BGR: %lf ms,write to file: %lf ms, show %lf ms\n", average_t01, average_t12, average_t23, average_t34);
        
    }
    if(videoEncoder != NULL) {
        videoEncoder->release();
        videoEncoder = NULL;
    }
    if(mjpgRecordFile != NULL)
        fclose(mjpgRecordFile);
    
    if(!cam->stopCapturing())
        printf("error in cam->stopCapturing() : %s\n", cam->getErrorMsg());
    if(!cam->close())
        printf("error in cam->close() : %s\n", cam->getErrorMsg());
    
    cv::destroyAllWindows();

    printf("press any key to exit\n");
    getchar();

    return 0;
}