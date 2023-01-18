#pragma once

#include "CameraInterface.h"

namespace RPCameraInterface
{

class CameraInfo
{
public:
    std::string id;
    std::string name;
    std::string description;
};

class CameraEnumeratorFieldBase : public CameraEnumeratorField
{
public:
	CameraEnumeratorFieldBase(const std::string& name, const std::string& type, const std::string& text, const std::string& value = "");
	virtual ~CameraEnumeratorFieldBase();
	
	virtual const char *getName();
	virtual const char *getType();
	virtual const char *getText();
	virtual const char *getValue();
	virtual void *getExtraParam();
	
	virtual void setName(const char *name);
	virtual void setType(const char *type);
	virtual void setText(const char *text);
	virtual void setValue(const char *value);
	virtual void setExtraParam(void *param);
	
	std::string name, type, text, value;
	void *extraParam;
};

class CameraEnumeratorBase : public CameraEnumerator
{
public:
	CameraEnumeratorBase(CaptureBackend backend);
    virtual ~CameraEnumeratorBase();

    virtual bool detectCameras() = 0;
    virtual const char *getCameraId(int index);
    virtual const char *getCameraName(int index);
    virtual const char *getCameraDescription(int index);
    virtual int getCameraIndex(const char *id);
    virtual const char *getCameraName(const char *id);
    virtual const char *getCameraDescription(const char *id);
    virtual int count();
    virtual CaptureBackend getBackend();
    virtual const char *getCameraType();
    
        
    virtual int getNbParamField();
    virtual CameraEnumeratorField *getParamField(int id); 
    
    CaptureBackend backend;
    std::string cameraType;
    std::vector<CameraInfo> listCameras;
    std::vector<CameraEnumeratorFieldBase*> listParamField;
};

class CameraInterfaceBase : public CameraInterface
{
public:
    CameraInterfaceBase(CaptureBackend backend);
    virtual ~CameraInterfaceBase();
    virtual bool open(const char *params) = 0;
    virtual bool close() = 0;
    virtual bool startCapturing() = 0;
    virtual bool stopCapturing() = 0;
    virtual bool hasRecordingCapability();
    virtual bool hasROICapability();
    virtual bool startRecording();
    virtual bool stopRecordingAndSaveToFile(const char *videoFilename, const char *timestampFilename);
    virtual void selectFormat(int formatId);
    virtual void selectVideoContainer(int containerId);
    virtual void selectVideoCodec(int codecId);
    virtual void selectFormat(ImageFormat format);
    virtual void selectVideoContainer(VideoContainerType container);
    virtual void selectVideoCodec(VideoCodecType codec);
    virtual bool setROI(int x, int y, int width, int height);
    virtual void getROI(int *x, int *y, int *width, int *height) const;
    virtual const char *getErrorMsg();
    virtual CaptureBackend getBackend();

protected:
	virtual ImageData *getNewFramePtr(bool skipOldFrames);
	virtual size_t getAvailableFormatCount();
	virtual ImageFormat getAvailableFormat(size_t id);
	virtual size_t getAvailableVideoCodecCount();
	virtual VideoCodecType getAvailableVideoCodec(size_t id);
    virtual size_t getAvailableVideoContainerCount();
    virtual VideoContainerType getAvailableVideoContainer(size_t id);
    
    CaptureBackend backend;
};

}//RPCameraInterface
