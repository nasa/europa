#ifndef _H_ObjectFilter
#define _H_ObjectFilter

#include "PlanDatabaseDefs.hh"
#include "../ConstraintEngine/Constraint.hh"
#include "../ConstraintEngine/EnumeratedDomain.hh"

namespace Prototype
{
  class ObjectFilter: public Constraint
  {
  public:
    ObjectFilter(const LabelStr& propagatorName,
		 const ConstraintEngineId& constraintEngine,
		 const ConstrainedVariableId& objectVariable,
		 int fieldIndex,
		 const ConstrainedVariableId& filter);

    void handleExecute();

    void handleExecute(const ConstrainedVariableId& variable, 
		       int argIndex, 
		       const DomainListener::ChangeType& changeType);

    /**
     * @brief A helper method to iterate over all objects in the given list and obtain the value of the given field, for each.
     * @param All Objects to iterate over
     * @param fieldIndex The index of the object variable to use as the source of data.
     * @return The union of all singleton values of the base domains of each filed.
     */
    static EnumeratedDomain constructUnion(const std::list<ObjectId> objects, int fieldIndex);

  private:
    const int m_fieldIndex;
    ObjectDomain& m_objectVar;
    EnumeratedDomain& m_filterVar;

    bool isValid() const;

    static const int OBJECT_VAR = 0;
    static const int FILTER_VAR = 1;
  };
}
#endif
