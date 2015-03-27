#ifndef H_NDDL_TEST_ENGINE_
#define H_NDDL_TEST_ENGINE_

#include "Engine.hh"

class NddlTestEngine : public EUROPA::EngineBase {
 public:
  NddlTestEngine();
  virtual ~NddlTestEngine();

  virtual void init();
  int run(int argc,const char **argv);
  int run(const char* txSource, const char* language);

 protected:
  virtual void createModules();
};

#endif // H_NDDL_TEST_ENGINE_
