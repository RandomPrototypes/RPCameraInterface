#pragma once

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
