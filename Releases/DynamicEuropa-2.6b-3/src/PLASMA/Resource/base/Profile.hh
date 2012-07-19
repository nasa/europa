#ifndef _H_Profile
#define _H_Profile

/**
 * @file Profile.hh
 * @author Michael Iatauro
 * @brief Defines a base class for computing Resource level profiles
 * @date March, 2006
 * @ingroup Resource
 */

#include "Domains.hh"
#include "PlanDatabaseDefs.hh"
#include "ConstraintEngineDefs.hh"
#include "Constraint.hh"
#include "CommonDefs.hh"
#include "ResourceDefs.hh"
#include "FVDetector.hh"
#include "LabelStr.hh"
#include "Debug.hh"
#include "Engine.hh"
#include "Factory.hh"

#include <map>
#include <utility>

namespace EUROPA {

    /**
     * @class Profile
     * @brief The base class for managing Resource level profiles
     *
     * The Profile class is, in general, responsible for managing the computation of bounds around the level of some Resource.
     * It does this by performing some computation on a set of Transactions, which are <timepoint, quantity> pairs, with possible
     * flexibility in either the timepoint or the quantity.  The computed profile is made available to other classes (an envelope
     * calculator, for instance) via a ProfileIterator.
     * The base Profile class is specifically responsible for maintaining the structures representing the profile (Instants) and
     * for causing subclasses to recalculate the profile.
     * When should we recalculate?
     */
    class Profile : public FactoryObj {
    public:

      /**
       * @brief Constructor.
       * @param flawDetector The object responsible for detecting flaws and violations during level calculation.
       */
      Profile(const PlanDatabaseId ce, const FVDetectorId flawDetector, const edouble initLevelLb = 0, const edouble initLevelUb = 0);
      virtual ~Profile();
      ProfileId& getId() {return m_id;}

      /**
       * @brief Adds a Transaction to the profile.  Creates Instants for the lower and upper bounds of the timepoint if necessary,
       * adds the Transaction to every Instant in the interval [timepoint.start timepoint.end], and marks the need for recalculation.
       * This method also adds listeners to the quantity and timepoint in order to track changes in their values.
       * @param e The Transaction.
       */
      virtual void addTransaction(const TransactionId e);

      /**
       * @brief Removes a Transaction from the profile.  Removes the Transaction from all instants in the interval [timepoint.start timepoint.end],
       * marks the need for recalculation, and removes the listeners on the quantity and timepoint.
       * @brief e The Transaction.
       */
      virtual void removeTransaction(const TransactionId e);


      /**
       * @brief Gets the bounds of the profile at a given point in time. Calling this method may cause recalculation.
       * @param time The time at which to get the level.
       * @param dest The interval into which the computed levels are stored.
       */
      virtual void getLevel(const eint time, IntervalDomain& dest);
      
      /**
       * @brief Validates the data structures.  Maybe this should be private.
       */
      virtual bool isValid();

      /**
       * @brief Forces a recomputation of the profile.
       */
      void recompute();

      const PlanDatabaseId& getPlanDatabase() const {return m_planDatabase;}


      virtual void getTransactionsToOrder(const InstantId& inst, std::vector<TransactionId>& results);

      const ResourceId& getResource() const {return m_detector->getResource();}

    protected:

      bool hasTransactions() {return !m_transactions.empty();}

      void transactionTimeChanged(const TransactionId e, const DomainListener::ChangeType& change);

      void transactionQuantityChanged(const TransactionId e, const DomainListener::ChangeType& change);

      //these methods need to marshal the data between two calls
      void temporalConstraintAdded(const ConstraintId c, const ConstrainedVariableId var, int argIndex);

      void temporalConstraintRemoved(const ConstraintId c, const ConstrainedVariableId var, int argIndex);

      /**
       * @brief Recompute the profile.  Iterates over a stored interval of time.
       * It is expected that the first Instant in the interval actually precede the first change
       * so that the flaw and violation detector can be initialized.
       */
      void handleRecompute();
      /**
       * @brief Hanlde invoked at the end of handleRecompute
       */
      virtual void postHandleRecompute() {}
      /**
       * @brief Initialize a recomputation with level data from the given Instant.
       */
      virtual void initRecompute(InstantId inst) = 0;

