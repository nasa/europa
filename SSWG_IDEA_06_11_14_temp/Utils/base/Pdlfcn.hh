#ifndef _H_Pdlfcn
#define _H_Pdlfcn

/**
 * @file   Pdlfcn.hh
 * @author Patrick Daley
 * @date   
 * @brief  These functions provide a wrapper for unix dlopen(), dlsym(), dlclose()
 *         and dlerror() functions. When compiled for MAC OSX, these functions 
 *         emulate the unix dl functions using Apple's API.
 * @ingroup Utils
 */
#include <dlfcn.h>

namespace EUROPA {

void * p_dlopen(const char *path, int mode);
void * p_dlsym(void * handle, const char * Symbol);
const char * p_dlerror(void);
int p_dlclose(void * handle);
}
#endif
