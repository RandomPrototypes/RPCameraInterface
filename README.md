# RPCameraInterface

Portable (Windows and Linux, no Mac OS yet) and unified interface for cameras.

**Still a prototype, not all backends are fully implemented yet.**

Similar to OpenCV camera interface but with a few differences :
* Provides the list of available cameras
* Provides the list of available formats and resolutions
* Supports MJPG encoding (allows higher framerate than OpenCV)
* Easy formats conversion based on ffmpeg
* Support on-device recording with preview (only for Android phones currently, but plan to add Raspberry pi, Oak-D,... later)

### On-device recording
Some devices have capability to record videos directly. This can be useful to reduce the required bandwidth while keeping high-framerate and high-resolution recording.
Preview can be streamed at low-resolution.
This currently only includes Android phones, but more will be added later.

### Examples

Get the list of available cameras :
``` cpp
    std::shared_ptr<CameraEnumerator> camEnum = getCameraEnumerator();
    camEnum->detectCameras();
    printf("%d cameras detected\n", camEnum->count());
    for(int i = 0; i < camEnum->count(); i++) {
        printf("%s: %s\n", camEnum->getCameraId(i).c_str(), camEnum->getCameraName(i).c_str());
    }
```
Open the first camera :
``` cpp
    std::string camId = camEnum->getCameraId(0);
    std::shared_ptr<CameraInterface> cam = getCameraInterface(camEnum->backend);
    cam->open(camId);
```
Get the list of supported formats and resolutions :
``` cpp
    std::vector<ImageFormat> listFormats = cam->getAvailableFormats();
    for(ImageFormat& format : listFormats)
        printf("%dx%d (%s)\n", format.width, format.height, toString(format.type).c_str());
```
Select the format and start capturing :
``` cpp
    cam->selectFormat(ImageFormat(ImageType::JPG, 1920, 1080));
    if(!cam->startCapturing())
        printf("error in startCapturing()\n");
```
Capture one frame :
``` cpp
    std::shared_ptr<ImageData> imgData = cam->getNewFrame(true);
```
Convert image formats :
``` cpp
    ImageFormatConverter converter(ImageFormat(ImageType::JPG, 1920, 1080), ImageFormat(ImageType::BGR24, 720, 480));
    std::shared_ptr<ImageData> imgData2 = std::make_shared<ImageData>();
    converter.convertImage(imgData, imgData2);
```
Convert to OpenCV mat :
``` cpp
    cv::Mat resultImg2(imgData2->imageFormat.height, imgData2->imageFormat.width, CV_8UC3, imgData2->data);
    cv::imshow("img", resultImg2);
    cv::waitKey(10);
```
### License
Apache 2

### Credits
The DShow backend is based on code from OpenCV and a few functions from libwebcam
The V4L2 backend is based on V4L2 samples and the enumerator is based on code from libwebcam
