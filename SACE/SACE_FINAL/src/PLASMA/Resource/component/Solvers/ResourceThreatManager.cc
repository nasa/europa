#include "ResourceThreatManager.hh"
#include "Instant.hh"
#include "Profile.hh"
#include "Resource.hh"
#include "PlanDatabase.hh"
#include "Context.hh"

#ifdef ABSOLUTE
#undef ABSOLUTE
#endif

namespace EUROPA {
  using namespace SOLVERS;
    /**
       config attributes are "first", "second", "third".  options are
       greatest/least, earliest/latest, upper/lower
     */

//     class TimeComparator {
//     public:
//       bool operator()(const InstantId& insta, const InstantId& instb) {
//         return insta->getTime() < instb->getTime();
//       }
//     };

//     class FlawMagnitudeComparator {
//     public:
//       bool operator()(const InstantId& insta, const InstantId& instb) {
//         return insta->
//       }
//     };

    class InstantComparator {
    public:
      enum FlawDirection {
        ABSOLUTE = 0,
        UPPER,
        LOWER
      };

      virtual ~InstantComparator() {}
      virtual bool operator()(const InstantId& a, const InstantId& b) const = 0;
      virtual std::string toString() const = 0;
      virtual InstantComparator* copy() const = 0;
    };

    DecisionOrder::~DecisionOrder() {
      for(std::list<InstantComparator*>::iterator it = m_cmps.begin(); it != m_cmps.end(); ++it) {
        delete *it;
      }
      m_cmps.clear();
    }

    DecisionOrder::DecisionOrder(const DecisionOrder& other) {
      for(std::list<InstantComparator*>::const_iterator it = other.m_cmps.begin(); it != other.m_cmps.end(); ++it) {
        m_cmps.push_back((*it)->copy());
      }
    }

    //returns true if a is better than b
    bool DecisionOrder::operator()(const InstantId& a, const InstantId& b, LabelStr& explanation) const {
      check_error(!m_cmps.empty());
      check_error(a.isValid() && b.isValid());
      debugMsg("ResourceThreatManager:betterThan", "Comparing instant " << a->getTime() << " on " << a->getProfile()->getResource()->toString() <<
               " to " << b->getTime() << " on " << b->getProfile()->getResource()->toString());
      for(std::list<InstantComparator*>::const_iterator it = m_cmps.begin(); it != m_cmps.end(); ++it) {
        InstantComparator* cmp = *it;
        check_error(cmp != NULL);
        debugMsg("ResourceThreatManager:betterThan", "Using " << cmp->toString());
        if((*cmp)(a, b)) {
          debugMsg("ResourceThreatManager:betterThan", "a better than b");
          explanation = cmp->toString();
          return true;
        }
        else if((*cmp)(b, a)) {
          debugMsg("ResourceThreatManager:betterThan", "b better than a");
          return false;
        }
        debugMsg("ResourceThreatManager:betterThan", "a == b.  Checking next comparator.");
      }
      return false;
    }

    void DecisionOrder::addOrder(InstantComparator* cmp) {
      check_error(cmp != NULL);
      debugMsg("ResourceThreatManager:betterThan", "Adding comparator " << cmp->toString());
      m_cmps.push_back(cmp);
    }

    class EarliestInstantComparator : public InstantComparator {
    public:
      bool operator()(const InstantId& a, const InstantId& b) const {
        return a->getTime() < b->getTime();
      }
      std::string toString() const {return "earliest";}
      InstantComparator* copy() const {return new EarliestInstantComparator();}
    };

    class LatestInstantComparator : public InstantComparator {
    public:
      bool operator()(const InstantId& a, const InstantId& b) const {
        return a->getTime() > b->getTime();
      }
      std::string toString() const {return "latest";}
      InstantComparator* copy() const {return new LatestInstantComparator();}
    };

