#ifndef _H_Pdlfcn
#define _H_Pdlfcn

/**   Pdlfcn.hh
 * @author Patrick Daley
 * @date   
 * @brief  These functions provide a wrapper for unix dlopen(), dlsym(),
 *         dlclose(), and dlerror)( functions. When compiled for __APPLE__,
 *         these functions emulate the unix function using Apple's API.
 *         OS-X 10.4  (Tiger) introduced a native Apple implementation of 
 *         these functions with GCC 4.0
 * @ingroup Utils
 */
#include "Pdlfcn.hh"

#if defined(__APPLE__) && (__GNUC__ < 4)   
#include <mach-o/dyld.h>
#else
#include <dlfcn.h>
#endif
#include<iostream>
#include<stdlib.h>

#define DLERROR_STR_MAX_LEN 256

namespace EUROPA {

#if defined(__APPLE__) && (__GNUC__ < 4)    

  /*
   * Provide portable dl functions using Apple's API for gcc prior to v4
   */

  // helper function to set or get error string used by dlerror
  static const char *setError(int isSet, const char *fmt, ...) {
    static char errorStr[DLERROR_STR_MAX_LEN];
    static int errorIsSet = 0;
    va_list arg; 
    if (isSet) {
      //set the error string
      va_start(arg, fmt);
      vsnprintf(errorStr, sizeof(errorStr), fmt, arg);
      va_end(arg);
      errorIsSet = 1;
    } else {
      //get the error string
      if (errorIsSet) {
        errorIsSet = 0;
        return errorStr;
      } else {
        return 0;
      }
    }
    return 0;
  }

  void * p_dlopen(const char *path, int mode) {  /* mode ignored */
    NSObjectFileImage *fileImage;
    NSModule handle = 0;
    NSObjectFileImageReturnCode  returnCode =
           NSCreateObjectFileImageFromFile(path, &fileImage);

    if(returnCode == NSObjectFileImageSuccess) {
      //try to load bundle (not used for EUROPA2)
      handle = NSLinkModule(fileImage, path,
                            NSLINKMODULE_OPTION_RETURN_ON_ERROR |
                            NSLINKMODULE_OPTION_PRIVATE);
    } else if(returnCode == NSObjectFileImageInappropriateFile) {
      //try to load dynamic library (normal path in EUROPA2)
      handle = NSAddImage(path, NSADDIMAGE_OPTION_RETURN_ON_ERROR);
    }
    if (!handle) {
      setError(1, "p_dlopen:  Could not load shared library: %s", path);
    }
    return handle;
  }

  void * p_dlsym(void * handle, const char * symbol) {
    char ubSymbol[256];
    snprintf (ubSymbol, sizeof(ubSymbol), "_%s", symbol);
    
    NSSymbol nssym = NSLookupSymbolInImage(handle, ubSymbol,
                            NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR |
                            NSLOOKUPSYMBOLINIMAGE_OPTION_BIND);
    if (!nssym) {
      setError(1, "p_dlsym: Did not find symbol %s", ubSymbol);
    }
    return (NSAddressOfSymbol(nssym));
  }


  const char * p_dlerror(void) {
    return setError(0, (char *) 0);
  }

  int p_dlclose(void * handle) {

    DYLD_BOOL result = NSUnLinkModule(handle, 0);
    if (!result) {
      setError(1, "p_dlclose: Failed to close library %s", NSNameOfModule(handle));
      //darwin doesn't seem to support unloading shared libraries, so fake it.
      return 0;
    }
    return 0;
  }

#else
  /*
   * use unix dl functions in all non Apple cases
   */
  void * p_dlopen(const char *path, int mode) {  
    return dlopen(path, mode);
  }

  void * p_dlsym(void * handle, const char * symbol) {
    return dlsym(handle, symbol);
  }

  const char * p_dlerror(void) {
    return dlerror();
  }

  int p_dlclose(void * handle) {
    return dlclose(handle);
  }
#endif

}
#endif