      /**
       * @brief Initialize a recomputation with no level data.  Used when there is no Instant preceeding the recomputation interval.
       */
      virtual void initRecompute() = 0;

      /**
       * @brief Recompute the levels for a particular Instant.
       * @param inst The Instant to be recomputed.
       */
      virtual void recomputeLevels( InstantId prev, InstantId inst ) = 0;

      /**
       * @brief Helper method for subclasses to respond to a transaction being added.
       * @param e The Transaction that was added
       */
      virtual void handleTransactionAdded(const TransactionId e);

      /**
       * @brief Helper method for subclasses to respond to a transaction being removed.
       * @param e The Transaction that was removed
       */
      virtual void handleTransactionRemoved(const TransactionId e);

      /**
       * @brief Helper method for subclasses to respond to a transaction's timepoint changing.
       * The base method handles the addition/removal of the transaction to/from instants.
       * @param e The Transaction whose time variable changed.
       * @param change The type of the change
       */
      virtual void handleTransactionTimeChanged(const TransactionId e, const DomainListener::ChangeType& change);

      /**
       * @brief Helper method for subclasses to respond to a transaction's quantity changing.
       * @param e The Transaction whose quantity variable changed.
       * @param change The type of the change
       */
      virtual void handleTransactionQuantityChanged(const TransactionId e, const DomainListener::ChangeType& change);

      //NOTE: DON'T FORGET TO IMPLEMENT THE DEFAULT  BEHAVIOR!!!
      /**
       * @brief Helper method for subclasses to respond to a temporal constraint being added between two transactions.
       * @param e The transaction whose timepoint has been constrained.
       * @param argIndex The index of the timepoint in the constraint.
       */
      virtual void handleTemporalConstraintAdded(const TransactionId predecessor, const int preArgIndex,
                                                 const TransactionId successor, const int sucArgIndex);

      /**
       * @brief Helper method for subclasses to respond to a temporal constraint being removed between two transactions.
       * @param e The transaction whose timepoint has been removed from the constraint.
       * @param argIndex The index of the timepoint in the constraint.
       */
      virtual void handleTemporalConstraintRemoved(const TransactionId predecessor, const int preArgIndex,
                                                   const TransactionId successor, const int sucArgIndex);


      /**
       * @brief Helper method for subclasses to respond to a variable deletion (the listener on that variable may need destruction).
       * @param t The transaction that has been removed.
       */
      void handleTransactionVariableDeletion(const TransactionId& t);

      bool needsRecompute() const {return m_needsRecompute;}

      std::string toString() const;
    private:
      friend class ProfilePropagator;
      friend class ProfileIterator;
      friend class Instant;

      /**
       * @class VariableListener
       * @brief Informs the profile of a change in either a quantity or time variable.
       */
      class VariableListener : public Constraint {
      public:
        VariableListener(const ConstraintEngineId& constraintEngine,
                         const ProfileId profile,
                         const TransactionId trans,
                         const std::vector<ConstrainedVariableId>& scope, const bool isQuantity = false);
        static const LabelStr& CONSTRAINT_NAME() {
          static const LabelStr sl_const("ProfileVariableListener");
          return sl_const;
        }
        static const LabelStr& PROPAGATOR_NAME() {
          static const LabelStr sl_const("Resource");
          return sl_const;
        }
        ProfileId getProfile() const {return m_profile;}
      private:
        bool canIgnore(const ConstrainedVariableId& variable,
                       int argIndex,
                       const DomainListener::ChangeType& changeType);
        void handleExecute(){};
        ProfileId m_profile;
        TransactionId m_trans;
        bool m_isQuantity;
      };

      class ConstraintAdditionListener : public ConstrainedVariableListener {
      public:
        ConstraintAdditionListener(const ConstrainedVariableId& var, TransactionId tid, ProfileId profile);
        ~ConstraintAdditionListener();
        void notifyDiscard();
        void notifyConstraintAdded(const ConstraintId& constr, int argIndex);
        void notifyConstraintRemoved(const ConstraintId& constr, int argIndex);
      private:
        ProfileId m_profile;
        TransactionId m_tid;
      };

