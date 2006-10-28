#include "SAVH_ThreatDecisionPoint.hh"
#include "SAVH_Instant.hh"
#include "SAVH_Transaction.hh"
#include "SAVH_Profile.hh"
#include "SAVH_Resource.hh"
#include "SAVH_FlowProfile.hh"
#include "Constraint.hh"
#include "DbClient.hh"

namespace EUROPA {
  using namespace SOLVERS;
  namespace SAVH {
    
    class ChoiceFilter {
    public:
      virtual bool operator()(const std::pair<TransactionId, TransactionId>& p) const = 0;
      virtual std::string toString() const = 0;
      virtual ~ChoiceFilter(){}
    private:
    };
    

    class ChoiceFilters {
    public:
      ChoiceFilters() {}
      ~ChoiceFilters() {
        for(std::list<ChoiceFilter*>::iterator it = m_filters.begin(); it != m_filters.end(); ++it)
          delete (*it);
        m_filters.clear();
      }
      bool operator()(const std::pair<TransactionId, TransactionId>& p) const {
        for(std::list<ChoiceFilter*>::const_iterator it = m_filters.begin(); it != m_filters.end(); ++it) {
          ChoiceFilter* filter = *it;
          debugMsg("ThreatDecisionPoint:filter", "Testing <" << p.first->toString() << ", " <<
                   p.second->toString() << ">");
          if(!(*filter)(p)) {
            debugMsg("ThreatDecisionPoint:filter", "Filtering out <" << p.first->toString() << ", " <<
                     p.second->toString() << "> because of " << filter->toString());
            return false;
          }
        }
        debugMsg("ThreatDecisionPoint:filter", "<" << p.first->toString() << ", " << p.second->toString() << "> passed.");
        return true;
      }
      void addFilter(ChoiceFilter* filter) {
        debugMsg("ThreatDecisionPoint:filter", "Adding filter " << filter->toString());
        m_filters.push_back(filter);
      }
    private:
      std::list<ChoiceFilter*> m_filters;
    };

    class DefaultChoiceFilter : public ChoiceFilter {
    public:
      DefaultChoiceFilter(FlowProfile* profile, const LabelStr& explanation, const InstantId& inst) 
        : ChoiceFilter(), m_profile(profile), m_explanation(explanation), m_inst(inst) {
        m_treatAsLowerFlaw = true;
        debugMsg("ThreatDecisionPoint:filter", "Creating filter for " << inst->getTime() << " on " << inst->getProfile()->getResource()->toString());
        //if there are flaws at both levels
        if(m_inst->hasLowerLevelFlaw() && m_inst->hasUpperLevelFlaw()) {
          debugMsg("ThreatDecisionPoint:filter", "Instant is flawed on both levels.");
          //  if we were chosen out of a lower level preference, behave like that
          if(m_explanation == LabelStr("lowerLevelFlaw") || m_explanation.toString().find("Lower") != std::string::npos) {
            debugMsg("ThreatDecisionPoint:filter", "Treating as lower flaw because of " << m_explanation.toString());
            m_treatAsLowerFlaw = true;
          }
          //  if we were chosen out of an upper level preference, behave like that
          else if(m_explanation == LabelStr("upperLevelFlaw") || m_explanation.toString().find("Upper") != std::string::npos) {
            debugMsg("ThreatDecisionPoint:filter", "Treating as upper flaw because of " << m_explanation.toString());
            m_treatAsLowerFlaw = false;
          }
          //  if we were chosen out of a magnitude preference 
          //    pick level with greatest magnitude, treat as a flaw on that level
          //    if the level magnitude is equal, arbitrarily choose lower level
          else {
            m_treatAsLowerFlaw = m_inst->getLowerFlawMagnitude() >= m_inst->getUpperFlawMagnitude();       
            debugMsg("ThreatDecisionPoint:filter", "Treating as " << (m_treatAsLowerFlaw ? "lower" : "upper") << 
                     " flaw because of magnitude.  Lower: " << m_inst->getLowerFlawMagnitude() << " Upper: " << m_inst->getUpperFlawMagnitude());
          }
        }
        else {
          m_treatAsLowerFlaw = m_inst->hasLowerLevelFlaw() && !m_inst->hasUpperLevelFlaw();
          debugMsg("ThreatDecisionPoint:filter", "Instant is only flawed on the " << (m_treatAsLowerFlaw ? "lower" : "upper") << " level.");
        }
      }
      virtual bool operator()(const std::pair<TransactionId, TransactionId>& p) const {
        return true;
      }
      virtual std::string toString() const {return "DefaultFilter";}
    protected:
      FlowProfile* m_profile;
      LabelStr m_explanation;
      InstantId m_inst;
      bool m_treatAsLowerFlaw;
    };

