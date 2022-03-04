# RPCameraInterface

Portable (Windows and Linux, no Mac OS yet) and unified interface for cameras.

**Still a prototype, not all backends are fully implemented yet.**

Similar to OpenCV camera interface but with a few differences :
* Provides the list of available cameras
* Provides the list of available formats and resolutions
* Supports MJPG encoding (allows higher framerate than OpenCV)
* Easy formats conversion based on ffmpeg
* Support on-device recording with preview (only for Android phones currently, but plan to add Raspberry pi, Oak-D,... later)

### License
Apache 2

### Credits
The DShow backend is based on OpenCV backend and a few functions from libwebcam
