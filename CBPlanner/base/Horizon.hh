#ifndef _H_Horizon
#define _H_Horizon

#include "Entity.hh"
#include "CBPlanner.hh"
#include <set>

namespace EUROPA {

  class Horizon : public Entity  {
  public:
    enum HorizonChangeType {
      START_DECREASED = 0,
      START_INCREASED,
      END_DECREASED,
      END_INCREASED
    };

    Horizon();
    Horizon(const int& start, const int& end);
    virtual ~Horizon();

    bool operator==(const Horizon& hor) const;

    void setHorizon(const int& start, const int& end);
    void getHorizon(int& start, int& end); 

    const HorizonId& getId();

    void print (std::ostream& os = std::cout);

  protected:
    HorizonId m_id;
    int m_start;
    int m_end;
  };

}

#endif
