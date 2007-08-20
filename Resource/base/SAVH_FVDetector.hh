#ifndef _H_SAVH_FVDetector
#define _H_SAVH_FVDetector

/**
 * @file SAVH_FVDetector.hh
 * @author Michael Iatauro
 * @brief Defines a base class for detecting flaws and violations in a profile, given bounds from some resource.
 * @date March, 2006
 * @ingroup Resource
 */

#include "SAVH_ResourceDefs.hh"
#include "SAVH_Resource.hh"

namespace EUROPA {
  namespace SAVH {

    /**
     * @class FVDetector
     * @brief The base class for detecting flaws and violations.
     *
     * Because the different classes of Resource have different definitions of what a flaw or a violation is,
     * but all of said classes have them, this base class defines the interface for detecting flaws and violations
     * and informing the Resource about it.
     */
    class FVDetector {
    public:
      /**
       * @brief Constructor
       * @param res The Resource to be notified when a flaw or violation is detected.
       */
      FVDetector(const ResourceId res) : m_id(this), m_res(res) {}

      virtual ~FVDetector() {m_id.remove();}

      /**
       * @brief Initialize a detection run with the given instant data
       * @param inst The source of the level data.
       */
      virtual void initialize(const InstantId inst) {if(m_res.isValid()) m_res->resetViolations(inst);}

      /**
       * @brief Initialize a detection run with no data.  Used when the first Instant in the recalculation interval is the first Instant.
       */
      virtual void initialize() {if(m_res.isValid()) m_res->resetViolations();}

      /**
       * @brief Detect flaws and violations at an instant.
       * @param inst The instant at which to detect a flaw or violation.
       * @return true if there should be no further level computation or flaw detection, false otherwise
       */
      virtual bool detect(const InstantId inst) = 0;

      FVDetectorId getId() {return m_id;}

      const ResourceId& getResource() const {return m_res;}
    protected:
      friend class Profile;

      /**
       * @brief Inform the FVDetector (and, ultimately, the Resource) that an Instant has been removed.
       */
      void notifyDeleted(const InstantId inst) {if(m_res.isValid()) m_res->notifyDeleted(inst);}

      /**
       * @brief Inform the Resource that there is a violation at an Instant.
       */
      void notifyOfViolation(const InstantId inst, ResourceProblem::Type problem) {if(m_res.isValid()) m_res->notifyViolated(inst,problem);}

      void notifyNoLongerViolated(const InstantId inst) {if(m_res.isValid()) m_res->notifyNoLongerViolated(inst);}      
      
      /**
       * @brief Inform the Resource that there is a flaw at an Instant.
       */
      void notifyOfFlaw(const InstantId inst) {if(m_res.isValid()) m_res->notifyFlawed(inst);}

      void notifyNoLongerFlawed(const InstantId inst) {if(m_res.isValid()) m_res->notifyNoLongerFlawed(inst);}
      
      bool allowViolations() const;
      
    private:
      FVDetectorId m_id;
      ResourceId m_res;
    };
    
    class FVDetectorFactory {
    public:
      static FVDetectorId createInstance(const LabelStr& name, const ResourceId res);
      const LabelStr& getName() const {return m_name;}
    protected:
      virtual ~FVDetectorFactory(){}
      static void registerFactory(const LabelStr& name, FVDetectorFactory* factory);
      virtual FVDetectorId create(const ResourceId res) const = 0;
      FVDetectorFactory(const LabelStr& name) : m_name(name) {registerFactory(m_name, this);}
    private:
      static std::map<double, FVDetectorFactory*>& factoryMap();
      const LabelStr m_name;
    };

    template<class FVDetectorType>
    class ConcreteFVDetectorFactory : public FVDetectorFactory {
    public:
      ConcreteFVDetectorFactory(const EUROPA::LabelStr& name) : FVDetectorFactory(name) {}
    protected:
    private:
      FVDetectorId create(const ResourceId res) const {return (new FVDetectorType(res))->getId();}
    };

#define REGISTER_FVDETECTOR(CLASS, NAME) (new EUROPA::SAVH::ConcreteFVDetectorFactory<CLASS>(EUROPA::LabelStr(#NAME)));
  }
}

#endif