    class PredecessorNotContributingChoiceFilter : public DefaultChoiceFilter {
    public:
      PredecessorNotContributingChoiceFilter(FlowProfile* profile, const LabelStr& explanation, const InstantId& inst)
        : DefaultChoiceFilter(profile, explanation, inst) {}

      bool operator()(const std::pair<TransactionId, TransactionId>& p) const {
        InstantId inst = InstantId::noId();
        bool contributing = false;

        if(m_treatAsLowerFlaw) {
          if(p.first->isConsumer()) {
            debugMsg("ThreatDecisionPoint:filter:predecessorNot", "Rejecting choice because flaw is lower level and  predecessor is a consumer.");
            return false;
          }
          contributing = m_profile->getEarliestLowerLevelInstant(p.first, inst);
        }
        else {
          if(!p.first->isConsumer()) {
            debugMsg("ThreatDecisionPoint:filter:predecesorNot", "Rejecting choice because flaw is upper level and predecessor is a producer.");
            return false;
          }
          contributing = m_profile->getEarliestUpperLevelInstant(p.first, inst);
        }
        checkError(contributing, "Should always have an instant for transaction " << p.first->toString());
        condDebugMsg(inst->getTime() <= m_inst->getTime(), "ThreatDecisionPoint:filter:predecessorNot",
                     "Rejecting choice because predecessor is contributing at this instant.");
        return inst->getTime() > m_inst->getTime();
      }
      std::string toString() const {return "PredecessorNotContributingFilter";}
    };

    class SuccessorContributingChoiceFilter : public DefaultChoiceFilter {
    public:
      SuccessorContributingChoiceFilter(FlowProfile* profile, const LabelStr& explanation, const InstantId& inst) 
        : DefaultChoiceFilter(profile, explanation, inst) {}

      bool operator()(const std::pair<TransactionId, TransactionId>& p) const {
        InstantId inst = InstantId::noId();
        bool contributing = false;

        if(m_treatAsLowerFlaw) {
          if(!p.second->isConsumer()) {
            debugMsg("ThreatDecisionPoint:filter:successor", "Rejecting choice because flaw is lower level and successor is a producer.");
            return false;
          }
          contributing = m_profile->getEarliestLowerLevelInstant(p.second, inst);
        }
        else {
          if(p.second->isConsumer()) {
            debugMsg("ThreatDecisionPoint:filter:successor", "Rejecting choice because flaw is upper level and successor is a consumer.");
            return false;
          }
          contributing = m_profile->getEarliestUpperLevelInstant(p.second, inst);
        }
        checkError(contributing, "Should always have an instant for transaction " << p.second->toString());
        condDebugMsg(inst->getTime() > m_inst->getTime(), "ThreatDecisionPoint:filter:successor",
                     "Rejecting choice because successor is not contributing at this instant.");
        return inst->getTime() <= m_inst->getTime();
      }
      std::string toString() const {return "SuccessorContributingFilter";}
    };

    class ChoiceComparator {
    public:
      virtual ~ChoiceComparator() {}
      virtual bool operator()(const std::pair<TransactionId, TransactionId>& p1, const std::pair<TransactionId, TransactionId>& p2) const {
        check_error(ALWAYS_FAIL, "This used to be a pure virtual method.");
        return false;
      }
      virtual std::string toString() const {
        check_error(ALWAYS_FAIL, "This used to be a pure virtual method.");
        return false;
      }
      virtual ChoiceComparator* copy() const = 0;
    private:
    };

