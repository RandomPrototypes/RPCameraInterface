# RPCameraInterface

Portable (Windows and Linux, no Mac OS yet) and unified interface for cameras.

**Still a prototype, not all backends are fully implemented yet.**

Similar to OpenCV camera interface but with a few differences :
* Provides the list of available cameras
* Provides the list of available formats and resolutions
* Supports MJPG encoding (allows higher framerate than OpenCV)
* Easy formats conversion based on ffmpeg
* Support on-device recording with preview (only for Android phones currently, but plan to add Raspberry pi, Oak-D,... later)

### On-device recording
Some devices have capability to record videos directly. This can be useful to reduce the required bandwidth while keeping high-framerate and high-resolution recording.
Preview can be streamed at low-resolution.
This currently only includes Android phones, but more will be added later.

### License
Apache 2

### Credits
The DShow backend is based on OpenCV backend and a few functions from libwebcam
