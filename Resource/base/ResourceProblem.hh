#ifndef _H_ResourceProblem
#define _H_ResourceProblem

#include "ResourceDefs.hh"

namespace EUROPA {

  /** ProblemId is defined in this file and not in ResourceDefs.hh because it is
   *  not supposed to be visible outside Resource. Only Flaw and Violation, which
   *  derive from Problem, are visible. Their Ids are defined in ResourceDefs.hh 
   */
  class ResourceProblem;
  typedef Id<ResourceProblem> ResourceProblemId;

  const static char* labels[] = { "ProductionRateExceeded", 
                                  "ConsumptionRateExceeded", 
                                  "ProductionSumExceeded",
                                  "ConsumptionSumExceeded",
                                  "LevelTooHigh",
				  "LevelTooLow",
				  "NoWayOutConsumption",
				  "NoWayOutProduction"
  };

  /**
   * @class ResourceProblem
   * @brief The parent class for Violation and Flaw. Only Violation and Flaw are
   * visible outside Resource
   */
  class ResourceProblem {
  public:
	/** Only LevelTooHigh and LevelTooLow are currently used by Flaw,
		Violation uses all types */
    enum Type { NoProblem = -1,
                ProductionRateExceeded = 0, /**< Excessive production in an instant. */
                ConsumptionRateExceeded, /**< Excessive consumption in an instant. */
                ProductionSumExceeded, /**< Total production for the resource exceded. */
                ConsumptionSumExceeded, /**< Total consumption fo rthe resource exceded. */
                LevelTooHigh, /**< Level is outside the upper limit, taking into account remaining possible production or consumption. */
                LevelTooLow, /**< Level is outside the lower limit, taking into account remaining possible production or consumption. */
		NoWayOutConsumption,
		NoWayOutProduction
	};

    static const char* getString(Type t) { return labels[t]; }

    const char* getString() const { return labels[m_type]; }

    /**
     * @brief Destructor
     */
    virtual ~ResourceProblem();
	
    /**
     * @brief Accessor for the Instant at which this Problem is discovered.
     * @return Reference to the instant.
     */
    const InstantId& getInstant() const {return m_instant;}

    /**
     * @brief Accessor for the type of Problem.
     * @return The type of problem
     */
    const Type getType() const {return m_type;}

    /**
     * @brief Output details. Primarily for debugging.
     * @arg The target output stream
	 * I do not intend to override this method, but just in case 
	 * let it be virtual.
     */
    virtual void print(std::ostream& os);

  protected:
    friend class Instant;

    /**
     * @brief Constructor always attached to an Instant
     * @arg The type of Problem discovered.
     * @arg The Instant in which the Problem was discovered.
     */
    ResourceProblem(Type type, const InstantId& instant);
    ResourceProblem(Type type);
    ResourceProblem(); // No Impl

    /**
     * @brief Accessor for the Id of the Violation given the pointer.
     * @return Reference to the instant.
     */
    const ResourceProblemId& getId() const {return m_id;}

    Type m_type;
    InstantId m_instant;
    ResourceProblemId m_id;	
  };
}

#endif
