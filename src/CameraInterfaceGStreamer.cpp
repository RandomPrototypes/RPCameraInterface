/*
 * 
 * Based on cap_gstreamer.cpp from OpenCV, modified to fit RPCameraInterface and add device enumeration
 * Original license below :
 */


/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2008, 2011, Nils Hasler, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

/*!
 * \file cap_gstreamer.cpp
 * \author Nils Hasler <hasler@mpi-inf.mpg.de>
 *         Max-Planck-Institut Informatik
 * \author Dirk Van Haerenborgh <vhdirk@gmail.com>
 *
 * \brief Use GStreamer to read/write video
 */



#include <RPCameraInterface/CameraInterfaceGStreamer.h>

#include <thread>
#include <sstream>

namespace RPCameraInterface
{

bool gstreamer_config_call_deinit = false;
bool gstreamer_config_start_loop = false;

std::string escapeCameraPath(std::string path)
{
    std::string result;
    for(size_t i = 0; i < path.size(); i++)
    {
        if(path[i] == '\\')
            result += "\\\\";
        else result += path[i];
    }
    return result;
}

std::string getGStreamerCameraSource()
{
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        return "mfvideosrc";
    #elif __APPLE__
        return "avfvideosrc";
    #elif __linux__
        return "v4l2src";
    #else
        #error unknown platform
    #endif
}

class ScopeGuardGstMapInfo
{
    GstBuffer* buf_;
    GstMapInfo* info_;
public:
    ScopeGuardGstMapInfo(GstBuffer* buf, GstMapInfo* info)
        : buf_(buf), info_(info)
    {}
    ~ScopeGuardGstMapInfo()
    {
        gst_buffer_unmap(buf_, info_);
    }
};

class ScopeGuardGstVideoFrame
{
    GstVideoFrame* frame_;
public:
    ScopeGuardGstVideoFrame(GstVideoFrame* frame)
        : frame_(frame)
    {}
    ~ScopeGuardGstVideoFrame()
    {
        gst_video_frame_unmap(frame_);
    }
};


/*!
 * \brief The gst_initializer class
 * Initializes gstreamer once in the whole process
 */
class gst_initializer
{
public:
    static gst_initializer& init()
    {
        static gst_initializer g_init;
        if (g_init.isFailed)
            printf("Can't initialize GStreamer");
        return g_init;
    }
private:
    bool isFailed;
    bool call_deinit;
    bool start_loop;
    GSafePtr<GMainLoop> loop;
    std::thread thread;

    gst_initializer() :
        isFailed(false)
    {
        call_deinit = gstreamer_config_call_deinit;
        start_loop = gstreamer_config_start_loop;

        GSafePtr<GError> err;
        gboolean gst_init_res = gst_init_check(NULL, NULL, err.getRef());
        if (!gst_init_res)
        {
            printf("Can't initialize GStreamer: %s\n", (err ? err->message : "<unknown reason>"));
            isFailed = true;
            return;
        }
        guint major, minor, micro, nano;
        gst_version(&major, &minor, &micro, &nano);
        if (GST_VERSION_MAJOR != major)
        {
            printf("incompatible GStreamer version\n");
            isFailed = true;
            return;
        }

        if (start_loop)
        {
            loop.attach(g_main_loop_new (NULL, FALSE));
            thread = std::thread([this](){
                g_main_loop_run (loop);
            });
        }
    }
    ~gst_initializer()
    {
        if (call_deinit)
        {
            // Debug leaks: GST_LEAKS_TRACER_STACK_TRACE=1 GST_DEBUG="GST_TRACER:7" GST_TRACERS="leaks"
            gst_deinit();
        }

        if (start_loop)
        {
            g_main_loop_quit(loop);
            thread.join();
        }
    }
};

CameraEnumeratorGStreamer::CameraEnumeratorGStreamer()
    :CameraEnumeratorBase(CaptureBackend::GStreamer)
{
    cameraType = "GStreamer camera";
    gst_initializer::init();
}

CameraEnumeratorGStreamer::~CameraEnumeratorGStreamer()
{
}

static gboolean my_bus_func (GstBus * bus, GstMessage * message, gpointer user_data)
{
    GstDevice *device;
    gchar *name;

    switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_DEVICE_ADDED:
        gst_message_parse_device_added (message, &device);
        name = gst_device_get_display_name (device);
        g_print("Device added: %s\n", name);
        g_free (name);
        gst_object_unref (device);
        break;
    case GST_MESSAGE_DEVICE_REMOVED:
        gst_message_parse_device_removed (message, &device);
        name = gst_device_get_display_name (device);
        g_print("Device removed: %s\n", name);
        g_free (name);
        gst_object_unref (device);
        break;
    default:
        break;
    }

