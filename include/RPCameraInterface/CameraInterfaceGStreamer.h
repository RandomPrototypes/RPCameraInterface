#pragma once

#include "CameraInterfaceBase.h"
#include <cassert>

#include <gst/gst.h>
#include <gst/gstbuffer.h>
#include <gst/video/video.h>
#include <gst/audio/audio.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/riff/riff-media.h>
#include <gst/pbutils/missing-plugins.h>

#define VERSION_NUM(major, minor, micro) (major * 1000000 + minor * 1000 + micro)
#define FULL_GST_VERSION VERSION_NUM(GST_VERSION_MAJOR, GST_VERSION_MINOR, GST_VERSION_MICRO)

#include <gst/pbutils/encoding-profile.h>
//#include <gst/base/gsttypefindhelper.h>

#include <map>
#include <sstream>

namespace RPCameraInterface
{


namespace {

#if defined __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunused-function"
#endif

template<typename T> static inline void GSafePtr_addref(T* ptr)
{
    if (ptr)
        g_object_ref_sink(ptr);
}

template<typename T> static inline void GSafePtr_release(T** pPtr);

template<> inline void GSafePtr_release<GError>(GError** pPtr) { g_clear_error(pPtr); }
template<> inline void GSafePtr_release<GstElement>(GstElement** pPtr) { if (pPtr) { gst_object_unref(G_OBJECT(*pPtr)); *pPtr = NULL; } }
template<> inline void GSafePtr_release<GstElementFactory>(GstElementFactory** pPtr) { if (pPtr) { gst_object_unref(G_OBJECT(*pPtr)); *pPtr = NULL; } }
template<> inline void GSafePtr_release<GstPad>(GstPad** pPtr) { if (pPtr) { gst_object_unref(G_OBJECT(*pPtr)); *pPtr = NULL; } }
template<> inline void GSafePtr_release<GstCaps>(GstCaps** pPtr) { if (pPtr) { gst_caps_unref(*pPtr); *pPtr = NULL; } }
template<> inline void GSafePtr_release<GstBuffer>(GstBuffer** pPtr) { if (pPtr) { gst_buffer_unref(*pPtr); *pPtr = NULL; } }
template<> inline void GSafePtr_release<GstSample>(GstSample** pPtr) { if (pPtr) { gst_sample_unref(*pPtr); *pPtr = NULL; } }
template<> inline void GSafePtr_release<GstBus>(GstBus** pPtr) { if (pPtr) { gst_object_unref(G_OBJECT(*pPtr)); *pPtr = NULL; } }
template<> inline void GSafePtr_release<GstMessage>(GstMessage** pPtr) { if (pPtr) { gst_message_unref(*pPtr); *pPtr = NULL; } }
template<> inline void GSafePtr_release<GMainLoop>(GMainLoop** pPtr) { if (pPtr) { g_main_loop_unref(*pPtr); *pPtr = NULL; } }

template<> inline void GSafePtr_release<GstEncodingVideoProfile>(GstEncodingVideoProfile** pPtr) { if (pPtr) { gst_encoding_profile_unref(*pPtr); *pPtr = NULL; } }
template<> inline void GSafePtr_release<GstEncodingContainerProfile>(GstEncodingContainerProfile** pPtr) { if (pPtr) { gst_object_unref(G_OBJECT(*pPtr)); *pPtr = NULL; } }

template<> inline void GSafePtr_addref<char>(char* pPtr);  // declaration only. not defined. should not be used
template<> inline void GSafePtr_release<char>(char** pPtr) { if (pPtr) { g_free(*pPtr); *pPtr = NULL; } }

#if defined __clang__
# pragma clang diagnostic pop
#endif

template <typename T>
class GSafePtr
{
protected:
    T* ptr;
public:
    inline GSafePtr() noexcept : ptr(NULL) { }
    inline ~GSafePtr() noexcept { release(); }
    inline void release() noexcept
    {
#if 0
        printf("release: %s:%d: %p\n", CV__TRACE_FUNCTION, __LINE__, ptr);
        if (ptr) {
            printf("    refcount: %d\n", (int)GST_OBJECT_REFCOUNT_VALUE(ptr)); \
        }
#endif
        if (ptr)
            GSafePtr_release<T>(&ptr);
    }

    inline operator T* () noexcept { return ptr; }
    inline operator /*const*/ T* () const noexcept { return (T*)ptr; }  // there is no const correctness in Gst C API

    T* get() { assert(ptr); return ptr; }
    /*const*/ T* get() const { assert(ptr); return (T*)ptr; }  // there is no const correctness in Gst C API

    const T* operator -> () const { assert(ptr); return ptr; }
    inline operator bool () const noexcept { return ptr != NULL; }
    inline bool operator ! () const noexcept { return ptr == NULL; }

    T** getRef() { assert(ptr == NULL); return &ptr; }

    inline GSafePtr& reset(T* p) noexcept // pass result of functions with "transfer floating" ownership
    {
        //printf("reset: %s:%d: %p\n", CV__TRACE_FUNCTION, __LINE__, p);
        release();
        if (p)
        {
            GSafePtr_addref<T>(p);
            ptr = p;
        }
        return *this;
    }

    inline GSafePtr& attach(T* p) noexcept  // pass result of functions with "transfer full" ownership
    {
        //printf("attach: %s:%d: %p\n", CV__TRACE_FUNCTION, __LINE__, p);
        release(); ptr = p; return *this;
    }
    inline T* detach() noexcept { T* p = ptr; ptr = NULL; return p; }

    inline void swap(GSafePtr& o) noexcept { std::swap(ptr, o.ptr); }
private:
    GSafePtr(const GSafePtr&); // = disabled
    GSafePtr& operator=(const T*); // = disabled
};

} // namespace

class CameraEnumeratorGStreamer : public CameraEnumeratorBase
{
public:
    CameraEnumeratorGStreamer();
    virtual ~CameraEnumeratorGStreamer();

    virtual bool detectCameras();
};

class CameraInterfaceGStreamer : public CameraInterfaceBase
{
public:
    CameraInterfaceGStreamer();
    ~CameraInterfaceGStreamer();
    virtual bool open(const char *params);
    virtual bool close();
    virtual void selectFormat(int formatId);
    virtual void selectFormat(ImageFormat format);
    virtual const char *getErrorMsg();

    virtual bool startCapturing();
    virtual bool stopCapturing();

protected:
	virtual ImageData *getNewFramePtr(bool skipOldFrames);
	virtual size_t getAvailableFormatCount();
	virtual ImageFormat getAvailableFormat(size_t id);

    bool isPipelinePlaying();
    void stopPipeline();
    void handleMessage(GstElement * pipeline);
private:
    std::vector<ImageFormat> listFormats;
    ImageFormat imageFormat;
    std::ostringstream errorMsg;
    std::string errorMsgStr;
    std::string cameraId;

    GSafePtr<GstElement> pipeline;
    GSafePtr<GstElement> camera_src;
    GSafePtr<GstElement> sink;
    GSafePtr<GstSample> sample;

    bool isCapturing;

    std::map<ImageType, std::string> listImageTypeStr;
};

}
