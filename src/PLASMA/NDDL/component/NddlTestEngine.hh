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
	void run(const char* txSource, const std::string& language);

  protected:
	virtual void createModules();
};

#endif // _H_NDDL_TEST_ENGINE_