    class ChoiceOrder {
    public:
      ChoiceOrder() {}
      ChoiceOrder(const ChoiceOrder& other) {
        debugMsg("ThreatDecisionPoint:sort", "Copying the choice order.");
        condDebugMsg(other.m_cmps.empty(), "ThreatDecisionPoint:sort", "Other order has no comparators.");
        for(std::list<ChoiceComparator*>::const_iterator it = other.m_cmps.begin(); it != other.m_cmps.end(); ++it) {
          debugMsg("ThreatDecisionPoint:sort", "Copying " << (*it)->toString());
          m_cmps.push_back((*it)->copy());
          //m_cmps.push_back(*it);
        }
      }
      ~ChoiceOrder() {
        for(std::list<ChoiceComparator*>::iterator it = m_cmps.begin(); it != m_cmps.end(); ++it)
          delete (*it);
        m_cmps.clear();
      }
      bool operator()(const std::pair<TransactionId, TransactionId>& p1, const std::pair<TransactionId, TransactionId>& p2) const {
        debugMsg("ThreatDecisionPoint:sort", "Comparing the following pairs:" << std::endl <<
                 "<" << p1.first->toString() << ", " << p1.second->toString() << ">" << std::endl <<
                 "<" << p2.first->toString() << ", " << p2.second->toString() << ">");
        checkError(!m_cmps.empty(), "No comparators.");
        for(std::list<ChoiceComparator*>::const_iterator it = m_cmps.begin(); it != m_cmps.end(); ++it) {
          ChoiceComparator* cmp = *it;
          check_error(cmp != NULL)

          debugMsg("ThreatDecisionPoint:sort", "Using " << cmp->toString());
          if((*cmp)(p1, p2)) {
            debugMsg("ThreatDecisionPoint:sort", "first < second.");
            return true;
          }
          if((*cmp)(p2, p1)) {
            debugMsg("ThreatDecisionPoint:sort", "second < first");
            return false;
          }
          debugMsg("ThreatDecisionPoint:sort", "first == second.  Checking next comparator.");
        }
        return false;
      }
      void addOrder(ChoiceComparator* cmp) {
        check_error(cmp != NULL);
        debugMsg("ThreatDecisionPoint:sort", "Adding comparator " << cmp->toString());
        m_cmps.push_back(cmp);
      }
    private:
      std::list<ChoiceComparator*> m_cmps;
    };

    class TransactionComparator {
    public:
      virtual ~TransactionComparator() {}
      virtual bool operator()(const TransactionId& t1, const TransactionId& t2) const = 0;
      virtual std::string toString() const = 0;
      virtual TransactionComparator* copy() const = 0;
    private:
    };

    class SwitchComparator : public ChoiceComparator {
    public:
      SwitchComparator(TransactionComparator* cmp, bool predecessor) : ChoiceComparator(), m_cmp(cmp), m_predecessor(predecessor) {}
      ~SwitchComparator(){delete  m_cmp;}
      bool operator()(const std::pair<TransactionId, TransactionId>& p1, const std::pair<TransactionId, TransactionId>& p2) const {
        return (m_predecessor ? (*m_cmp)(p1.first, p2.first) : (*m_cmp)(p1.second, p2.second));
      }
      std::string toString() const {
        return m_cmp->toString() + (m_predecessor ? "Predecessor" : "Successor");
      }
      ChoiceComparator* copy() const {
        return (new SwitchComparator(m_cmp->copy(), m_predecessor));
      }
    private:
      TransactionComparator* m_cmp;
      bool m_predecessor;
    };

    class LeastImpactComparator : public ChoiceComparator {
    public:
      LeastImpactComparator() : ChoiceComparator() {}
      bool operator()(const std::pair<TransactionId, TransactionId>& p1, const std::pair<TransactionId, TransactionId>& p2) const {
        double score1 = std::max(pseudoAbs(p1.first->time()->lastDomain().getLowerBound() - p1.second->time()->lastDomain().getLowerBound()),
                                 pseudoAbs(p1.first->time()->lastDomain().getUpperBound() - p1.second->time()->lastDomain().getUpperBound()));
        double score2 = std::max(pseudoAbs(p2.first->time()->lastDomain().getLowerBound() - p2.second->time()->lastDomain().getLowerBound()),
                                 pseudoAbs(p2.first->time()->lastDomain().getUpperBound() - p2.second->time()->lastDomain().getUpperBound()));

        debugMsg("ThreatDecisionPoint:filter:leastImpact", std::endl <<
                 "<" << p1.first->toString() << ", " << p1.second->toString() << "> score: " << score1 << std::endl <<
                 "<" << p2.first->toString() << ", " << p2.second->toString() << "> score: " << score2);
        return score1 < score2; 
      }
      std::string toString() const {
        return "LeastImpactComparator";
      }
      ChoiceComparator* copy() const {
        return (new LeastImpactComparator());
      }
    private:
      inline double pseudoAbs(double value) const {
        return (value < 0 ? 0 : value);
      }
    };

    class EarliestTransactionComparator : public TransactionComparator {
    public:
      bool operator()(const TransactionId& t1, const TransactionId& t2) const {
        return t1->time()->lastDomain().getLowerBound() < t2->time()->lastDomain().getLowerBound();
      }
      std::string toString() const {return "earliest";}
      TransactionComparator* copy() const {return new EarliestTransactionComparator();}
    };

