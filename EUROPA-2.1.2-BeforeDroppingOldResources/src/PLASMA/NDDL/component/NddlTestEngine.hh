#ifndef _H_NDDL_TEST_ENGINE_
#define _H_NDDL_TEST_ENGINE_

#include "Engine.hh"

using namespace EUROPA;

class NddlTestEngine : public EngineBase
{
  public:
	NddlTestEngine();
	virtual ~NddlTestEngine();

	virtual void init();
	int run(int argc,const char **argv);
	void run(const char* txSource, const char* language);

  protected:
	virtual void createModules();
};

#endif // _H_NDDL_TEST_ENGINE_
