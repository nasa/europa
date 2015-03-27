#ifndef H_ANML_TEST_ENGINE_
#define H_ANML_TEST_ENGINE_

#include "NddlTestEngine.hh"

class AnmlTestEngine : public NddlTestEngine
{
  public:
	AnmlTestEngine();
	virtual ~AnmlTestEngine();

  protected:
	virtual void createModules();
};

#endif // H_ANML_TEST_ENGINE_
