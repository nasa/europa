#include "TemporalVariableFilter.hh"

#include "Token.hh"
#include "ConstrainedVariable.hh"
#include "TokenVariable.hh"

namespace EUROPA {
  TemporalVariableFilter::TemporalVariableFilter(const DecisionManagerId& dm)
    : Condition(dm, false){}

  TemporalVariableFilter::~TemporalVariableFilter(){}
   
   
  bool TemporalVariableFilter::test(const EntityId& entity){
    return !isTemporalVariable(entity);
  }

  bool TemporalVariableFilter::isTemporalVariable(const EntityId& entity){
    static const LabelStr sl_start("start");
    static const LabelStr sl_end("end");
    static const LabelStr sl_duration("duration");

    if(!ConstrainedVariableId::convertable(entity))
      return false;

    ConstrainedVariableId var(entity);
    
    if(var->getParent().isNoId() || !TokenId::convertable(var->getParent()))
      return false;
    
    if(var->getName() == sl_start ||
       var->getName() == sl_end ||
       var->getName() == sl_duration)
      return true;

    return false;
  }
}
