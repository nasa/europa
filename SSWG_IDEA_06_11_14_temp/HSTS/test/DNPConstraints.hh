/**
 * @file DNPConstraints.hh
 * @author Will Edgington
 * @date December 2004
 * @brief Declarations of the constraint functions specific to the DNP domain.
 */

#include "TestSupport.hh"
#include "Utils.hh"
#include "LabelStr.hh"
#include "Variable.hh"
#include "Constraints.hh"
#include "AbstractDomain.hh"
#include "EnumeratedDomain.hh"
#include "IntervalIntDomain.hh"
#include "SymbolDomain.hh"

/**
 * @class BOUNDS_RECORD_END_STORAGE
 * @brief Calculate storage used on the recorder at the end of a record.
 * @note From the Europa DNP model.
 */
class BOUNDS_RECORD_END_STORAGE : public Constraint {
public:
  BOUNDS_RECORD_END_STORAGE(const LabelStr& name,
                            const LabelStr& propagatorName,
                            const ConstraintEngineId& constraintEngine,
                            const std::vector<ConstrainedVariableId>& variables);

  ~BOUNDS_RECORD_END_STORAGE() {
  }

  void handleExecute();
};

/**
 * @class BOUNDS_RECORD_START_STORAGE
 * @brief Calculate storage used on the recorder at the start of a record.
 * @note From the Europa DNP model.
 */
class BOUNDS_RECORD_START_STORAGE : public Constraint {
public:
  BOUNDS_RECORD_START_STORAGE(const LabelStr& name,
                              const LabelStr& propagatorName,
                              const ConstraintEngineId& constraintEngine,
                              const std::vector<ConstrainedVariableId>& variables);

  ~BOUNDS_RECORD_START_STORAGE() {
  }

  void handleExecute();
};

/**
 * @class COMPUTE_PLAYBACK_DURATION
 * @brief Calculate how long a play back will take.
 * @note From the Europa DNP model.
 */
class COMPUTE_PLAYBACK_DURATION : public Constraint {
public:
  COMPUTE_PLAYBACK_DURATION(const LabelStr& name,
                            const LabelStr& propagatorName,
                            const ConstraintEngineId& constraintEngine,
                            const std::vector<ConstrainedVariableId>& variables);

  ~COMPUTE_PLAYBACK_DURATION() {
  }
  
  void handleExecute();
};

/**
 * @class BOUNDS_PLAYBACK_START_STORAGE
 * @brief Calculate the storage on the recorder at the start of a play back.
 * @note From the Europa DNP model.
 */
class BOUNDS_PLAYBACK_START_STORAGE : public Constraint {
public:
  BOUNDS_PLAYBACK_START_STORAGE(const LabelStr& name,
                                const LabelStr& propagatorName,
                                const ConstraintEngineId& constraintEngine,
                                const std::vector<ConstrainedVariableId>& variables);

  ~BOUNDS_PLAYBACK_START_STORAGE() {
  }
  
  void handleExecute();
};

/**
 * @class BOUNDS_PLAYBACK_END_STORAGE
 * @brief Calculate storage on recorder at end of play back.
 * @note From the Europa DNP model.
 */
class BOUNDS_PLAYBACK_END_STORAGE : public Constraint {
public:
  BOUNDS_PLAYBACK_END_STORAGE(const LabelStr& name,
                              const LabelStr& propagatorName,
                              const ConstraintEngineId& constraintEngine,
                              const std::vector<ConstrainedVariableId>& variables);

  ~BOUNDS_PLAYBACK_END_STORAGE() {
  }
  
  void handleExecute();
};

/**
 * @class FIGURE_EARLIER_OP_IDS
 * @brief Calculate op Ids that could occur earlier: 0 <= A < B, but B never modified.
 * @note From the Europa DNP model.
 */
class FIGURE_EARLIER_OP_IDS : public Constraint {
public:
  FIGURE_EARLIER_OP_IDS(const LabelStr& name,
                        const LabelStr& propagatorName,
                        const ConstraintEngineId& constraintEngine,
                        const std::vector<ConstrainedVariableId>& variables);

  ~FIGURE_EARLIER_OP_IDS() {
  }
  
  void handleExecute();
};

#define registerDNPConstraints() { \
  REGISTER_CONSTRAINT(BOUNDS_PLAYBACK_START_STORAGE, "BOUNDS_PLAYBACK_START_STORAGE", "Default"); \
  REGISTER_CONSTRAINT(BOUNDS_PLAYBACK_END_STORAGE, "BOUNDS_PLAYBACK_END_STORAGE", "Default"); \
  REGISTER_CONSTRAINT(BOUNDS_RECORD_END_STORAGE, "BOUNDS_RECORD_END_STORAGE", "Default"); \
  REGISTER_CONSTRAINT(BOUNDS_RECORD_START_STORAGE, "BOUNDS_RECORD_START_STORAGE", "Default"); \
  REGISTER_CONSTRAINT(COMPUTE_PLAYBACK_DURATION, "COMPUTE_PLAYBACK_DURATION", "Default"); \
  REGISTER_CONSTRAINT(FIGURE_EARLIER_OP_IDS, "FIGURE_EARLIER_OP_IDS", "Default"); \
}