    return G_SOURCE_CONTINUE;
}

bool CameraEnumeratorGStreamer::detectCameras()
{
    listCameras.clear();

    GstDeviceMonitor* monitor = gst_device_monitor_new();
    /*GstBus *bus = gst_device_monitor_get_bus (monitor);
    gst_bus_add_watch (bus, my_bus_func, NULL);
    gst_object_unref (bus);

    GstCaps *caps = gst_caps_new_empty_simple ("video/x-raw");
    gst_device_monitor_add_filter (monitor, "Video/Source", caps);
    gst_caps_unref (caps);*/

    if(!gst_device_monitor_start(monitor)){
        printf("WARNING: Monitor couldn't started!!\n");
    }

    GList* devices = gst_device_monitor_get_devices(monitor);

    for(size_t i = 0; i < g_list_length(devices); i++) {
        GstDevice *device = (GstDevice*)g_list_nth_data(devices, i);
        gchar *device_class = gst_device_get_device_class(device);
        if (std::string(device_class) == "Video/Source" || std::string(device_class) == "Source/Video") {
            gchar *device_name = gst_device_get_display_name(device);
            GstStructure *properties = gst_device_get_properties(device);
            if (gst_structure_has_field(properties, "device.path")) {
                const gchar *path = gst_structure_get_string(properties, "device.path");
                CameraInfo camInfo;
                camInfo.id = std::string(path);
                camInfo.name = std::string(device_name);
                camInfo.description = std::string(device_name);
                listCameras.push_back(camInfo);
            }
            g_free(device_name);
		    gst_structure_free(properties);
        }
        g_free(device_class);
		gst_object_unref(device);
    }
    g_list_free(devices);
	gst_device_monitor_stop(monitor);
    
    return true;
}

CameraInterfaceGStreamer::CameraInterfaceGStreamer()
    :CameraInterfaceBase(CaptureBackend::GStreamer)
{
    gst_initializer::init();
    isCapturing = false;
}

CameraInterfaceGStreamer::~CameraInterfaceGStreamer()
{
    if(isCapturing)
        stopCapturing();
}

GstDevice *getDeviceById(GList* devices, const char *id)
{
    for(size_t i = 0; i < g_list_length(devices); i++) {
        GstDevice *device = (GstDevice*)g_list_nth_data(devices, i);
        gchar *device_class = gst_device_get_device_class(device);
        if (std::string(device_class) == "Video/Source" || std::string(device_class) == "Source/Video") {
            GstStructure *properties = gst_device_get_properties(device);
            if (gst_structure_has_field(properties, "device.path")) {
                const gchar *path = gst_structure_get_string(properties, "device.path");
                if(std::string(path) == std::string(id)) {
		            gst_structure_free(properties);
                    g_free(device_class);
                    return device;
                }
            }
		    gst_structure_free(properties);
        }
        g_free(device_class);
		gst_object_unref(device);
    }
    return NULL;
}

bool CameraInterfaceGStreamer::open(const char *params)
{
    listFormats.clear();

    GstDeviceMonitor* monitor = gst_device_monitor_new();
    if(!gst_device_monitor_start(monitor)){
        errorMsg.clear();
        errorMsg << "WARNING: Monitor couldn't started!!\n";
    }

    GList* devices = gst_device_monitor_get_devices(monitor);

    GstDevice *device = getDeviceById(devices, params);
    if(device == NULL) {
        g_list_free(devices);
	    gst_device_monitor_stop(monitor);   
        return false;
    }
    cameraId = params;

    GstCaps *caps = gst_device_get_caps(device);
    for(size_t i = 0; i < gst_caps_get_size(caps); i++) {
        ImageFormat format;
        GstStructure *cap = gst_caps_get_structure(caps, i);
        const gchar *cap_name = gst_structure_get_name(cap);
        if(!strcmp(cap_name, "video/x-raw")) {
            if(!gst_structure_has_field(cap, "format"))
                continue;
            std::string formatStr = gst_structure_get_string(cap, "format");
            format.type = toImageType(formatStr.c_str());
            if(format.type == ImageType::UNKNOWN)
                continue;
            if(listImageTypeStr.find(format.type) == listImageTypeStr.end())
                listImageTypeStr.insert(std::make_pair(format.type, formatStr));
        } else if(!strcmp(cap_name, "image/jpeg")) {
            format.type = ImageType::JPG;
        } else {
            continue;
        }
        if(!gst_structure_get_int(cap, "width", &(format.width)) || !gst_structure_get_int(cap, "height", &(format.height)))
            continue;
        gint num, den;
        if(gst_structure_get_fraction (cap, "framerate", &num, &den))
            format.fps = (int)(num/den);
        listFormats.push_back(format);
    }

    gst_caps_unref(caps);


    g_list_free(devices);
	gst_device_monitor_stop(monitor);    
    return true;
}