      struct ConstraintMessage {
        ConstrainedVariableId var;
        int index;
        bool addition;
        ConstraintMessage(ConstrainedVariableId _var, int _index, bool _addition)
          : var(_var), index(_index), addition(_addition) {}
      };

      /**
       * @class ConstraintRemovalListener
       * @brief Listens for removal messages on constraints.
       * 
       * Because it's possible for the ConstraintAdditionListener to get removed before a constraint it's listening for,
       * this class has been added to catch any final messages about removal.
       */

      class ConstraintRemovalListener : public ConstraintEngineListener {
      public:
        ConstraintRemovalListener(const ConstraintEngineId& ce, ProfileId profile);
        void notifyRemoved(const ConstraintId& constr);
      private:
        ProfileId m_profile;
      };

      bool hasConstraint(const ConstraintId& constr) const;

    public:
      /**
       * @brief Gets the Instant with the greatest time that is not greater than the given time.
       * @return An iterator pointing to the instant.
       */
      std::map<eint, InstantId>::iterator getGreatestInstant(const eint time);
      
      /**
       * @brief Gets the Instant with the least time not less than the given time.
       * @return An iterator pointing to the instant.
       */
      std::map<eint, InstantId>::iterator getLeastInstant(const eint time);

      const std::map<eint, InstantId>& getInstants() {return m_instants;}

      // PHM Some refactoring needed so that customized subclass can
      // support reftimes.  Does not change this class functionality.
    protected:
      /**
       * @brief Determines whether an instant contains a "significant"
       * transaction (i.e., at its upper/lower bound) that changes the
       * profile at that instant.
       *
       * @param instant The instant
       */
      virtual bool containsChange(const InstantId instant);
      /**
       * @brief Conditionally adds Instants for the upper and lower
       * bounds of the Transaction's time.
       *
       * @param t The transaction
       */
      virtual void addInstantsForBounds(const TransactionId t);

      /**
       * @brief Create an instant for a time.
       * @param time The time
       */
      void addInstant(const eint time);
      /**
       * @brief Eliminate an instant for a time.
       * @param time The time
       */
      void removeInstant(const eint time);
    public:
      /**
       * @brief Get all the transactions in the profile.
       */
      const std::set<TransactionId>& getAllTransactions() const {return m_transactions;}


    private:


      /** 
       * @brief Handle an addition or removal message on a temporal constraint.
       * Waits for two consecutive addition or removal messages in order to ensure that both variables in the scope of the 
       * constraint are resource-temporal variables.  
       * 
       * @param c The constraint.
       * @param var The resource-related temporal variable the constraint is on.
       * @param argIndex The index of the resource-related temporal variable in the scope of the constraint.
       * @param addition True if the message is an addition, false otherwise.
       */      
      void handleConstraintMessage(const ConstraintId c, const ConstrainedVariableId var, int argIndex, bool addition);

      /** 
       * @brief Checks the m_constraintsForNotification map for internal consistency.
       * 
       * 
       * @return true if the map is consistent.  False otherwise.
       */
      bool checkMessageConsistency();

      ProfileId m_id;
      unsigned int m_changeCount; /*<! The number of times that the profile has changed.  Used to detect stale iterators.*/
      bool m_needsRecompute; /*<! A flag indicating the necessity of profile recomputation*/
      unsigned int m_constraintKeyLb; /*<! The lower bound on the constraint key when searching for new constraints. */
    protected:
      const edouble m_initLevelLb, m_initLevelUb;
      PlanDatabaseId m_planDatabase; /*<! The plan database.  Used for creating the variable listeners. */
    private:
      FVDetectorId m_detector; /*<! The flaw and violation detector. */
      std::set<TransactionId> m_transactions; /*<! The set of Transactions that impact this profile. */
      std::multimap<TransactionId, ConstraintId> m_variableListeners; /*<! The listeners on the Transactions. */
      std::map<TransactionId, ConstrainedVariableListenerId> m_otherListeners;
      std::map<ConstrainedVariableId, TransactionId> m_transactionsByTime;
      ConstraintSet m_temporalConstraints; 
      ConstraintEngineListenerId m_removalListener;
    protected:
      std::map<eint, InstantId> m_instants; /*<! A map from times to Instants. */
      ProfileIteratorId m_recomputeInterval; /*<! The stored interval of recomputation.*/
    };

