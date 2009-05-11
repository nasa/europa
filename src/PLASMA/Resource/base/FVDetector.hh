#ifndef _H_FVDetector
#define _H_FVDetector

/**
 * @file FVDetector.hh
 * @author Michael Iatauro
 * @brief Defines a base class for detecting flaws and violations in a profile, given bounds from some resource.
 * @date March, 2006
 * @ingroup Resource
 */

#include "ResourceDefs.hh"
#include "Resource.hh"
#include "Factory.hh"

namespace EUROPA {

    class FVDetectorFactoryMgr;
    typedef Id<FVDetectorFactoryMgr> FVDetectoryFactoryMgrId;

    /**
     * @class FVDetector
     * @brief The base class for detecting flaws and violations.
     *
     * Because the different classes of Resource have different definitions of what a flaw or a violation is,
     * but all of said classes have them, this base class defines the interface for detecting flaws and violations
     * and informing the Resource about it.
     */
    class FVDetector : public FactoryObj {
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

      FVDetectorId& getId() {return m_id;}

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
      void notifyOfViolation(const InstantId inst, Resource::ProblemType problem) {if(m_res.isValid()) m_res->notifyViolated(inst,problem);}

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

    class FVDetectorArgs : public FactoryArgs
    {
    public:
        const ResourceId& resource;
        FVDetectorArgs(const ResourceId& r) : resource(r) {}
    };

    template<class FVDetectorType>
    class FVDetectorFactory : public Factory
    {
    public:
      FVDetectorFactory(const EUROPA::LabelStr& name) : Factory(name) {}

      virtual EUROPA::FactoryObjId& createInstance(const EUROPA::FactoryArgs& fa) {
          const FVDetectorArgs& args = (const FVDetectorArgs&)fa;
          return (EUROPA::FactoryObjId&)(new FVDetectorType(args.resource))->getId();
      }
    };

#define REGISTER_FVDETECTOR(MGR, CLASS, NAME) \
    (MGR->registerFactory((new EUROPA::FVDetectorFactory<CLASS>(EUROPA::LabelStr(#NAME)))->getId()));
}

#endif