    class LatestTransactionComparator : public TransactionComparator {
    public:
      bool operator()(const TransactionId& t1, const TransactionId& t2) const {
        debugMsg("ThreatDecisionPoint:sort:latest", "Comparing upper bounds of timepoints for " << t1->toString() << " and " << t2->toString());
        return t1->time()->lastDomain().getUpperBound() > t2->time()->lastDomain().getUpperBound();
      }
      std::string toString() const {return "latest";}
      TransactionComparator* copy() const {return new LatestTransactionComparator();}
    };

    class LongestTransactionComparator : public TransactionComparator {
    public:
      bool operator()(const TransactionId& t1, const TransactionId& t2) const {
        return 
          (t1->time()->lastDomain().getUpperBound() - t1->time()->lastDomain().getLowerBound()) > 
          (t2->time()->lastDomain().getUpperBound() - t2->time()->lastDomain().getLowerBound());
      }
      std::string toString() const {return "longest";}
      TransactionComparator* copy() const {return new LongestTransactionComparator();}
    };

    class ShortestTransactionComparator : public TransactionComparator {
    public:
      bool operator()(const TransactionId& t1, const TransactionId& t2) const {
        return 
          (t1->time()->lastDomain().getUpperBound() - t1->time()->lastDomain().getLowerBound()) < 
          (t2->time()->lastDomain().getUpperBound() - t2->time()->lastDomain().getLowerBound());
      }
      std::string toString() const {return "shortest";}
      TransactionComparator* copy() const {return new ShortestTransactionComparator();}
    };
    
    class AscendingKeyTransactionComparator : public TransactionComparator {
    public:
      bool operator()(const TransactionId& t1, const TransactionId& t2) const {
        return t1->time()->getKey() < t2->time()->getKey();
      }
      std::string toString() const {return "ascendingKey";}
      TransactionComparator* copy() const {return new AscendingKeyTransactionComparator();}
    };

    class DescendingKeyTransactionComparator : public TransactionComparator {
    public:
      bool operator()(const TransactionId& t1, const TransactionId& t2) const {
        return t1->time()->getKey() > t2->time()->getKey();
      }
      std::string toString() const {return "descendingKey";}
      TransactionComparator* copy() const {return new DescendingKeyTransactionComparator();}
    };
    

    bool ThreatDecisionPoint::test(const EntityId& entity) {
      return InstantId::convertable(entity);
    }

    /**
       configuration:
       filter="predecessorNot" will filter out decisions except those whose predecessor is not contributing at this instant
       filter="successor" will filter out decisions except those whose successor is contributing at this instant
       filter="both" applies both filters
       filter="none" applies neither (default)
       constraint="precedesOnly" will create only precedence constraint (default)
       constraint="precedesFirst" will create precedence constraints before concurrency constraints
       constraint="concurrentOnly" will create only concurrency constraints
       constraint="concurrentFirst" will create concurrency constraints before precedence constraints
       iterate="pairFirst" if it creates both types of constraints, it will iterate over the set of pairs before switching constraint type
       iterate="constraintFirst" if it creates both types of constraints, it will create both types of constraints before moving to the next pair

       these can be combined with commas.  orderings after the first will be used to break ties, left to right.
       order="earliestPredecessor" will order choices in ascending predecessor time
       order="latestPredecessor" will order choices in descending predecessor time
       order="longestPredecessor" will order choices in descending interval size
       order="shortestPredecessor" will order choices in ascending interval size
       order="ascendingKeyPredecessor" will order choices in ascending key of the time variable of the predecessor (default)
       order="descendingKeyPredecessor" will order choices in descending key of the time variable of the predecessor
       order="earliestSuccessor" will order choices in ascending successor time
       order="latestSuccessor" will order choices in descending successor time
       order="longestSuccessor" will order choices in descending interval size
       order="shortestSuccessor" will order choices in ascending interval size
       order="ascendingKeySuccessor" will order choices in ascending key of the time variable of the successor (default)
       order="descendingKeySuccessor" will order choices in descending key of the time variable of the successor
       order="leastImpact" will order choices by last estimated temporal impact
       
     */
    ThreatDecisionPoint::ThreatDecisionPoint(const DbClientId& client, const InstantId& flawedInstant, const TiXmlElement& configData, const LabelStr& explanation) 
      : DecisionPoint(client, flawedInstant->getKey(), explanation), m_flawedInstant(flawedInstant), m_index(0) {
      m_instTime = m_flawedInstant->getTime();
      m_resName = m_flawedInstant->getProfile()->getResource()->getName();

      //process the configuration data for ordering choices
      //store the filter, defaulting to "none"
      m_filter = (configData.Attribute("filter") == NULL ? "none" : configData.Attribute("filter"));

      //store the order, with ascendingKeyPredecessor,ascendingKeySuccessor as the universal tie-breaker
      m_order = (configData.Attribute("order") == NULL ? "" : configData.Attribute("order"));
      if(m_order.size() > 0)
        m_order += ",";
      m_order += "ascendingKeyPredecessor,ascendingKeySuccessor";

      //store the names of the constraints to get created
      if(configData.Attribute("constraint") == NULL)
        m_constraintNames.push_back("precedes");
      else {
        std::string order = configData.Attribute("constraint");
        if(order == "precedesOnly" || order == "precedesFirst")
          m_constraintNames.push_back("precedes");
        if(order == "concurrentOnly" || order == "concurrentFirst" || order == "precedesFirst")
          m_constraintNames.push_back("concurrent");
        if(order == "concurrentFirst")
          m_constraintNames.push_back("precedes");
      }
      check_error(m_constraintNames.size() == 1 || m_constraintNames.size() == 2, "Expected one or two constraint names.");
      m_constraintIt = m_constraintNames.begin();

      m_constraintOrder = (configData.Attribute("iterate") == NULL ? "pairFirst" : configData.Attribute("iterate"));
      checkError(m_constraintOrder == "pairFirst" || m_constraintOrder == "constraintFirst", "Expected 'pairFirst' or 'constraintFirst' for iterate attribute.");
    }

