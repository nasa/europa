#include "UnboundVariableDecisionPoint.hh"
#include "ConstrainedVariable.hh"
#include "DbClient.hh"
#include "AbstractDomain.hh"
#include "Debug.hh"
#include "ValueSource.hh"

/**
 * @brief Provides implementation for base class and common subclasses for handling variable flaws.
 * @author Conor McGann
 * @date March, 2005
 */
namespace EUROPA {
  namespace SOLVERS {

    bool UnboundVariableDecisionPoint::test(const EntityId& entity){ 
      return ConstrainedVariableId::convertable(entity);
    }

    UnboundVariableDecisionPoint::UnboundVariableDecisionPoint(const DbClientId& dbClient, 
							       const ConstrainedVariableId& flawedVariable, 
							       const TiXmlElement& configData)
      : DecisionPoint(dbClient, flawedVariable->getKey()),
	m_flawedVariable(flawedVariable),
	m_choices(ValueSource::getSource(flawedVariable)){
      checkError(flawedVariable->lastDomain().areBoundsFinite(),
		 "Attempted to allocate a Decision Point for a domain with infinite bounds for variable " 
		 << flawedVariable->toString());

      checkError(strcmp(configData.Value(), "FlawHandler") == 0,
		 "Configuration error. Expected element <FlawHandler> but found " << configData.Value());
    }

    UnboundVariableDecisionPoint::~UnboundVariableDecisionPoint(){
      delete m_choices;
    }

    const ConstrainedVariableId& UnboundVariableDecisionPoint::getFlawedVariable() const{return m_flawedVariable;}

    bool UnboundVariableDecisionPoint::canUndo() const {
      return DecisionPoint::canUndo() && m_flawedVariable->isSpecified();
    }

    void UnboundVariableDecisionPoint::handleInitialize(){}

    void UnboundVariableDecisionPoint::handleExecute(){
      double nextValue = getNext();
      debugMsg("SolverDecisionPoint:handleExecute", "For " << m_flawedVariable->toString() << 
               ", assigning value " << nextValue << ".");
      m_client->specify(m_flawedVariable, nextValue);
      debugMsg("UnboundVariableDecisionPoint:handleExecute", m_flawedVariable->toString());
    }

    void UnboundVariableDecisionPoint::handleUndo(){
      debugMsg("SolverDecisionPoint:handleUndo", "Retracting assignment decision on " << m_flawedVariable->toString());
      debugMsg("UnboundDecisionPoint:handleUndo", m_choices->getCount() << " possible values left");
      m_client->reset(m_flawedVariable);
    }

    std::string UnboundVariableDecisionPoint::toString() const{
      std::stringstream strStream;
      strStream << "VARIABLE=" <<m_flawedVariable->getName().toString() << "(" << m_flawedVariable->getKey() << ")"
		<< m_flawedVariable->lastDomain().toString();
      return strStream.str();
    }


    MinValue::MinValue(const DbClientId& client, const ConstrainedVariableId& flawedVariable, const TiXmlElement& configData)
      : UnboundVariableDecisionPoint(client, flawedVariable, configData), m_choiceIndex(0){}

    bool MinValue::hasNext() const {return m_choiceIndex < m_choices->getCount();}

    double MinValue::getNext(){return m_choices->getValue(m_choiceIndex++);}

    /** MAX VALUE **/
    MaxValue::MaxValue(const DbClientId& client, const ConstrainedVariableId& flawedVariable, const TiXmlElement& configData)
      : UnboundVariableDecisionPoint(client, flawedVariable, configData), m_choiceIndex(m_choices->getCount()){}

    bool MaxValue::hasNext() const { return m_choiceIndex > 0; }

    double MaxValue::getNext(){return m_choices->getValue(--m_choiceIndex);}

    /** RANDOM VALUE **/
    RandomValue::RandomValue(const DbClientId& client, const ConstrainedVariableId& flawedVariable, const TiXmlElement& configData)
      : UnboundVariableDecisionPoint(client, flawedVariable, configData), m_distribution(NORMAL){
      static bool sl_seeded(false);

      if(!sl_seeded){
	unsigned int seedValue = time(NULL);
	srand(seedValue);
	sl_seeded = true;
	debugMsg("RandomValue:RandomValue", "Seeding Random Number Generator with " << seedValue);
      }

      // If there is an XML child for the distribution, then obtain it. Make sure there is at most 1.
      bool foundOne = false;
      for (TiXmlElement * child = configData.FirstChildElement(); 
	   child != NULL; 
	   child = child->NextSiblingElement()) {
	checkError(foundOne == false, 
		   "Configuration File Error. Should only have at most one child, but found this extra one: " 
		   << child->Value());
	checkError(strcmp(child->Value(), "distribution") == 0,
		   "Configuration File Error. Should only have children of type 'distribution' but found: " 
		   << child->Value());

	const char * distribution = child->Attribute("name");
	if(strcmp(distribution, "NORMAL") == 0)
	  m_distribution = NORMAL;
	else if(strcmp(distribution, "UNIFORM") == 0)
	  m_distribution = UNIFORM;
	else {
	  checkError(ALWAYS_FAILS, "Configuration error. " << 
		     distribution << " is not a valid probability distribution.");
	}

	debugMsg("RandomValue:RandomValue",
		 "Configuring " << toString() << " with distribution = " << distribution);
      }
    }

    bool RandomValue::hasNext() const{ return m_choices->getCount() > m_usedIndeces.size(); }

    double RandomValue::getNext(){
      unsigned int tryCount = 1; /*!< Number of attempts to get a new selection */
      unsigned int index = rand() % m_choices->getCount();

      while(m_usedIndeces.find(index) != m_usedIndeces.end()){
	index = rand() % m_choices->getCount();
	tryCount++;
      }

      // Insert selected index so we do not repeat it
      m_usedIndeces.insert(index);

      // Now we should have a selection that indexes into the vector of choices.
      double value = m_choices->getValue(index);

      debugMsg("RandomValue:getNext", 
	       "Selecting value " << value << " at position " << index << " after " << tryCount << " attempts.");

      return value;
    }
  }
}
