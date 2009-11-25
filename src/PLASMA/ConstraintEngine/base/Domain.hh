#ifndef _H_Domain
#define _H_Domain

/**
 * @file Domain.hh
 * @author Conor McGann
 * @date August, 2003
 * @brief Declares new domain management object model.
 *
 * A new approach for domains is presented which allows for more customized domain implementations to assess possible
 * speed improvements. Domain is the base class for all other domains. It is the only domain reference
 * available in the  core of the framework.
 *
 * The semantics of a domain are described in terms of:
 * @li Open vs Closed - A domain is open if new values are permitted to be added to the domain. Conversely,
 * a domain is closed if no new values may be added. One can explicitly close a domain, or construct it with
 * all initial values and close it upon construction implicitly. An 'open' domain is equivalently described as
 * 'dynamic'. Some operations require a domain to be closed before use.
 * @li Finite vs Infinite - A domain is finite if it has a finite number of elements in its domain.
 * @li Enumerated vs. Interval - A domain may be represented as an explicit enumeration of values or as an
 * upper and lower bound only, as a shorthand. A domain may not alter between enumerated and interval.
 * @li Symbolic vs Numeric - If the elements in the domain are numbers or symbols (including strings).
 * @li Empty vs. not Empty - If there are no elements in the domain.
 * @li Types - A domain may be further typed to restrict its semantics by constructing it with a given type
 * descriptor.
 *
 * Operations among domains are type checked according the compatibility rules enforced by a DomainComparator.
 * A default DomainComparator is provided. It may be extended to support additional semantic relationships
 * between types.
 *
 * A central design element is the use of a DomainListener to optionally observe changes on the domain. This is
 * an Observer pattern where the Domain is the subject and the DomainListener is the observer. This mechamism
 * is the basis for propagation.
 *
 *
 * @see Variable, Constraint
 */
#include "ConstraintEngineDefs.hh"
#include "LabelStr.hh"
#include "DomainListener.hh"
#include "Number.hh"
#include <list>
#include <map>
#include <string>
#include <set>

#ifdef EUROPA_FAST
#define safeConversion(value) (value)
#define checkPrecision(value)
#define safeComparison(domA, domB)
#else
#define safeConversion(value) check(value)
#define checkPrecision(value) testPrecision(value)
#define safeComparison(domA, domB) assertSafeComparison(domA, domB)
#endif

namespace EUROPA {
  using std::ostream;

  // TODO!: drop this?
  /**
   * @class DomainComparator
   * @brief Class for testng if 2 domains can be compared. Exetend this class to customize how this is
   * done.
   * @see Domain::canBeCompared, Domain::comparator
   */
  class DomainComparator{
  public:
    DomainComparator();
    virtual ~DomainComparator();

    /**
     * @brief Retrieve the singleton comparator
     */
    static const DomainComparator& getComparator();

    /**
     * @brief Tests if domains can be compared.
     */
    virtual bool canCompare(const Domain& domx, const Domain& domy) const;

    /**
     * @brief Set the comparator to be used. Can only be set if it is currently null.
     */
    static void setComparator(DomainComparator* comparator);

    /**
     * @brief return true iff comparator is null. False otherwise.
     */
    bool comparatorIsNull();


  private:
    static DomainComparator* s_instance; /*!< Access pointer location. Enforces singleton pattern */
  };

  ostream& operator<<(ostream& os, const Domain& dom);

  /**
   * @class Domain
   * @brief The base class for all domains used in the ConstraintEngine.
   *
   * This base class provides the core mechanisms to integrate a domain into the ConstraintEngine framework without
   * committing to a concrete type for the domain. This allows us to pass around and use domains in the core but still
   * provide specializations as necessary in a very extendible way.
   * @see DomainListener
   */
  class Domain {
  public:
#ifdef E2_LONG_INT
    typedef unsigned long int size_type;
#else
    typedef unsigned int size_type;
#endif

    /**
     * Destructor.
     */
    virtual ~Domain();

    /**
     * @brief Check if the domain is an enumerated set.
     */
    bool isEnumerated() const;

    /**
     * @brief Check if the domain is an interval.
     */
    bool isInterval() const;

    /**
     * @brief Test if the domain is closed (i.e. can be used yet)
     * @see close
     */
    bool isClosed() const;

    /**
     * @brief Test if the domain is closed
     * @note negation of isClosed
     */
    bool isOpen() const;

    /**
     * @brief Check if the domain is empty.
     */
    virtual bool isEmpty() const = 0;

    /**
     * @brief Check if there are a finite number of values in the domain.
     * @return true if there are a finite number of values in the domain.
     */
    virtual bool isFinite() const = 0;

    /**
     * @brief !isFinite()
     */
    bool isInfinite() const;

    /**
     * @brief Tests if both bounds are finite. Trivially true for symbolic domains.
     */
    virtual bool areBoundsFinite() const;

    /**
     * @brief Check if the domain is a singleton.
     */
    virtual bool isSingleton() const = 0;

