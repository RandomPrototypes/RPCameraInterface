#include <RPCameraInterface/CameraInterfaceScreenCaptureLite.h>

namespace RPCameraInterface
{

bool getWindowByName(std::string name, SL::Screen_Capture::Window *result) 
{
    auto windows = SL::Screen_Capture::GetWindows();
    for (size_t i = 0; i < windows.size(); i++) {
        if(windows[i].Name == name) {
            *result = windows[i];
            return true;
        }
    }
    return false;
}

CameraEnumeratorScreenCaptureLite::CameraEnumeratorScreenCaptureLite()
    :CameraEnumeratorBase(CaptureBackend::ScreenCaptureLite)
{
    cameraType = "Window capture";
}

CameraEnumeratorScreenCaptureLite::~CameraEnumeratorScreenCaptureLite()
{
}

bool CameraEnumeratorScreenCaptureLite::detectCameras()
{
    listCameras.clear();
    auto windows = SL::Screen_Capture::GetWindows();
    for (size_t i = 0; i < windows.size(); i++) {
        CameraInfo camInfo;
        camInfo.id = windows[i].Name;
        camInfo.name = windows[i].Name;
        listCameras.push_back(camInfo);
    }
    return true;
}

CameraInterfaceScreenCaptureLite::CameraInterfaceScreenCaptureLite()
    :CameraInterfaceBase(CaptureBackend::ScreenCaptureLite)
{
    frameGrabber = NULL;
    currentImage = NULL;
    maxBufferSize = 2;
    ROI_x = 0;
    ROI_y = 0;
    ROI_width = 0;
    ROI_height = 0;
}

CameraInterfaceScreenCaptureLite::~CameraInterfaceScreenCaptureLite()
{
}

bool CameraInterfaceScreenCaptureLite::open(const char *params)
{
    SL::Screen_Capture::Window win;
    if(!getWindowByName(params, &win))
        return false;
    winName = params;
    imageFormat.width = win.Size.x;
    imageFormat.height = win.Size.y;
    imageFormat.type = ImageType::BGR24;
    return true;
}

bool CameraInterfaceScreenCaptureLite::close()
{
    return true;
}

size_t CameraInterfaceScreenCaptureLite::getAvailableFormatCount()
{
	return 1;
}

ImageFormat CameraInterfaceScreenCaptureLite::getAvailableFormat(size_t id)
{
	return imageFormat;
}
	
void CameraInterfaceScreenCaptureLite::selectFormat(int formatId)
{
}
void CameraInterfaceScreenCaptureLite::selectFormat(ImageFormat format)
{
}
ImageData *CameraInterfaceScreenCaptureLite::getNewFramePtr(bool skipOldFrames)
{
    if(currentImage == NULL) {
        currentImage = createImageDataRawPtr();
        currentImage->setDataReleasedWhenDestroy(false);
        currentImage->setImageFormat(imageFormat);
        currentImage->allocData(imageFormat.width * imageFormat.height * 3);
        currentImage->setDataSize(imageFormat.width * imageFormat.height * 3);
    }
    mutex.lock();
    while(recentImages.size() == 0) {
        mutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        mutex.lock();
    }
    if(skipOldFrames && recentImages.size() > 1) {
        for(size_t i = 0; i + 1 < recentImages.size(); i++)
            memoryBuffer.push_back(recentImages[i]);
        recentImages.erase(recentImages.begin(), recentImages.begin() + recentImages.size() - 1);
    }
    //printf("recentImages: %d, memoryBuffer: %d\n", recentImages.size(), memoryBuffer.size());
    auto timestamp = recentImages[0]->getTimestamp();
    currentImage->setTimestamp(timestamp);
    memcpy(currentImage->getDataPtr(), recentImages[0]->getDataPtr(), currentImage->getDataSize());
    memoryBuffer.push_back(recentImages[0]);
    recentImages.erase(recentImages.begin());
    mutex.unlock();

    ImageData *resultImage = createImageDataRawPtr();
    resultImage->setDataReleasedWhenDestroy(false);
    resultImage->setImageFormat(imageFormat);
    resultImage->setTimestamp(currentImage->getTimestamp());
    resultImage->setDataPtr(currentImage->getDataPtr());
    resultImage->setDataSize(currentImage->getDataSize());
    return resultImage;
}
const char *CameraInterfaceScreenCaptureLite::getErrorMsg()
{
    return errorMsg.c_str();
}

bool CameraInterfaceScreenCaptureLite::startCapturing()
{
    frameGrabber =
        SL::Screen_Capture::CreateCaptureConfiguration([this]() {
            std::vector<SL::Screen_Capture::Window> filtereditems;
            SL::Screen_Capture::Window win;
            if(getWindowByName(winName, &win))
                filtereditems.push_back(win);
            return filtereditems;
        })->onNewFrame([&, this](const SL::Screen_Capture::Image &img, const SL::Screen_Capture::Window &window) {
            //printf("onNewFrame\n");
            auto timestamp = getTimestampMs();
            mutex.lock();
            ImageData *newImage = NULL;
            int roiX, roiY, roiW, roiH;
            getROI(&roiX, &roiY, &roiW, &roiH);
            if(memoryBuffer.size() == 0) {
                if(recentImages.size() < maxBufferSize) {
                    newImage = createImageDataRawPtr();
                    newImage->setDataReleasedWhenDestroy(false);
                } else {
                    newImage = recentImages[0];
                    recentImages.erase(recentImages.begin());
                }
            } else {
                newImage = memoryBuffer[memoryBuffer.size()-1];
                memoryBuffer.erase(memoryBuffer.begin() + memoryBuffer.size() - 1);
            }

            if(newImage->getImageFormat().width != roiW || newImage->getImageFormat().height != roiH) {
                if(newImage->getDataPtr() != NULL)
                    newImage->freeData();
                ImageFormat imageFormat2 = imageFormat;
                imageFormat2.width = roiW;
                imageFormat2.height = roiH;
                newImage->setImageFormat(imageFormat2);
                newImage->allocData(roiW * roiH * 3);
                newImage->setDataSize(roiW * roiH * 3);
            }
            newImage->setTimestamp(timestamp);
            auto imgsrc = StartSrc(img);
            for (int i = 0; i < roiY; i++)
                imgsrc = SL::Screen_Capture::GotoNextRow(img, imgsrc);
            int w = std::min(roiW, Width(img) - roiX);
            int h = std::min(roiH, Height(img) - roiY);
            for (int i = 0; i < h; i++) {
                auto startimgsrc = imgsrc + roiX;
                unsigned char *dst = newImage->getDataPtr() + i*roiW*3;
                for (int j = 0; j < w; j++) {
                    *dst++ = imgsrc->B;
                    *dst++ = imgsrc->G;
                    *dst++ = imgsrc->R;
                    imgsrc++;
                }
                if(w < imageFormat.width)
                    memset(dst, 0, (roiW - w) * 3);
                imgsrc = SL::Screen_Capture::GotoNextRow(img, startimgsrc);
            }
            if(h < roiH)
                memset(newImage->getDataPtr() + h*roiW*3, 0, (roiH - h) * roiW*3);
            recentImages.push_back(newImage);
            mutex.unlock();
        })->start_capturing();
    frameGrabber->setFrameChangeInterval(std::chrono::milliseconds(30));
    return true;
}
bool CameraInterfaceScreenCaptureLite::stopCapturing()
{
    frameGrabber->pause();
    frameGrabber = NULL;
    return true;
}

bool CameraInterfaceScreenCaptureLite::hasROICapability()
{
    return true;
}

bool CameraInterfaceScreenCaptureLite::setROI(int x, int y, int width, int height)
{
    ROI_x = std::max(x, 0);
    ROI_y = std::max(y, 0);
    ROI_width = std::max(0, std::min(x+width-ROI_x, imageFormat.width - ROI_x));
    ROI_height = std::max(0, std::min(y+height-ROI_y, imageFormat.height - ROI_y));
    return true;
}

void CameraInterfaceScreenCaptureLite::getROI(int *x, int *y, int *width, int *height) const
{
    if(ROI_width > 0 && ROI_height > 0) {
        *x = ROI_x;
        *y = ROI_y;
        *width = ROI_width;
        *height = ROI_height;
    } else {
        *x = 0;
        *y = 0;
        *width = imageFormat.width;
        *height = imageFormat.height;
    }
}

}