    /**
     * @class ProfileIterator
     * @brief Used to iterate over a calculated profile.  Each call to next() advances the iterator to the next point in time at which a
     * level changes.  Directly constructible only inside the Profile class. (Is this a good idea?)
     */
    class ProfileIterator {
    public:

      /**
       * @brief Constructor for an iterator over a profile.
       * @param prof The profile to be iterated.
       * @param startTime The time to begin iteration.  The time of the first Instant is guaranteed to be the greatest time not greater than this value.
       * @param endTime The time to end iteration.  The time of the last Instant is guaranteed to be the greatest time not greater than this value.
       */
      ProfileIterator(const ProfileId prof, const eint startTime = MINUS_INFINITY, const eint endTime = PLUS_INFINITY);
      
      ~ProfileIterator() {m_id.remove();}

      /**
       * @brief Determine whether or not this ProfileIterator has iterated past its end time.
       */
      bool done() const;

      /**
       * @brief Gets the time of the current position of this ProfileIterator
       * @return The time
       */
      eint getTime() const;
      
      /**
       * @brief Gets the value of the lower bound of the profile at the current instant in time.
       * @return The value of the profile.
       */
      edouble getLowerBound() const;

      /**
       * @brief Gets the value of the upper bound of the profile at the current instant in time.
       * @return The value of the profile.
       */
      edouble getUpperBound() const;

      /**
       * @brief Gets the current instant.
       * @return The instant.
       */
      InstantId getInstant() const;
      /**
       * @brief Steps the ProfileIterator to the next instant in time.
       * @return true if the ProfileIterator ends on a valid Instant, false otherwise.
       */
      bool next();

      ProfileIteratorId getId(){return m_id;}

      /**
       * @brief Determines if the interval of the iterator is still in synch with the actual profile.
       */
      bool isStale() const;

      eint getStartTime() const;
      eint getEndTime() const;
    protected:
    private:
      ProfileIteratorId m_id;
      ProfileId m_profile;
      unsigned int m_changeCount; /*<! A copy of the similar variable in Profile when this iterator was instantiated.  Used to detect staleness. */
      eint m_startTime, m_endTime;
      std::map<eint, InstantId>::const_iterator m_start, m_end, m_realEnd; /*<! The start and end times over which this iterator goes*/
    };

    class ProfileArgs : public FactoryArgs
    {
    public:
        ProfileArgs(const PlanDatabaseId& db, const FVDetectorId& detector,
                    const edouble initCapacityLb = 0, const edouble initCapacityUb = 0)
            : m_db(db), m_detector(detector), m_initCapacityLb(initCapacityLb), m_initCapacityUb(initCapacityUb)
        {
        }

        const PlanDatabaseId m_db;
        const FVDetectorId m_detector;
        const edouble m_initCapacityLb;
        const edouble m_initCapacityUb;
    };

    template<class ProfileType>
    class ProfileFactory : public Factory {
    public:
      ProfileFactory(const LabelStr& name) : Factory(name) {}

      virtual EUROPA::FactoryObjId& createInstance(const EUROPA::FactoryArgs& fa)
      {
          const ProfileArgs& args = (const ProfileArgs&)fa;
          return (EUROPA::FactoryObjId&)
              (new ProfileType(args.m_db, args.m_detector, args.m_initCapacityLb, args.m_initCapacityUb))->getId();
      }
    };

    #define REGISTER_PROFILE(MGR, CLASS, NAME) (MGR->registerFactory((new EUROPA::ProfileFactory<CLASS>(#NAME))->getId()));

}

#endif
