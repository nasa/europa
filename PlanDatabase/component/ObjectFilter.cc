#include "ObjectFilter.hh"
#include "Object.hh"
#include "EnumeratedDomain.hh"
#include "Utils.hh"
#include <list>

namespace Prototype {

  EnumeratedDomain ObjectFilter::constructUnion(const std::list<ObjectId> objects, int fieldIndex){
    check_error(!objects.empty());
    check_error(fieldIndex >= 0);

    ConstrainedVariableId var = objects.front()->getVariables()[fieldIndex];
    bool isNumeric = var->baseDomain().isNumeric();

    EnumeratedDomain newDomain(isNumeric);

    for(std::list<ObjectId>::const_iterator it = objects.begin(); it != objects.end(); ++it){
      ObjectId object = *it;
      check_error(object->getVariables().size() > (unsigned int) fieldIndex);
      ConstrainedVariableId fieldVar = object->getVariables()[fieldIndex];
      check_error(fieldVar.isValid());
      check_error(fieldVar->baseDomain().isSingleton());
      check_error(fieldVar->lastDomain().isSingleton());
      newDomain.insert(fieldVar->baseDomain().getSingletonValue());
    }

    newDomain.close();
    return newDomain;
  }

  ObjectFilter::ObjectFilter(const LabelStr& propagatorName,
			     const ConstraintEngineId& constraintEngine,
			     const ConstrainedVariableId& objectVariable,
			     int fieldIndex,
			     const ConstrainedVariableId& filter)
    : Constraint(LabelStr("ObjectFilter"), LabelStr("Default"), constraintEngine, makeScope(objectVariable, filter)),
      m_fieldIndex(fieldIndex), 
      m_objectVar(static_cast<ObjectDomain&>(getCurrentDomain(getScope()[OBJECT_VAR]))),
      m_filterVar(static_cast<EnumeratedDomain&>(getCurrentDomain(getScope()[FILTER_VAR]))){
    check_error(isValid());
  }

  void ObjectFilter::handleExecute(){
    check_error(isValid());

    // Iterate over field and tes
    std::list<ObjectId> originalObjects;
    m_objectVar.getValues(originalObjects);

    // Initialize to the base domain
    EnumeratedDomain fieldValues(m_filterVar.isNumeric());

    // Iterate over the values, getting the field variable each time and making sure it is in the list
    for (std::list<ObjectId>::const_iterator it = originalObjects.begin(); it != originalObjects.end(); ++it){
      ObjectId object = *it;
      double fieldValue = getCurrentDomain(object->getVariables()[m_fieldIndex]).getSingletonValue();
      if(m_filterVar.isMember(fieldValue))
	fieldValues.insert(fieldValue);
      else
	m_objectVar.remove(object);

      if(m_objectVar.isEmpty())
	return;
    }

    fieldValues.close();

    // Now restrict the filterVariable
    if(!fieldValues.isEmpty())
      m_filterVar.intersect(fieldValues);
  }

  void ObjectFilter::handleExecute(const ConstrainedVariableId& variable, 
				   int argIndex, 
				   const DomainListener::ChangeType& changeType){
    handleExecute();
  }

  bool ObjectFilter::isValid() const {
    check_error(m_fieldIndex >= 0);

    // For all objects in the object variable, confirm that:
    // 0. It is a valid object if.
    // 1. The object has a field given by fieldIndex
    // 2. The field variables specified domain is a singleton
    std::list<ObjectId> objects;
    m_objectVar.getValues(objects);
    for(std::list<ObjectId>::const_iterator it = objects.begin(); it!= objects.end(); ++it){
      ObjectId object = *it;
      check_error(object.isValid());
      check_error(object->getVariables().size() > (unsigned int) m_fieldIndex);
      check_error(object->getVariables()[m_fieldIndex]->specifiedDomain().isSingleton());
    }

    return true;
  }
}
