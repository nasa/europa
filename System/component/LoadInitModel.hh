#ifndef _H_LoadInitModel
#define _H_LoadInitModel

#define check_always(cond, optarg...) { \
  if (!(cond)) { \
    Error(#cond, ##optarg, __FILE__, __LINE__).handleAssert(); \
  } \
}


namespace EUROPA {
  /*
   * These inline functions are used to allow access 
   * to static variables within a function.
   */

  inline SamplePlanDatabase* &accessSamplePlanDB() {
    static SamplePlanDatabase *pSPdb;
    return pSPdb;
  }

  inline void* &accessLibHandle() {
    static void* s_libHandle;
    return s_libHandle;
  }

  int loadInitModel(const char* libPath, const char* initialStatePath);
  void unloadModel(void);
}

#endif