    /**
     * @brief Return the number of elements in the domain.
     * @note Can only be called if (!isDynamic() && isFinite()).
     * @return the number of values in the domain.
     */
    virtual size_type getSize() const = 0;

    /**
     * @brief Access upper bound
     */
    virtual edouble getUpperBound() const = 0;

    /**
     * @brief Access lower bound
     */
    virtual edouble getLowerBound() const = 0;

    /**
     * @brief Access both bounds in a convenience method, and indicates if the domain is infinite
     * @param lb update this value with the lower bound
     * @param ub update this value with the upper bound
     * @return true if !isFinite()
     */
    virtual bool getBounds(edouble& lb, edouble& ub) const = 0;

    /**
     * @brief Fill the given list with the contents of the set.
     * @note Should only be called on finite (and thus closed) domains.
     * @param results The target collection to fill with all values in the set.
     */
    virtual void getValues(std::list<edouble>& results) const = 0;

    /**
     * @brief Access singleton value. Must be a singleton or this will fail.
     */
    virtual edouble getSingletonValue() const = 0;

    /**
     * @brief Close the domain.
     *
     * @note When completed, we require (isClosed() == true).
     * @see isClosed()
     */
    virtual void close();

    /**
     * @brief Re-open the domain.
     *
     * @note When completed, we require (isOpen() == true).
     * @see isOpen()
     */

    virtual void open();

    /**
     * @brief Touch the domain. Will produce a change event that will generate necessary change evaluation
     */
    void touch();

    /**
     * @brief Empty the domain.
     * @note Completion of this will require that (isEmpty() == true).
     */
    virtual void empty() = 0;

    /**
     * @brief Set to a singleton.
     * @note May empty the domain if value is not a member of the current domain.
     * @param value the target singleton value.
     */
    virtual void set(edouble value) = 0;

    /**
     * @brief Indicates assigment to the target domain as a relaxation triggered explicitly
     * rather than through algorithm for relaxation.
     * @see relax
     */
    virtual void reset(const Domain& dom) = 0;

    /**
     * @brief Indicates assigment to the target domain as a relaxation triggered internally i.e. via relaxation algorithm.
     * @param dom the target domain to relax it to.
     * @see reset
     */
    virtual void relax(const Domain& dom) = 0;

    /**
     * @brief Indicates relaxation to a singleton value. Occurs when domain has been emptied previously
     */
    virtual void relax(edouble value) = 0;

    /**
     * @brief Add an element to the set. This is only permitted on dynamic domains.
     *
     * @param value The value to insert. If not currently present it will be inserted. Otherwise it will be
     * ignored. If inserted, this operation constitutes a domain relaxation and will result in a relaxation event
     * being raised.
     * @see DomainListener::DOMAIN_RELAXED
     * @todo Consider if it makes sense to error out if isMember(value).
     */
    virtual void insert(edouble value) = 0;

    /**
     * @brief Add a list of elements to the set. Only permotted on dynamic and enumerated domains
     */
    virtual void insert(const std::list<edouble>& values) = 0;

    /**
     * @brief Remove the given element form the domain.
     * @param value. The value to be removed.
     * @note If the value was in the domain, this call will generate a value removal event.
     * @see DomainListener::VALUE_REMOVED
     */
    virtual void remove(edouble value) = 0;

    /**
     * @brief Restricts this domain to the intersection of its values with the given domain.
     * @param dom the domain to intersect with. Must not be empty.
     * @return true if the intersection results in a change to this domain, otherwise false.
     */
    virtual bool intersect(const Domain& dom) = 0;

    /**
     * @brief Convenience version of intersect.
     * @param lb the lower bound of domain to intersect with.
     * @param ub the upper bound of domain to intersect with.
     * @return true if the intersection results in a change to this domain, otherwise false.
     * @note ub must be >= lb.
     * @note The domain must be numeric
     */
    virtual bool intersect(edouble lb, edouble ub) = 0;

    /**
     * @brief Restricts this domain to the difference of its values with the given domain.
     * @param dom the domain to differ with. Must not be empty.
     * @return true if the operation results in a change to this domain, otherwise false.
     */
    virtual bool difference(const Domain& dom) = 0;

    /**
     * @brief Assign the values from the given domain, to this domain.
     * @note Can only be called on domains that have no listeners attached,
     * since it will not cause propagation. It is more of a utility.
     */
    virtual Domain& operator=(const Domain& dom) {
      return(*this);
    }

    /**
     * @brief Mutually constrain both domains to their respective intersections.
     * @note The implementation must ensure that at most one domain is
     * emptied in the process.
     * @param dom The domain to perform mutual intersection with.
     * @return true if the intersection results in a change to either
     * domain, otherwise false.
     */
    virtual bool equate(Domain& dom) = 0;

    /**
     * @brief Test for membership.
     * @param value to test for membership.
     * @return true if a member of the domain, otherwise false.
     */
    virtual bool isMember(edouble value) const = 0;

    /**
     * @brief Test if this domain is a subset of dom.
     * @param dom the domain tested against.
     * @param true if all elements of this domain are in dom. Otherwise false.
     */
    virtual bool isSubsetOf(const Domain& dom) const = 0;