bool CameraInterfaceGStreamer::close()
{
    if(isCapturing)
        stopCapturing();
    return true;
}

size_t CameraInterfaceGStreamer::getAvailableFormatCount()
{
	return listFormats.size();
}

ImageFormat CameraInterfaceGStreamer::getAvailableFormat(size_t id)
{
	if(id < listFormats.size())
		return listFormats[id];
	return ImageFormat();
}
	
void CameraInterfaceGStreamer::selectFormat(int formatId)
{
    if(formatId < listFormats.size())
        imageFormat = listFormats[formatId];
}
void CameraInterfaceGStreamer::selectFormat(ImageFormat format)
{
    imageFormat = format;
}

bool CameraInterfaceGStreamer::isPipelinePlaying()
{
    if (!pipeline || !GST_IS_ELEMENT(pipeline.get()))
    {
        errorMsg.clear();
        errorMsg << "GStreamer: pipeline have not been created\n";
        return false;
    }
    GstState current, pending;
    GstClockTime timeout = 5*GST_SECOND;
    GstStateChangeReturn ret = gst_element_get_state(pipeline, &current, &pending, timeout);
    if (!ret)
    {
        errorMsg.clear();
        errorMsg << "unable to query pipeline state\n";
        return false;
    }
    return current == GST_STATE_PLAYING;
}

