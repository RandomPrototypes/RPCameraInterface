/*M///////////////////////////////////////////////////////////////////////////////////////
//
// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Copyright (C) 2014, Itseez, Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
//M*/

#ifndef _CAP_DSHOW_HPP_
#define _CAP_DSHOW_HPP_

//Include Directshow stuff here so we don't worry about needing all the h files.
#define NO_DSHOW_STRSAFE
#include "dshow.h"
#include "strmif.h"
#include "aviriff.h"
#include "dvdmedia.h"
#include "bdaiface.h"

//for threading
#include <process.h>

//this is for TryEnterCriticalSection
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x400
#endif

#include <initguid.h>
#include <vector>
#include <string>

/*extern GUID MEDIASUBTYPE_GREY;
extern GUID MEDIASUBTYPE_Y8;
extern GUID MEDIASUBTYPE_Y800;
extern GUID MEDIASUBTYPE_Y16;
extern GUID MEDIASUBTYPE_BY8;

extern GUID CLSID_CaptureGraphBuilder2;
extern GUID CLSID_FilterGraph;
extern GUID CLSID_NullRenderer;
extern GUID CLSID_SampleGrabber;
extern GUID CLSID_SystemDeviceEnum;
extern GUID CLSID_VideoInputDeviceCategory;
extern GUID FORMAT_VideoInfo;
extern GUID IID_IAMAnalogVideoDecoder;
extern GUID IID_IAMCameraControl;
extern GUID IID_IAMCrossbar;
extern GUID IID_IAMStreamConfig;
extern GUID IID_IAMVideoProcAmp;
extern GUID IID_IBaseFilter;
extern GUID IID_ICaptureGraphBuilder2;
extern GUID IID_ICreateDevEnum;
extern GUID IID_IGraphBuilder;
extern GUID IID_IMPEG2PIDMap;
extern GUID IID_IMediaControl;
extern GUID IID_IMediaEventEx;
extern GUID IID_IMediaFilter;
extern GUID IID_ISampleGrabber;
extern GUID LOOK_UPSTREAM_ONLY;
extern GUID MEDIASUBTYPE_AYUV;
extern GUID MEDIASUBTYPE_IYUV;
extern GUID MEDIASUBTYPE_RGB24;
extern GUID MEDIASUBTYPE_RGB32;
extern GUID MEDIASUBTYPE_RGB555;
extern GUID MEDIASUBTYPE_RGB565;
extern GUID MEDIASUBTYPE_I420;
extern GUID MEDIASUBTYPE_UYVY;
extern GUID MEDIASUBTYPE_Y211;
extern GUID MEDIASUBTYPE_Y411;
extern GUID MEDIASUBTYPE_Y41P;
extern GUID MEDIASUBTYPE_YUY2;
extern GUID MEDIASUBTYPE_YUYV;
extern GUID MEDIASUBTYPE_YV12;
extern GUID MEDIASUBTYPE_YVU9;
extern GUID MEDIASUBTYPE_YVYU;
extern GUID MEDIASUBTYPE_MJPG; // MGB
extern GUID MEDIATYPE_Interleaved;
extern GUID MEDIATYPE_Video;
extern GUID PIN_CATEGORY_CAPTURE;
extern GUID PIN_CATEGORY_PREVIEW;*/

namespace RPCameraInterface
{

class videoDevice;

//videoInput defines
#define VI_VERSION      0.1995
#define VI_MAX_CAMERAS  20
#define VI_NUM_TYPES    22 //MGB
#define VI_NUM_FORMATS  18 //DON'T TOUCH

class videoFormat
{
public:
    std::string format;
    int fourcc;
    int width, height;

    videoFormat(const std::string& format, int fourcc, int width, int height)
        :format(format), fourcc(fourcc), width(width), height(height)
    {
    }
};
//////////////////////////////////////   VIDEO INPUT   /////////////////////////////////////
class videoInput{

    public:
        videoInput();
        ~videoInput();

        //Functions in rough order they should be used.
        static int listDevices(bool silent = false);

        //needs to be called after listDevices - otherwise returns NULL
        static char * getDeviceName(int deviceID);

        //choose to use callback based capture - or single threaded
        void setUseCallback(bool useCallback);

        //call before setupDevice
        //directshow will try and get the closest possible framerate to what is requested
        void setIdealFramerate(int deviceID, int idealFramerate);

        //some devices will stop delivering frames after a while - this method gives you the option to try and reconnect
        //to a device if videoInput detects that a device has stopped delivering frames.
        //you MUST CALL isFrameNew every app loop for this to have any effect
        void setAutoReconnectOnFreeze(int deviceNumber, bool doReconnect, int numMissedFramesBeforeReconnect);

        //Choose one of these five to setup your device
        bool setupDevice(int deviceID);
        bool setupDevice(int deviceID, int w, int h);
        bool setupDeviceFourcc(int deviceID, int w, int h,int fourcc);

        //These two are only for capture cards
        //USB and Firewire cameras shouldn't specify connection
        bool setupDevice(int deviceID, int connection);
        bool setupDevice(int deviceID, int w, int h, int connection);

        bool setFourcc(int deviceNumber, int fourcc);

        //call before setupDevice
        std::vector<videoFormat> getVideoFormats(int deviceID);

        //If you need to you can set your NTSC/PAL/SECAM
        //preference here. if it is available it will be used.
        //see #defines above for available formats - eg VI_NTSC_M or VI_PAL_B
        //should be called after setupDevice
        //can be called multiple times
        bool setFormat(int deviceNumber, int format);