    ThreatDecisionPoint::~ThreatDecisionPoint() {}

    void ThreatDecisionPoint::createFilter(ChoiceFilters& filters, const std::string& filter, ProfileId& profile) {
      checkError(filter == "none" || filter == "predecessorNot" || filter == "successor" || filter == "both",
                 "Unknown filter attribute '" << filter << "'");
      if(filter == "successor" || filter == "both")
        filters.addFilter(new SuccessorContributingChoiceFilter((FlowProfile*) profile, getExplanation(), m_flawedInstant));
      if(filter == "predecessorNot" || filter == "both")
        filters.addFilter(new PredecessorNotContributingChoiceFilter((FlowProfile*) profile, getExplanation(), m_flawedInstant));
      filters.addFilter(new DefaultChoiceFilter((FlowProfile*) profile, getExplanation(), m_flawedInstant));
    }

    std::string ThreatDecisionPoint::toString() const {
      std::stringstream os;
      os << "INSTANT=" << m_instTime << " on " << m_resName.toString() << " : ";

      TransactionId predecessor = m_choices[m_index].first;
      TransactionId successor = m_choices[m_index].second;
      os << "  DECISION (" << m_index << " of CHOICES) " 
         << predecessor->toString() 
         << " to be before " << successor->toString()
         << " : ";

      os << "  CHOICES ";
      for(unsigned int i = 0; i < m_choiceCount; i++)
        os << " : " << toString(m_choices[i]);
      return os.str();
    }

    std::string ThreatDecisionPoint::toString(const std::pair<TransactionId, TransactionId>& choice) const {
      std::stringstream os;
      os << "<" << choice.first->toString() << " *** " << choice.second->toString() << ">";
      return os.str();
    }
    
    void ThreatDecisionPoint::handleInitialize() {
      check_error(m_flawedInstant.isValid());
      m_flawedInstant->getProfile()->getResource()->getOrderingChoices(m_flawedInstant, m_choices);

      //filter based on the configuration
      ChoiceFilters filter;
      createFilter(filter, m_filter, const_cast<ProfileId&>(m_flawedInstant->getProfile()));
      //order based ont he configuration
      ChoiceOrder order;
      createOrder(order);

      std::set<std::pair<TransactionId, TransactionId>, ChoiceOrder> sort(order);

      std::insert_iterator<std::set<std::pair<TransactionId, TransactionId>, ChoiceOrder > > choiceIt = 
        inserter(sort, sort.begin());
//       std::back_insert_iterator<std::vector<std::pair<TransactionId, TransactionId> > > choiceIt =
//         std::back_inserter(m_choices);
      
      for(std::vector<std::pair<TransactionId, TransactionId> >::iterator it = m_choices.begin();
          it != m_choices.end(); ++it) {
        if(filter(*it))
          *choiceIt++ = *it;
      }
      
      //       std::sort(m_choices.begin(), m_choices.end(), order);
      m_choices.clear();
      m_choices.insert(m_choices.end(), sort.begin(), sort.end());
      m_choiceCount = m_choices.size();
      debugMsg("ThreatDecisionPoint:handleInitialize", "Found " << m_choiceCount << " choices after filtering.");
      //for some reason, std::sort copies the comparator object, so I'm going to try this with a set.
      //std::sort(m_choices.begin(), m_choices.end(), order);
      m_flawedInstant = InstantId::noId();
    }

