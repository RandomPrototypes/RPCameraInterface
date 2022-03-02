#ifndef CAMERAENUMERATORQT_H
#define CAMERAENUMERATORQT_H

#include "CameraInterface.h"

namespace RPCameraInterface
{

class CameraEnumeratorQt : public CameraEnumerator
{
public:
    CameraEnumeratorQt();
    virtual ~CameraEnumeratorQt();

    virtual bool detectCameras();
};

}

#endif // CAMERAENUMERATORQT_H
