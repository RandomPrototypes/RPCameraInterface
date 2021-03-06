#pragma once

//Taken from OpenCV

#ifndef RPCAM_EXPORTS
# if (defined _WIN32 || defined WINCE || defined __CYGWIN__)
#   if defined(RPCAMERAINTERFACE_EXPORTS)
#      define RPCAM_EXPORTS __declspec(dllexport)
#   else
#      define RPCAM_EXPORTS __declspec(dllimport)
#   endif
# elif defined __GNUC__ && __GNUC__ >= 4 && (defined(RPCAMERAINTERFACE_EXPORTS) || defined(__APPLE__))
#   define RPCAM_EXPORTS __attribute__ ((visibility ("default")))
# endif
#endif

#ifndef RPCAM_EXPORTS
# define RPCAM_EXPORTS
#endif
