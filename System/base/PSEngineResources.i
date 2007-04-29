%module PSEngineInterface
%{
#include "PSResources.hh"
#define PSEngine PSEngineWithResources
%}

%include "PSEngine.i"

namespace EUROPA {

  class PSResource;
  class PSResourceProfile;

  %template(PSResourceList) PSList<PSResource*>;

  class PSResource : public PSObject
  {
  public:
    PSResourceProfile* getLimits();
    PSResourceProfile* getLevels();
  protected:
    PSResource();
  };

  class PSResourceProfile
  {
  public:
    const PSList<TimePoint>& getTimes();
    double getLowerBound(TimePoint time);
    double getUpperBound(TimePoint time);
  protected:
    PSResourceProfile();
  };
}
