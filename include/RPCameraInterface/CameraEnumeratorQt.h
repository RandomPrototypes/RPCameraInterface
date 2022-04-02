#ifndef CAMERAENUMERATORQT_H
#define CAMERAENUMERATORQT_H

#include "CameraInterfaceBase.h"

namespace RPCameraInterface
{

class CameraEnumeratorQt : public CameraEnumeratorBase
{
public:
    CameraEnumeratorQt();
    virtual ~CameraEnumeratorQt();

    virtual bool detectCameras();
};

}

#endif // CAMERAENUMERATORQT_H
