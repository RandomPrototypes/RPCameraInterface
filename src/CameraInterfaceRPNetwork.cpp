#include <RPCameraInterface/CameraInterfaceRPNetwork.h>
//#include <QDebug>
#include <string.h>
#include <BufferedSocket/DataPacket.h>

namespace RPCameraInterface
{

enum RPNetworkCameraCmd
{
    EXIT = 100,
    LIST_CAMERAS =200,
    TIMESTAMP = 300,
    START_RECORDING = 400,
    STOP_RECORDING = 500,
    CAPTURE_IMG = 600
};

CameraEnumeratorRPNetwork::CameraEnumeratorRPNetwork()
    :CameraEnumeratorBase(CaptureBackend::RPNetworkCamera)
{
    cameraType = "Network camera";
    listRequiredField.push_back(CameraEnumeratorField("ip_address", "text", "ip address", ""));
    listRequiredField.push_back(CameraEnumeratorField("port", "text", "port", "25600"));
}

CameraEnumeratorRPNetwork::~CameraEnumeratorRPNetwork()
{
}

std::vector<std::string> splitString(const char* str, int length, char delim)
{
    std::vector<std::string> list;
    int start = 0;
    for(int i = 0; i <= length; i++)
    {
        if(i == length || str[i] == delim)
        {
            if(i > start)
                list.push_back(std::string(&str[start], &str[i]));
            start = i+1;
        }
    }
    return list;
}

bool CameraEnumeratorRPNetwork::detectCameras()
{
    //qDebug() << "detectCameras";
    listCameras.clear();
    std::shared_ptr<BufferedSocket> bufferedSock = createBufferedSocket();
    const char *ip_address = listRequiredField[0].value.c_str();
    int port = std::stoi(listRequiredField[1].value);
    if (!bufferedSock->connect(ip_address, port))
    {
        return false;
    }
    //qDebug() << "connected";
    bufferedSock->sendInt32(RPNetworkCameraCmd::LIST_CAMERAS);
    bufferedSock->sendInt64(0);
    int ret = bufferedSock->readInt32();
    if(ret != RPNetworkCameraCmd::LIST_CAMERAS)
    {
        //qDebug() << "bad reply";
        bufferedSock->disconnect();
        return false;
    }
    int nbCameras = bufferedSock->readInt32();
    //qDebug() << "nbCameras : " << nbCameras;
    for(int i = 0; i < nbCameras; i++)
    {
        CameraInfo camInfo;
        int size = bufferedSock->readInt32();
        char *data = new char[size+1];
        bufferedSock->readNBytes(data, size);
        std::vector<std::string> lines = splitString(data, size, '\n');
        for(std::string& line : lines)
        {
            if(line.rfind("id:", 0) == 0)
                camInfo.id = std::string(ip_address)+":"+std::to_string(port)+":"+line.substr(3);
            else if(line.rfind("name:", 0) == 0)
                camInfo.name = line.substr(5);
            else if(line.rfind("desc:", 0) == 0)
                camInfo.description = line.substr(5);
        }
        //qDebug() << camInfo.id.c_str();
        listCameras.push_back(camInfo);
        delete [] data;
    }
    bufferedSock->disconnect();
    return true;
}






CameraInterfaceRPNetwork::CameraInterfaceRPNetwork()
    :CameraInterfaceBase(CaptureBackend::RPNetworkCamera)
{
	bufferedSock = createBufferedSocket();
}

CameraInterfaceRPNetwork::~CameraInterfaceRPNetwork()
{
}

bool CameraInterfaceRPNetwork::open(const char *params)
{
    //qDebug() << "CameraInterfaceRPNetwork::open(\"" << params.c_str() << "\")";
    std::vector<std::string> list_params = splitString(params, strlen(params), ':');
    if(!bufferedSock->connect(list_params[0].c_str(), std::stoi(list_params[1])))
    {
        return false;
    }
    //qDebug() << "connection successful";
    if(!syncTimestamp())
        return false;
    selectVideoCodec(VideoCodecType::H264);
    selectVideoContainer(VideoContainerType::MP4);
    return true;
}

bool CameraInterfaceRPNetwork::close()
{
    bufferedSock->disconnect();
    return true;
}


size_t CameraInterfaceRPNetwork::getAvailableFormatCount()
{
	return 1;
}
ImageFormat CameraInterfaceRPNetwork::getAvailableFormat(size_t id)
{
	ImageFormat format;
	format.width = 640;
	format.height = 480;
	format.type = ImageType::JPG;
	return format;
}

size_t CameraInterfaceRPNetwork::getAvailableVideoCodecCount()
{
	return 1;
}

VideoCodecType CameraInterfaceRPNetwork::getAvailableVideoCodec(size_t id)
{
	return VideoCodecType::H264;
}

size_t CameraInterfaceRPNetwork::getAvailableVideoContainerCount()
{
	return 1;
}

VideoContainerType CameraInterfaceRPNetwork::getAvailableVideoContainer(size_t id)
{
	return VideoContainerType::MP4;
}

void CameraInterfaceRPNetwork::selectFormat(ImageFormat format)
{
    imageFormat = format;
}


void CameraInterfaceRPNetwork::selectVideoContainer(VideoContainerType container)
{
    videoContainerType = container;
}

void CameraInterfaceRPNetwork::selectVideoCodec(VideoCodecType codec)
{
    videoCodecType = codec;
}

ImageData *CameraInterfaceRPNetwork::getNewFramePtr(bool skipOldFrames)
{
    bufferedSock->sendInt32(CAPTURE_IMG);
    std::shared_ptr<DataPacket> packet = createDataPacket();
    packet->putInt32(imageFormat.width);
    packet->putInt32(imageFormat.height);
    std::string format = "";
    if(imageFormat.type == ImageType::JPG)
        format = "MJPG";
    packet->putInt32(format.size());
    packet->putNBytes(format.c_str(), format.size());
    bufferedSock->sendInt64(packet->size());
    bufferedSock->sendNBytes(packet->getRawPtr(), packet->size());

    if(bufferedSock->readInt32() != CAPTURE_IMG)
    {
        //qDebug() << "protocol error";
        errorMsg = "protocol error\n";
        return NULL;
    }

    int32_t frame_id = bufferedSock->readInt32();
    int64_t timestamp = bufferedSock->readInt64();
    int64_t size = bufferedSock->readInt64();
    ImageData *img = createImageDataRawPtr(ImageFormat(ImageType::JPG, imageFormat.width, imageFormat.height));
    img->setTimestamp(timestamp + timestampOffsetMs);
    img->allocData(size);
    bufferedSock->readNBytes((char*)img->getDataPtr(), size);
    //qDebug() << "new frame, id : " << frame_id << "timestamp : " << img->timestamp;
    return img;
}
const char *CameraInterfaceRPNetwork::getErrorMsg()
{
    return errorMsg.c_str();
}

bool CameraInterfaceRPNetwork::startCapturing()
{
    return true;
}

bool CameraInterfaceRPNetwork::stopCapturing()
{
    return true;
}

bool CameraInterfaceRPNetwork::hasRecordingCapability()
{
    return true;
}
bool CameraInterfaceRPNetwork::startRecording()
{
    bufferedSock->sendInt32(START_RECORDING);
    bufferedSock->sendInt64(0);
    if(bufferedSock->readInt32() != START_RECORDING)
    {
        //qDebug() << "protocol error";
        errorMsg = "protocol error\n";
        return false;
    }
    return true;
}
bool CameraInterfaceRPNetwork::stopRecordingAndSaveToFile(std::string videoFilename, std::string timestampFilename)
{
    bufferedSock->sendInt32(STOP_RECORDING);
    bufferedSock->sendInt64(0);
    if(bufferedSock->readInt32() != STOP_RECORDING)
    {
        //qDebug() << "protocol error";
        errorMsg = "protocol error\n";
        return false;
    }
    int64_t startRecordTimestamp = bufferedSock->readInt64();
    int64_t size = bufferedSock->readInt64();

    //qDebug() << "writing to file...";
    FILE *timestampFile = fopen(timestampFilename.c_str(), "w");
    fprintf(timestampFile, "%ld\n", startRecordTimestamp);
    fclose(timestampFile);
    FILE *file = fopen(videoFilename.c_str(), "wb");
    int64_t offset = 0;
    while(offset < size)
    {
        char buf[1024];
        int max_read_len = (int)std::min((int64_t)sizeof(buf), size - offset);
        int len = bufferedSock->readNBytes(buf, max_read_len);
        if(len < 0)
            break;
        if(file != NULL)
            fwrite(buf, 1, len, file);
        offset += len;
    }
    if(!file)
    {
        //qDebug() << "can not open file: " << videoFilename.c_str();
        errorMsg = "can not open file: "+videoFilename;
        return false;
    }
    fclose(file);
    //qDebug() << "Done";
    return true;
}

bool CameraInterfaceRPNetwork::syncTimestamp()
{
    bufferedSock->sendInt32(TIMESTAMP);
    bufferedSock->sendInt64(0);
    uint64_t startTimestamp = getTimestampMs();
    if(bufferedSock->readInt32() != TIMESTAMP)
    {
        //qDebug() << "protocol error";
        errorMsg = "protocol error\n";
        return false;
    }
    uint64_t endTimestamp = getTimestampMs();
    int64_t targetTimestampMs = bufferedSock->readInt64();
    //qDebug() << "roundtrip time : " << (endTimestamp - startTimestamp) << "ms";
    timestampOffsetMs = ((int64_t)(startTimestamp+endTimestamp)/2) - targetTimestampMs;
    //qDebug() << "offset : " << timestampOffsetMs << "ms";
    return true;
}

}