ImageData *CameraInterfaceGStreamer::getNewFramePtr(bool skipOldFrames)
{
    // start the pipeline if it was not in playing state yet
    if (!isPipelinePlaying()) {
      //  this->startPipeline();
      return NULL;
    }

    // bail out if EOS
    if (gst_app_sink_is_eos(GST_APP_SINK(sink.get()))) {
        errorMsg.clear();
        errorMsg << "GStreamer: app sink is eos\n";
        return NULL;
    }

    sample.attach(gst_app_sink_pull_sample(GST_APP_SINK(sink.get())));
    if (!sample) {
        errorMsg.clear();
        errorMsg << "GStreamer: can not gst_app_sink_pull_sample\n";
        return NULL;
    }

    //if (isPosFramesEmulated)
      //  emulatedFrameNumber++;







    GstCaps* frame_caps = gst_sample_get_caps(sample);  // no lifetime transfer
    if (!frame_caps)
    {
        errorMsg.clear();
        errorMsg << "GStreamer: gst_sample_get_caps() returns NULL\n";
        return NULL;
    }

    if (!GST_CAPS_IS_SIMPLE(frame_caps))
    {
        // bail out in no caps
        errorMsg.clear();
        errorMsg << "GStreamer: GST_CAPS_IS_SIMPLE(frame_caps) check is failed\n";
        return NULL;
    }

    GstVideoInfo info = {};
    gboolean video_info_res = gst_video_info_from_caps(&info, frame_caps);
    if (!video_info_res)
    {
        errorMsg.clear();
        errorMsg << "GStreamer: gst_video_info_from_caps() is failed. Can't handle unknown layout\n";
    }

    // gstreamer expects us to handle the memory at this point
    // so we can just wrap the raw buffer and be done with it
    GstBuffer* buf = gst_sample_get_buffer(sample);  // no lifetime transfer
    if (!buf)
        return NULL;

    // at this point, the gstreamer buffer may contain a video meta with special
    // stride and plane locations. We __must__ consider in order to correctly parse
    // the data. The gst_video_frame_map will parse the meta for us, or default to
    // regular strides/offsets if no meta is present.
    GstVideoFrame frame = {};
    GstMapFlags flags = static_cast<GstMapFlags>(GST_MAP_READ | GST_VIDEO_FRAME_MAP_FLAG_NO_REF);
    if (!gst_video_frame_map(&frame, &info, buf, flags))
    {
        errorMsg.clear();
        errorMsg << "GStreamer: Failed to map GStreamer buffer to system memory\n";
        return NULL;
    }

    ScopeGuardGstVideoFrame frame_guard(&frame);  // call gst_video_frame_unmap(&frame) on scope leave

    int frame_width = GST_VIDEO_FRAME_COMP_WIDTH(&frame, 0);
    int frame_height = GST_VIDEO_FRAME_COMP_HEIGHT(&frame, 0);
    if (frame_width <= 0 || frame_height <= 0)
    {
        errorMsg.clear();
        errorMsg << "GStreamer: Can't query frame size from GStreamer sample\n";
        return NULL;
    }

    GstStructure* structure = gst_caps_get_structure(frame_caps, 0);  // no lifetime transfer
    if (!structure)
    {
        errorMsg.clear();
        errorMsg << "GStreamer: Can't query 'structure'-0 from GStreamer sample\n";
        return NULL;
    }

    const gchar* name_ = gst_structure_get_name(structure);
    if (!name_)
    {
        errorMsg.clear();
        errorMsg << "GStreamer: Can't query 'name' from GStreamer sample\n";
        return NULL;
    }

    const gchar* format_ = frame.info.finfo->name;
    //printf("%s\n", format_);
    //printf("n_planes = %u, size %dx%d\n", GST_VIDEO_FRAME_N_PLANES(&frame), frame_width, frame_height);

    ImageData *data = createImageDataRawPtr();
    data->setDataReleasedWhenDestroy(true);
    data->setTimestamp(getTimestampMs());
    data->getImageFormat().type = imageFormat.type;
    data->getImageFormat().width = imageFormat.width;
    data->getImageFormat().height = imageFormat.height;

    //data->setDataPtr((unsigned char*)GST_VIDEO_FRAME_PLANE_DATA(&frame, 0));
    data->allocData(frame.map[0].size);
    memcpy(data->getDataPtr(), frame.map[0].data, frame.map[0].size);
    //data->setDataPtr(frame.map[0].data);
    data->setDataSize(frame.map[0].size); 
    //printf("size %d\n", frame.map[0].size);


    return data;
}
const char *CameraInterfaceGStreamer::getErrorMsg()
{
    errorMsgStr = errorMsg.str();
    return errorMsgStr.c_str();
}

