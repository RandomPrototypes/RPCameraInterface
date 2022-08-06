#include <RPCameraInterface/CameraInterfaceOpenCV.h>
#include <opencv2/opencv.hpp>

namespace RPCameraInterface
{

CameraEnumeratorOpenCV::CameraEnumeratorOpenCV()
    :CameraEnumeratorBase(CaptureBackend::OpenCV)
{
    cameraType = "OpenCV camera";
    listParamField.push_back(new CameraEnumeratorFieldBase("cam_id", "text", "camera id", "0"));
    listParamField.push_back(new CameraEnumeratorFieldBase("cam_res", "text", "camera resolution", "1280x720"));
    listParamField.push_back(new CameraEnumeratorFieldBase("capture_backend", "text", "backend", "any"));
}

CameraEnumeratorOpenCV::~CameraEnumeratorOpenCV()
{
}

bool CameraEnumeratorOpenCV::detectCameras()
{
    //qDebug() << "detectCameras";
    listCameras.clear();
    CameraInfo camInfo;
    const char *cam_id = listParamField[0]->getValue();
    const char *cam_res = listParamField[1]->getValue();
    const char *capture_backend = listParamField[2]->getValue();
    if(strlen(cam_id) == 0)
        return false;
    camInfo.id = "backend="+std::string(capture_backend)+"; res="+std::string(cam_res)+"; "+std::string(cam_id);
    std::string name;
    if(!strcmp(capture_backend, "any"))
        camInfo.name = std::string(cam_id);
    else camInfo.name = std::string(capture_backend)+": "+std::string(cam_id);
    camInfo.description = "backend: "+std::string(capture_backend)+", camera id: "+std::string(cam_id);
    CameraInterfaceOpenCV cam;
    if(!cam.open(camInfo.id.c_str()))
        return false;
    cam.close();
    listCameras.push_back(camInfo);
    return true;
}

CameraInterfaceOpenCV::CameraInterfaceOpenCV()
    :CameraInterfaceBase(CaptureBackend::OpenCV)
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


inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

bool CameraInterfaceOpenCV::open(const char *params)
{
    std::map<std::string, int> listBackends;
    listBackends.insert(std::make_pair("any", cv::CAP_ANY));
    listBackends.insert(std::make_pair("vfw", cv::CAP_VFW));
    listBackends.insert(std::make_pair("v4l", cv::CAP_V4L));
    listBackends.insert(std::make_pair("v4l2", cv::CAP_V4L2));
    listBackends.insert(std::make_pair("firewire", cv::CAP_FIREWIRE));
    listBackends.insert(std::make_pair("qt", cv::CAP_QT));
    listBackends.insert(std::make_pair("unicap", cv::CAP_UNICAP));
    listBackends.insert(std::make_pair("dshow", cv::CAP_DSHOW));
    listBackends.insert(std::make_pair("pvapi", cv::CAP_PVAPI));
    listBackends.insert(std::make_pair("openni", cv::CAP_OPENNI));
    listBackends.insert(std::make_pair("openni_asus", cv::CAP_OPENNI_ASUS));
    listBackends.insert(std::make_pair("android", cv::CAP_ANDROID));
    listBackends.insert(std::make_pair("xiapi", cv::CAP_XIAPI));
    listBackends.insert(std::make_pair("avfoundation", cv::CAP_AVFOUNDATION));
    listBackends.insert(std::make_pair("giganetix", cv::CAP_GIGANETIX));
    listBackends.insert(std::make_pair("msmf", cv::CAP_MSMF));
    listBackends.insert(std::make_pair("winrt", cv::CAP_WINRT));
    listBackends.insert(std::make_pair("intelperc", cv::CAP_INTELPERC));
    listBackends.insert(std::make_pair("openni2", cv::CAP_OPENNI2));
    listBackends.insert(std::make_pair("openni2_asus", cv::CAP_OPENNI2_ASUS));
    listBackends.insert(std::make_pair("gphoto2", cv::CAP_GPHOTO2));
    listBackends.insert(std::make_pair("gstreamer", cv::CAP_GSTREAMER));
    listBackends.insert(std::make_pair("ffmpeg", cv::CAP_FFMPEG));
    listBackends.insert(std::make_pair("images", cv::CAP_IMAGES));
    listBackends.insert(std::make_pair("aravis", cv::CAP_ARAVIS));
    listBackends.insert(std::make_pair("opencv_mjpeg", cv::CAP_OPENCV_MJPEG));
    listBackends.insert(std::make_pair("intel_mfx", cv::CAP_INTEL_MFX));
    listBackends.insert(std::make_pair("xine", cv::CAP_XINE));
    try
    {
        int id = std::stoi(params);
        cap = new cv::VideoCapture(id, cv::CAP_ANY);
    } catch(const std::invalid_argument& ia) {
        std::string str = params;
        ltrim(str);
        int capture_backend = cv::CAP_ANY;
        int resolution[2] = {0,0};
        if(str.rfind("backend", 0) == 0) {
            str.erase(str.begin(), str.begin()+strlen("backend"));
            ltrim(str);
            if(str[0] != '=')
                return false;
            str.erase(str.begin(), str.begin()+1);
            ltrim(str);
            int longest_match = 0;
            for(auto& backend : listBackends) {
                if(str.rfind(backend.first, 0) == 0 && backend.first.size() > longest_match) {
                    capture_backend = backend.second;
                    longest_match = backend.first.size();
                }
            }
            if(longest_match == 0)
                return false;
            str.erase(str.begin(), str.begin()+longest_match);
            ltrim(str);
            if(str[0] == ';')
                str.erase(str.begin(), str.begin()+1);
            ltrim(str);
        }
        if(str.rfind("res", 0) == 0) {
            str.erase(str.begin(), str.begin()+strlen("res"));
            ltrim(str);
            if(str[0] != '=')
                return false;
            str.erase(str.begin(), str.begin()+1);
            for(int i = 0; i < 2; i++) {
                ltrim(str);
                while(str[0] >= '0' && str[0] <= '9') {
                    resolution[i] = resolution[i] * 10 + str[0]-'0';
                    str.erase(str.begin(), str.begin()+1);
                }
                ltrim(str);
                if(i == 0)
                {
                    if(str[0] != 'x' && str[0] != 'X')
                        return false;
                    str.erase(str.begin(), str.begin()+1);
                }
            }
            if(str[0] == ';')
                str.erase(str.begin(), str.begin()+1);
            ltrim(str);
        }
        try
        {
            int id = std::stoi(str.c_str());
            cap = new cv::VideoCapture(id, capture_backend);
        } catch(std::invalid_argument& ia2) {
            cap = new cv::VideoCapture(str.c_str(), capture_backend);
        } catch(const std::exception& e) {
            return false;
        }
        if(resolution[0] != 0)
            cap->set(cv::CAP_PROP_FRAME_WIDTH, resolution[0]);
        if(resolution[1] != 0)
            cap->set(cv::CAP_PROP_FRAME_HEIGHT, resolution[1]);
    } catch(const std::exception& e) {
        return false;
    }
    
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

void CameraInterfaceOpenCV::testAvailableFormats()
{
    std::vector<ImageFormat> list;
    ImageFormat format;
    format.type = ImageType::BGR24;
    format.width = cvRound(cap->get(cv::CAP_PROP_FRAME_WIDTH));
    format.height = cvRound(cap->get(cv::CAP_PROP_FRAME_HEIGHT));
    list.push_back(format);
    listFormats = list;

    //currently disabled because too slow
    /*
    std::vector<cv::Size> listSizeToTest = {cv::Size(320,240), cv::Size(640,480), cv::Size(1280,720), cv::Size(1920,1080)
                                            //, cv::Size(3840,2160), cv::Size(4096,2160)
                                            };
    for(const cv::Size& size : listSizeToTest) {
        ImageFormat format;
        format.width = size.width;
        format.height = size.height;
        format.type = ImageType::BGR24;
        if(testResolution(size.width, size.height, false))
            list.push_back(format);
        //format.type = ImageType::JPG;
        //if(testResolution(size.width, size.height, true))
        //    list.push_back(format);
    }
    listFormats = list;*/
}

size_t CameraInterfaceOpenCV::getAvailableFormatCount()
{
	if(listFormats.size() == 0)
		testAvailableFormats();
	return listFormats.size();
}

ImageFormat CameraInterfaceOpenCV::getAvailableFormat(size_t id)
{
	if(listFormats.size() == 0)
		testAvailableFormats();
	if(id < listFormats.size())
		return listFormats[id];
	return ImageFormat();
}
	
void CameraInterfaceOpenCV::selectFormat(int formatId)
{
    if(listFormats.size() == 0)
        testAvailableFormats();
    if(formatId < listFormats.size())
        imageFormat = listFormats[formatId];
}
void CameraInterfaceOpenCV::selectFormat(ImageFormat format)
{
    imageFormat = format;
}
ImageData *CameraInterfaceOpenCV::getNewFramePtr(bool skipOldFrames)
{
    if(cap != NULL)
    {
        *cap >> frame;
        ImageData *data = createImageDataRawPtr();
        data->setDataReleasedWhenDestroy(false);
        data->setTimestamp(getTimestampMs());
        data->getImageFormat().type = ImageType::BGR24;
        data->getImageFormat().width = frame.cols;
        data->getImageFormat().height = frame.rows;
        data->setDataPtr(const_cast<unsigned char*>(frame.data));
        data->setDataSize(frame.rows*frame.cols*3);
        return data;
    }
    return NULL;
}
const char *CameraInterfaceOpenCV::getErrorMsg()
{
    return errorMsg.c_str();
}

bool CameraInterfaceOpenCV::startCapturing()
{
    if(cap != NULL) {
        /*if(imageFormat.type == ImageType::JPG) {
            cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('m', 'j', 'p', 'g'));
            cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
        } else*/ {
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

}
