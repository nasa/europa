#ifndef _H_ANML_TEST_ENGINE_
#define _H_ANML_TEST_ENGINE_

#include "NddlTestEngine.hh"

using namespace EUROPA;

class AnmlTestEngine : public NddlTestEngine
{
  public:
	AnmlTestEngine();
	virtual ~AnmlTestEngine();

  protected:
	virtual void createModules();
};

#endif // _H_ANML_TEST_ENGINE_