bool CameraInterfaceGStreamer::startCapturing()
{
    std::ostringstream desc;
    desc << getGStreamerCameraSource();
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
        desc << " device-path=\"" << escapeCameraPath(cameraId) << "\"";
    #else
        desc << " device=" << cameraId;
    #endif
    if(imageFormat.type == ImageType::JPG) {
        desc << " ! image/jpeg";
    } else {
        desc << " ! video/x-raw, format=" << listImageTypeStr[imageFormat.type];
    }
    desc << ",width=(int)" << imageFormat.width << ", height=(int)" << imageFormat.height;
    //desc <<  " ! " << "videoconvert"
    desc <<  " ! appsink drop=true";
    std::string command = desc.str();
    GSafePtr<GstElement> uridecodebin;
    GSafePtr<GstElement> color;
    GstStateChangeReturn status;

    GSafePtr<GstElement> convert;
    GSafePtr<GstElement> resample;
    GSafePtr<GError> err;
    uridecodebin.attach(gst_parse_launch(command.c_str(), err.getRef()));
    if (!uridecodebin)
    {
        errorMsg.clear();
        errorMsg << "Error opening bin: " << (err ? err->message : "<unknown reason>") << "\n";
        return false;
    }
    GstIterator *it = gst_bin_iterate_elements(GST_BIN(uridecodebin.get()));

    gboolean done = false;
    GValue value = G_VALUE_INIT;

    while (!done)
    {
        GstElement *element = NULL;
        GSafePtr<gchar> name;
        switch (gst_iterator_next (it, &value))
        {
            case GST_ITERATOR_OK:
                element = GST_ELEMENT (g_value_get_object (&value));
                name.attach(gst_element_get_name(element));
                if (name)
                {
                    if (strstr(name, "opencvsink") != NULL || strstr(name, "appsink") != NULL)
                    {
                        sink.attach(GST_ELEMENT(gst_object_ref(element)));
                    }
                    else if (strstr(name, "videoconvert") != NULL)
                    {
                        color.attach(GST_ELEMENT(gst_object_ref(element)));
                    }
                    else if (strstr(name, getGStreamerCameraSource().c_str()) != NULL)
                    {
                        camera_src.attach(GST_ELEMENT(gst_object_ref(element)));
                    }
                    name.release();

                    done = sink /*&& color*/ && camera_src;
                }
                g_value_unset (&value);
                break;
            case GST_ITERATOR_RESYNC:
                gst_iterator_resync (it);
                break;
            case GST_ITERATOR_ERROR:
            case GST_ITERATOR_DONE:
                done = TRUE;
                break;
        }
    }
    gst_iterator_free (it);

    if (!sink)
    {
        errorMsg.clear();
        errorMsg << "cannot find appsink in manual pipeline\n";
        return false;
    }

    pipeline.swap(uridecodebin);

    if (strstr(command.c_str(), " max-buffers=") == NULL)
    {
        //TODO: is 1 single buffer really high enough?
        gst_app_sink_set_max_buffers(GST_APP_SINK(sink.get()), 1);
    }

    gst_app_sink_set_emit_signals (GST_APP_SINK(sink.get()), false);



    if (!pipeline || !GST_IS_ELEMENT(pipeline.get()))
    {
        errorMsg.clear();
        errorMsg << "GStreamer: pipeline have not been created\n";
        return false;
    }
    status = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (status == GST_STATE_CHANGE_ASYNC)
    {
        // wait for status update
        status = gst_element_get_state(pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
    }
    if (status == GST_STATE_CHANGE_FAILURE)
    {
        handleMessage(pipeline);
        pipeline.release();
        errorMsg.clear();
        errorMsg << "unable to start pipeline\n";
        return false;
    }

    handleMessage(pipeline);

    isCapturing = true;
    return true;
}

void CameraInterfaceGStreamer::stopPipeline()
{
    if (!pipeline || !GST_IS_ELEMENT(pipeline.get()))
    {
        errorMsg.clear();
        errorMsg << "GStreamer: pipeline have not been created\n";
        return;
    }
    if (gst_element_set_state(pipeline, GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE)
    {
        errorMsg.clear();
        errorMsg << "unable to stop pipeline\n";
        pipeline.release();
    }
}

bool CameraInterfaceGStreamer::stopCapturing()
{
    camera_src.release();
    sink.release();
    sample.release();
    if (isPipelinePlaying())
        stopPipeline();
    if (pipeline && GST_IS_ELEMENT(pipeline.get()))
    {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        pipeline.release();
    }
    isCapturing = false;
    return true;
}



void CameraInterfaceGStreamer::handleMessage(GstElement * pipeline)
{
    GSafePtr<GstBus> bus;
    GstStreamStatusType tp;
    GstElement * elem = NULL;

    bus.attach(gst_element_get_bus(pipeline));

    while (gst_bus_have_pending(bus))
    {
        GSafePtr<GstMessage> msg;
        msg.attach(gst_bus_pop(bus));
        if (!msg || !GST_IS_MESSAGE(msg.get()))
            continue;

        if (gst_is_missing_plugin_message(msg))
        {
            errorMsg.clear();
            errorMsg << "your GStreamer installation is missing a required plugin\n";
        }
        else
        {
            switch (GST_MESSAGE_TYPE (msg)) {
            case GST_MESSAGE_STATE_CHANGED:
                GstState oldstate, newstate, pendstate;
                gst_message_parse_state_changed(msg, &oldstate, &newstate, &pendstate);
                break;
            case GST_MESSAGE_ERROR:
            {
                GSafePtr<GError> err;
                GSafePtr<gchar> debug;
                gst_message_parse_error(msg, err.getRef(), debug.getRef());
                GSafePtr<gchar> name; name.attach(gst_element_get_name(GST_MESSAGE_SRC (msg)));
                errorMsg.clear();
                errorMsg << "Embedded video playback halted; module " << name.get() << " reported: " << (err ? err->message : "<unknown reason>") << "\n";
                errorMsg << "GStreamer debug: " << debug.get() << "\n";

                gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
                break;
            }
            case GST_MESSAGE_EOS:
                break;
            case GST_MESSAGE_STREAM_STATUS:
                gst_message_parse_stream_status(msg,&tp,&elem);
                break;
            default:
                break;
            }
        }
    }
}

}