    /**
     * @brief Test if the intersection between this domain and the given domain is empty.
     * @param dom the domain tested against.
     * @param true if any elements of this domain are in dom. Otherwise false.
     */
    virtual bool intersects(const Domain& dom) const = 0;

    /**
     * @brief Test for equality.
     */
    virtual bool operator==(const Domain& dom) const;

    /**
     * @brief Test for inequality.
     */
    virtual bool operator!=(const Domain& dom) const;

    /**
     * @brief Attach a DomainListener.
     * @note Requires that no domain listener is currently attached.
     * Will error out if that is not the case.
     * @param listener the listener to attach.
     */
    virtual void setListener(const DomainListenerId& listener);

    /**
     * @brief Accessor for the listener.
     * @return the listener. May be noId() if no listener attached
     */
    virtual const DomainListenerId& getListener() const;

    /**
     * "Deeply" copy the concrete C++ object into new memory and return a pointer to it.
     */
    virtual Domain *copy() const = 0;

    /**
     * @brief Creates a verbose string for displaying the contents of the domain
     */
    virtual std::string toString() const;

    /**
     * @brief Print this on the output stream.
     */
    virtual void operator>>(ostream& os) const;

    /**
     * @brief Tests if two domains can be compared. For example, one cannot compare a symbolic
     * enumerated domain with a numeric domain. This is useful to enforce type checking
     * in constraints in particular.
     * @see DomainComparator::canCompare
     */
    static bool canBeCompared(const Domain& domx, const Domain& domy);

    const DataTypeId& getDataType() const;

    // TODO: all these just delegate to the data type, should be dropped eventually, preserved for now for backwards compatibility
    const LabelStr& getTypeName() const;
    bool isSymbolic() const;
    bool isEntity() const;
    bool isNumeric() const;
    bool isBool() const;
    bool isString() const;
    bool isRestricted() const;
    edouble minDelta() const;
    std::string toString(edouble value) const;

    /**
     * @brief Returns a value for number based on the semantics of the domain.
     */
    virtual edouble translateNumber(edouble number, bool asMin = true) const;

    /**
     * @brief Converts the string to its double representation as a value, if it is present in the domain.
     * @param strValue The value as a string
     * @param dblValue The value returned, if available. Only relevant if a member of the domain.
     * @return true if the value was present, otherwise false.
     */
    virtual bool convertToMemberValue(const std::string& strValue, edouble& dblValue) const = 0;

    /**
     * Tests if 2 values are the same with respect to the minimum difference for the target domain.
     */
    inline virtual bool compareEqual(edouble a, edouble b) const 
    { return(a < b ? b - a < minDelta() : a - b < minDelta()); }

    /**
     * @brief Tests if one value is less than another to within minDelta
     */
    inline bool lt(edouble a, edouble b) const { return a != PLUS_INFINITY && b != MINUS_INFINITY && (a + minDelta() <= b); }

    /**
     * @brief Tests if one value equals another to within minDelta
     */
    inline bool eq(edouble a, edouble b) const { return compareEqual(a, b); }

    /**
     * @brief Tests if one value is less than or equal to another to within minDelta
     */
    inline bool leq(edouble a, edouble b) const { return a == b || (a - minDelta() < b); }

  protected:
    /**
     * @brief Constructor.
     * @param closed indicates if the domain is to be dynamic or closed on construction.  Once closed, no
     * additions to extend the contents of the domain will be permitted.
     * @param enumerated true if this is an explicit enumeration.
     * @param typeName indicates the type name to use
     * @todo Review how semantics of closed can be enforced in operations.
     */
    Domain(const DataTypeId& dataType, bool enumerated, bool closed);

    /**
     * @brief Copy Constructor
     * @param org Original domain
     */
    Domain(const Domain& org);

    /**
     * @brief Helper method to push messages to the listener.
     * @see DomainListener
     */
    void notifyChange(const DomainListener::ChangeType& changeType);

    /**
     * @brief Check that the value is correct w.r.t. the semantics of the domain. Infinite safe if numeric.
     */
    virtual bool check_value(edouble value) const;

    /**
     * @brief Check the precision of a value in terms of a particular Domain class.
     * @note 'Hook' to permit subclasses to be more restrictive; called by check_value()
     * unless compiled EUROPA_FAST.
     */
    virtual void testPrecision(const edouble& value) const = 0;

    /**
     * @brief Utility function for internal use
     */
    static void assertSafeComparison(const Domain& domA, const Domain& domB);

    void setDataType(const DataTypeId& dt);
    friend class RestrictedDT;

    DataTypeId m_dataType;
    bool m_enumerated; /**< True is domain is enumerated (as opposed to interval) */
    bool m_closed; /**< False if the domain is dynamic (can be added to), otherwise true. */
    DomainListenerId m_listener; /**< Holds reference to attached listener.  May be noId. */
  };

  //BACKWARD COMPATIBILITY
  //typedef Domain AbstractDomain

}
#endif