    class MostInstantComparator : public InstantComparator {
    public:
      MostInstantComparator(const FlawDirection& dir = ABSOLUTE) : InstantComparator(), m_dir(dir) {}
      bool operator()(const InstantId& a, const InstantId& b) const {
        if(m_dir == UPPER) {
          if(a->hasUpperLevelFlaw()) {
            if(b->hasUpperLevelFlaw()) {
              return a->getUpperFlawMagnitude() > b->getUpperFlawMagnitude();
            }
            else
              return true;
          }
          else
            return false;
        }
        else if(m_dir == LOWER) {
          if(a->hasLowerLevelFlaw()) {
            if(b->hasLowerLevelFlaw())
              return a->getLowerFlawMagnitude() > b->getLowerFlawMagnitude();
            else
              return true;
          }
          else
            return false;
        }
        else {
          double flawA = 0, flawB = 0;
          if(a->hasUpperLevelFlaw())
            flawA = a->getUpperFlawMagnitude();
          if(a->hasLowerLevelFlaw())
            flawA = std::max(flawA, a->getLowerFlawMagnitude());
          if(b->hasUpperLevelFlaw())
            flawB = b->getUpperFlawMagnitude();
          if(b->hasLowerLevelFlaw())
            flawB = std::max(flawB, b->getLowerFlawMagnitude());
          return flawA > flawB;
        }
        check_error(ALWAYS_FAIL);
        return false;
      }
      std::string toString() const {return std::string("mostFlawed") + (m_dir == ABSOLUTE ? "Absolute" : (m_dir == UPPER ? "Upper" : "Lower"));}
      InstantComparator* copy() const {return new MostInstantComparator(m_dir);}
    private:
      FlawDirection m_dir;
    };

    class LeastInstantComparator : public InstantComparator {
    public:
      LeastInstantComparator(const FlawDirection& dir = ABSOLUTE) : InstantComparator(), m_dir(dir) {}
      bool operator()(const InstantId& a, const InstantId& b) const {
        if(m_dir == UPPER) {
          if(a->hasUpperLevelFlaw()) {
            if(b->hasUpperLevelFlaw()) {
              return a->getUpperFlawMagnitude() < b->getUpperFlawMagnitude();
            }
            else
              return true;
          }
          else
            return false;
        }
        else if(m_dir == LOWER) {
          if(a->hasLowerLevelFlaw()) {
            if(b->hasLowerLevelFlaw())
              return a->getLowerFlawMagnitude() < b->getLowerFlawMagnitude();
            else
              return true;
          }
          else
            return false;
        }
        else {
          double flawA = PLUS_INFINITY, flawB = PLUS_INFINITY;
          if(a->hasUpperLevelFlaw())
            flawA = a->getUpperFlawMagnitude();
          if(a->hasLowerLevelFlaw())
            flawA = std::min(flawA, a->getLowerFlawMagnitude());
          if(b->hasUpperLevelFlaw())
            flawB = b->getUpperFlawMagnitude();
          if(b->hasLowerLevelFlaw())
            flawB = std::min(flawB, b->getLowerFlawMagnitude());
          return flawA < flawB;
        }
        check_error(ALWAYS_FAIL);
        return false;
      }
      std::string toString() const {return std::string("leastFlawed") + (m_dir == ABSOLUTE ? "Absolute" : (m_dir == UPPER ? "Upper" : "Lower"));}
      InstantComparator* copy() const {return new LeastInstantComparator(m_dir);}
    private:
      FlawDirection m_dir;
    };

    class UpperInstantComparator : public InstantComparator {
    public:
      bool operator()(const InstantId& a, const InstantId& b) const {
        return a->hasUpperLevelFlaw() && !b->hasUpperLevelFlaw();
      }
      std::string toString() const {return "upperLevelFlaw";}
      InstantComparator* copy() const {return new UpperInstantComparator();}
    };

    class LowerInstantComparator : public InstantComparator {
    public:
      bool operator()(const InstantId& a, const InstantId& b) const {
        return a->hasLowerLevelFlaw() && !b->hasLowerLevelFlaw();
      }
      std::string toString() const {return "lowerLevelFlaw";}
      InstantComparator* copy() const {return new LowerInstantComparator();}
    };