    bool ThreatDecisionPoint::hasNext() const {
      return m_index < m_choiceCount && m_constraintIt != m_constraintNames.end();
    }

    bool ThreatDecisionPoint::canUndo() const {
      return DecisionPoint::canUndo() && m_constr.isValid();
    }

    void ThreatDecisionPoint::handleExecute() {
      check_error(m_constr.isNoId());
      checkError(m_index < m_choiceCount, "Tried to execute past available choices:" << m_index << ">=" << m_choiceCount);
      TransactionId predecessor = m_choices[m_index].first;
      TransactionId successor = m_choices[m_index].second;
      debugMsg("SolverDecisionPoint:handleExecute", "For " << m_instTime << " on " << m_resName.toString() << ", assigning " << 
               predecessor->toString() << " to be before " << successor->toString() << " because of " << getExplanation().toString() << ".");
      m_constr = m_client->createConstraint((*m_constraintIt).c_str(), makeScope(predecessor->time(), successor->time()));
    }

    void ThreatDecisionPoint::handleUndo() {
      debugMsg("SolverDecisionPoint:handleUndo", "Retracting ordering decision on " << m_instTime << " on " <<
               m_resName.toString());
      check_error(m_constr.isValid());
      m_constr->discard();
      m_constr = ConstraintId::noId();
      //advance constraints before advancing pairs
      if(m_constraintOrder == "constraintFirst") {
        ++m_constraintIt;
        if(m_constraintIt == m_constraintNames.end()) {
          m_index++;
          m_constraintIt = m_constraintNames.begin();
        }
      }
      else if(m_constraintOrder == "pairFirst") {
        m_index++;
        if(m_index == m_choices.size()) {
          m_index = 0;
          m_constraintIt++;
        }
      }
      //m_index++
    }

    //this parsing could be tightened up a bit more.
    void ThreatDecisionPoint::createOrder(ChoiceOrder& order) {
      check_error(m_order.size() > 0, "Empty choice ordering.  Bizarre.");

      std::string::size_type curPos = 0;
      while(curPos != std::string::npos) {
        std::string::size_type nextPos = m_order.find(',', curPos);
        std::string orderStr = m_order.substr(curPos, (nextPos == std::string::npos ? nextPos : nextPos - curPos));
        if(orderStr == "leastImpact") {
          order.addOrder(new LeastImpactComparator());
        }
        else {
          bool predecessor = false;

          if(orderStr.find("Predecessor") != std::string::npos)
            predecessor = true;
          else if(orderStr.find("Successor") != std::string::npos)
            predecessor = false;
          else {
            checkError(ALWAYS_FAIL, "Expected a 'Predecessor' or 'Successor' order.");
          }
          ChoiceComparator* cmp=NULL;
          if(orderStr.find("earliest") != std::string::npos)
            cmp = new SwitchComparator(new EarliestTransactionComparator(), predecessor);
          else if(orderStr.find("latest") != std::string::npos)
            cmp = new SwitchComparator(new LatestTransactionComparator(), predecessor);
          else if(orderStr.find("longest") != std::string::npos)
            cmp = new SwitchComparator(new LongestTransactionComparator(), predecessor);
          else if(orderStr.find("shortest") != std::string::npos)
            cmp = new SwitchComparator(new ShortestTransactionComparator(), predecessor);
          else if(orderStr.find("ascendingKey") != std::string::npos)
            cmp = new SwitchComparator(new AscendingKeyTransactionComparator(), predecessor);
          else if(orderStr.find("descendingKey") != std::string::npos)
            cmp = new SwitchComparator(new DescendingKeyTransactionComparator(), predecessor);
          else {
            checkError(ALWAYS_FAIL, "Unknown choice order '" << orderStr);
          }
          order.addOrder(cmp);
        }
        curPos = (nextPos == std::string::npos ? nextPos : nextPos + 1);
      }
    }
  }
}