        //Tells you when a new frame has arrived - you should call this if you have specified setAutoReconnectOnFreeze to true
        bool isFrameNew(int deviceID);

        bool isDeviceSetup(int deviceID) const;

        //Returns the pixels - flipRedAndBlue toggles RGB/BGR flipping - and you can flip the image too
        unsigned char * getPixels(int deviceID, bool flipRedAndBlue = true, bool flipImage = false);

        unsigned char *getRawPixels(int id, int *size);

        //Or pass in a buffer for getPixels to fill returns true if successful.
        bool getPixels(int id, unsigned char * pixels, bool flipRedAndBlue = true, bool flipImage = false);

        //Launches a pop up settings window
        //For some reason in GLUT you have to call it twice each time.
        bool showSettingsWindow(int deviceID);

        //Manual control over settings thanks.....
        //These are experimental for now.
        bool setVideoSettingFilter(int deviceID, long Property, long lValue, long Flags = 0, bool useDefaultValue = false);
        bool setVideoSettingFilterPct(int deviceID, long Property, float pctValue, long Flags = 0);
        bool getVideoSettingFilter(int deviceID, long Property, long &min, long &max, long &SteppingDelta, long &currentValue, long &flags, long &defaultValue);

        bool setVideoSettingCamera(int deviceID, long Property, long lValue, long Flags = 0, bool useDefaultValue = false);
        bool setVideoSettingCameraPct(int deviceID, long Property, float pctValue, long Flags = 0);
        bool getVideoSettingCamera(int deviceID, long Property, long &min, long &max, long &SteppingDelta, long &currentValue, long &flags, long &defaultValue);

        //bool setVideoSettingCam(int deviceID, long Property, long lValue, long Flags = NULL, bool useDefaultValue = false);

        //get width, height and number of pixels
        int  getWidth(int deviceID) const;
        int  getHeight(int deviceID) const;
        int  getSize(int deviceID) const;
        int  getFourcc(int deviceID) const;
        double getFPS(int deviceID) const;

        int getChannel(int deviceID) const;

        // RGB conversion setting
        bool getConvertRGB(int deviceID);
        bool setConvertRGB(int deviceID, bool enable);

        //completely stops and frees a device
        void stopDevice(int deviceID);

        //as above but then sets it up with same settings
        bool restartDevice(int deviceID);

        //number of devices available
        int  devicesFound;

        // mapping from OpenCV CV_CAP_PROP to videoinput/dshow properties
        int getVideoPropertyFromCV(int cv_property);
        int getCameraPropertyFromCV(int cv_property);

        bool isDeviceDisconnected(int deviceID);

        int property_window_count(int device_idx);

        GUID getMediasubtype(int deviceID);

    private:
        void setPhyCon(int deviceID, int conn);
        void setAttemptCaptureSize(int deviceID, int w, int h,GUID mediaType=MEDIASUBTYPE_RGB24);
        bool setup(int deviceID);
        void processPixels(unsigned char * src, unsigned char * dst, int width, int height, bool bRGB, bool bFlip, int bytesperpixel = 3);
        int  start(int deviceID, videoDevice * VD);
        int  getDeviceCount();
        void getMediaSubtypeAsString(GUID type, char * typeAsString);
        GUID *getMediaSubtypeFromFourcc(int fourcc);
        int   getFourccFromMediaSubtype(GUID type) const;

        void getVideoPropertyAsString(int prop, char * propertyAsString);
        void getCameraPropertyAsString(int prop, char * propertyAsString);

        HRESULT getDevice(IBaseFilter **pSrcFilter, int deviceID, WCHAR * wDeviceName, char * nDeviceName);
        static HRESULT ShowFilterPropertyPages(IBaseFilter *pFilter);
        static HRESULT ShowStreamPropertyPages(IAMStreamConfig  *pStream);

        HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath);
        HRESULT routeCrossbar(ICaptureGraphBuilder2 **ppBuild, IBaseFilter **pVidInFilter, int conType, GUID captureMode);

        //don't touch
        static bool comInit();
        static bool comUnInit();

        int  connection;
        int  callbackSetCount;
        bool bCallback;

        GUID CAPTURE_MODE;

        //Extra video subtypes
        // GUID MEDIASUBTYPE_Y800;
        // GUID MEDIASUBTYPE_Y8;
        // GUID MEDIASUBTYPE_GREY;

        videoDevice * VDList[VI_MAX_CAMERAS];
        GUID mediaSubtypes[VI_NUM_TYPES];
        long formatTypes[VI_NUM_FORMATS];

        static void __cdecl basicThread(void * objPtr);

        static char deviceNames[VI_MAX_CAMERAS][255];
};

/*namespace cv
{

class VideoCapture_DShow : public IVideoCapture
{
public:
    VideoCapture_DShow(int index);
    virtual ~VideoCapture_DShow();

    virtual double getProperty(int propIdx) const CV_OVERRIDE;
    virtual bool setProperty(int propIdx, double propVal) CV_OVERRIDE;

    virtual bool grabFrame() CV_OVERRIDE;
    virtual bool retrieveFrame(int outputType, OutputArray frame) CV_OVERRIDE;
    virtual int getCaptureDomain() CV_OVERRIDE;
    virtual bool isOpened() const;
protected:
    void open(int index);
    void close();

    int m_index, m_width, m_height, m_fourcc;
    int m_widthSet, m_heightSet;
    bool m_convertRGBSet;
    static videoInput g_VI;
};

}*/

}

#endif //_CAP_DSHOW_HPP_