    //at some point, this should take data about ordering choices by earliest/latest, most/least flawed, and most/least transactions
    ResourceThreatManager::ResourceThreatManager(const TiXmlElement& configData) : FlawManager(configData), m_preferUpper(false), m_preferLower(false) {
      std::string order = (configData.Attribute("order") == NULL ? "lower,most,earliest" : configData.Attribute("order"));
      std::string::size_type curPos = 0;
      InstantComparator::FlawDirection dir = InstantComparator::ABSOLUTE;
      while(curPos != std::string::npos) {
        std::string::size_type nextPos = order.find(',', curPos);
        std::string orderStr = order.substr(curPos, (nextPos == std::string::npos ? nextPos : nextPos - curPos));
        //InstantComparator* cmp;
        if(orderStr == "earliest")
          m_order.addOrder(new EarliestInstantComparator());
        else if(orderStr == "latest")
          m_order.addOrder(new LatestInstantComparator());
        else if(orderStr == "lower") {
          m_order.addOrder(new LowerInstantComparator());
          dir = InstantComparator::LOWER;
          m_preferLower = true;
        }
        else if(orderStr == "upper") {
          m_order.addOrder(new UpperInstantComparator());
          dir = InstantComparator::UPPER;
          m_preferUpper = true;
        }
        else if(orderStr == "most") {
          m_order.addOrder(new MostInstantComparator(dir));
        }
        else if(orderStr == "least") {
          m_order.addOrder(new LeastInstantComparator(dir));
        }
        else {
          checkError(ALWAYS_FAIL, "Unknown decision order '" << orderStr << "'");
        }
        curPos = (nextPos == std::string::npos ? nextPos : nextPos + 1);
        checkError(!m_preferUpper || !m_preferLower, "Can't prefer both upper and lower level flaws.");
      }
    }

    ResourceThreatManager::~ResourceThreatManager(){}

    //re-impelement this to delegate to FlawManager::staticMatch
    bool ResourceThreatManager::staticMatch(const EntityId& entity) {
      return !InstantId::convertable(entity);
    }

    //there may be no dynamic information here
    bool ResourceThreatManager::dynamicMatch(const EntityId& entity) {
      return staticMatch(entity);
    }

    void ResourceThreatManager::handleInitialize() {
    }

    //this should use data from the constructor
    bool ResourceThreatManager::betterThan(const EntityId& a, const EntityId& b, LabelStr& explanation) {
      check_error(InstantId::convertable(a) && InstantId::convertable(b));
      InstantId instA(a);
      InstantId instB(b);
      //return instA->getTime() < instB->getTime();
      return m_order(a, b, explanation);
    }

    std::string ResourceThreatManager::toString(const EntityId& entity) const {
      check_error(InstantId::convertable(entity));
      InstantId inst(entity);
      std::stringstream os;
      os << "THREAT: " << inst->getTime() << " on " << inst->getProfile()->getResource()->toString();
      return os.str();
    }

    class ThreatIterator : public FlawIterator {
    public:
      ThreatIterator(ResourceThreatManager& manager) : FlawIterator(manager) {
        const ObjectSet& objs = manager.getPlanDatabase()->getObjects();
        for(ObjectSet::const_iterator it = objs.begin(); it != objs.end(); ++it) {
          ObjectId obj(*it);
          check_error(obj.isValid());
          debugMsg("ThreatIterator:ThreatIterator", "Checking to see if " << obj->toString() << " is a resource...");
          if(!ResourceId::convertable(obj))
            continue;
          ResourceId res(obj);
          std::vector<InstantId> temp;
          debugMsg("ThreatIterator:ThreatIterator", "Resource!  Getting flawed instants...");
          res->getFlawedInstants(temp);
          m_flawedInstants.insert(m_flawedInstants.end(), temp.begin(), temp.end());
        }
        debugMsg("ThreatIterator:ThreatIterator", "Got " << m_flawedInstants.size() << " total instants.");
        m_it = m_flawedInstants.begin();
        advance();
        condDebugMsg(done(), "ThreatIterator:ThreatIterator", "Advanced to the end right away.");
      }
    protected:
    private:
      const EntityId nextCandidate() {
        EntityId candidate;
        if(m_it != m_flawedInstants.end()) {
          candidate = *m_it;
          ++m_it;
        }
        return candidate;
      }
      std::vector<InstantId> m_flawedInstants;
      std::vector<InstantId>::iterator m_it;
    };

    IteratorId ResourceThreatManager::createIterator() {
      return (new ThreatIterator(*this))->getId();
    }
}
