#ifndef _H_AbstractDomain
#define _H_AbstractDomain

/**
 * @file AbstractDomain.hh
 * @author Conor McGann
 * @date August, 2003
 * @brief Declares new domain management object model.
 *
 * A new approach for domains is presented which allows for more customized domain implementations to assess possible
 * speed improvements. AbstractDomain is the base class for all other domains. It is the only domain reference 
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
 * an Observer pattern where the AbstractDomain is the subject and the DomainListener is the observer. This mechamism
 * is the basis for propagation.
 *
 * 
 * @see Variable, Constraint
 */
#include "ConstraintEngineDefs.hh"
#include "LabelStr.hh"
#include "DomainListener.hh"
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

  /**
   * @class DomainComparator
   * @brief Class for testng if 2 domains can be compared. Exetend this class to customize how this is
   * done.
   * @see AbstractDomain::canBeCompared, AbstractDomain::comparator
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
    virtual bool canCompare(const AbstractDomain& domx, const AbstractDomain& domy) const;
 
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

  ostream& operator<<(ostream& os, const AbstractDomain& dom);

  /**
   * @class AbstractDomain
   * @brief The base class for all domains used in the ConstraintEngine.
   *
   * This base class provides the core mechanisms to integrate a domain into the ConstraintEngine framework without
   * committing to a concrete type for the domain. This allows us to pass around and use domains in the core but still 
   * provide specializations as necessary in a very extendible way.
   * @see DomainListener
   */
  class AbstractDomain {
  public:

    /**
     * Destructor.
     */
    virtual ~AbstractDomain();

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
     * @brief Test if the domain is closed (i.e. can be used yet)
     * @see close
     */
    virtual bool isClosed() const;

    /**
     * @brief Test if the domain is closed
     * @note negation of isClosed
     */
    virtual bool isOpen() const;

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
     * @brief Check if the domain is symbolic.
     */
    virtual bool isSymbolic() const {return !isNumeric();}

    /**
      * @brief Check if the domain contains entities
      */
    virtual bool isEntity() const {return false;}

    /**
     * @brief Check if the domain is numeric.
     */
    virtual bool isNumeric() const = 0;

    /**
     * @brief Check if the domain is Boolean.
     */
    virtual bool isBool() const = 0;

    /**
     * @brief Check if the domain is String.
     */
    virtual bool isString() const = 0;

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
     * @brief Get the domain's type's name.
     */
    virtual const LabelStr& getTypeName() const;

    /**
     * @brief Check if the domain is an enumerated set.
     */
    virtual bool isEnumerated() const;

    /**
     * @brief Check if the domain is an interval.
     */
    virtual bool isInterval() const;

    /**
     * @brief Check if the domain is a singleton.
     */
    virtual bool isSingleton() const = 0;

    /**
     * @brief Check if the domain is empty.
     */
    virtual bool isEmpty() const = 0;

    /**
     * @brief Empty the domain.
     * @note Completion of this will require that (isEmpty() == true).
     */
    virtual void empty() = 0;

    /**
     * @brief Return the number of elements in the domain.
     * @note Can only be called if (!isDynamic() && isFinite()).
     * @return the number of values in the domain.
     */
    virtual unsigned int getSize() const = 0;

    /**
     * @brief Print this on the output stream.
     */
    virtual void operator>>(ostream& os) const;

    /**
     * @brief Access upper bound
     */
    virtual double getUpperBound() const = 0;

    /**
     * @brief Access lower bound
     */
    virtual double getLowerBound() const = 0;

    /**
     * @brief Access singleton value. Must be a singleton or this will fail.
     */
    virtual double getSingletonValue() const = 0;

    /**
     * @brief Access both bounds in a convenience method, and indicates if the domain is infinite
     * @param lb update this value with the lower bound
     * @param ub update this value with the upper bound
     * @return true if !isFinite()
     */
    virtual bool getBounds(double& lb, double& ub) const = 0;

    /**
     * @brief Set to a singleton.
     * @note May empty the domain if value is not a member of the current domain.
     * @param value the target singleton value.
     */
    virtual void set(double value) = 0;

    /**
     * @brief Indicates assigment to the target domain as a relaxation triggered explicitly 
     * rather than through algorithm for relaxation.
     * @see relax
     */
    virtual void reset(const AbstractDomain& dom) = 0;

    /**
     * @brief Indicates assigment to the target domain as a relaxation triggered internally i.e. via relaxation algorithm.
     * @param dom the target domain to relax it to.
     * @see reset
     */
    virtual void relax(const AbstractDomain& dom) = 0;

    /**
     * @brief Indicates relaxation to a singleton value. Occurs when domain has been emptied previously
     */
    virtual void relax(double value) = 0;

    /**
     * @brief Add an element to the set. This is only permitted on dynamic domains.
     *
     * @param value The value to insert. If not currently present it will be inserted. Otherwise it will be
     * ignored. If inserted, this operation constitutes a domain relaxation and will result in a relaxation event
     * being raised.
     * @see DomainListener::DOMAIN_RELAXED
     * @todo Consider if it makes sense to error out if isMember(value).
     */
    virtual void insert(double value) = 0;

    /**
     * @brief Add a list of elements to the set. Only permotted on dynamic and enumerated domains
     */
    virtual void insert(const std::list<double>& values) = 0;

    /**
     * @brief Remove the given element form the domain.
     * @param value. The value to be removed.
     * @note If the value was in the domain, this call will generate a value removal event.
     * @see DomainListener::VALUE_REMOVED
     */
    virtual void remove(double value) = 0;

    /**
     * @brief Restricts this domain to the intersection of its values with the given domain.
     * @param dom the domain to intersect with. Must not be empty.
     * @return true if the intersection results in a change to this domain, otherwise false.
     */
    virtual bool intersect(const AbstractDomain& dom) = 0;

    /**
     * @brief Convenience version of intersect.
     * @param lb the lower bound of domain to intersect with.
     * @param ub the upper bound of domain to intersect with.
     * @return true if the intersection results in a change to this domain, otherwise false.
     * @note ub must be >= lb.
     * @note The domain must be numeric
     */
    virtual bool intersect(double lb, double ub) = 0;

    /**
     * @brief Restricts this domain to the difference of its values with the given domain.
     * @param dom the domain to differ with. Must not be empty.
     * @return true if the operation results in a change to this domain, otherwise false.
     */
    virtual bool difference(const AbstractDomain& dom) = 0;

    /**
     * @brief Assign the values from the given domain, to this domain.
     * @note Can only be called on domains that have no listeners attached,
     * since it will not cause propagation. It is more of a utility.
     */
    virtual AbstractDomain& operator=(const AbstractDomain& dom) {
      return(*this);
    }

    /**
     * @brief Test for membership.
     * @param value to test for membership.
     * @return true if a member of the domain, otherwise false.
     */
    virtual bool isMember(double value) const = 0;

    /**
     * @brief Test for equality.
     */
    virtual bool operator==(const AbstractDomain& dom) const;

    /**
     * @brief Test for inequality.
     */
    virtual bool operator!=(const AbstractDomain& dom) const;

    /**
     * @brief Test if this domain is a subset of dom.
     * @param dom the domain tested against.
     * @param true if all elements of this domain are in dom. Otherwise false.
     */
    virtual bool isSubsetOf(const AbstractDomain& dom) const = 0;

    /**
     * @brief Test if the intersection between this domain and the given domain is empty.
     * @param dom the domain tested against.
     * @param true if any elements of this domain are in dom. Otherwise false.
     */
    virtual bool intersects(const AbstractDomain& dom) const = 0;

    /**
     * @brief Mutually constrain both domains to their respective intersections.
     * @note The implementation must ensure that at most one domain is
     * emptied in the process.
     * @param dom The domain to perform mutual intersection with.
     * @return true if the intersection results in a change to either
     * domain, otherwise false.
     */
    virtual bool equate(AbstractDomain& dom) = 0;

    /**
     * @brief Fill the given list with the contents of the set.
     * @note Should only be called on finite (and thus closed) domains.
     * @param results The target collection to fill with all values in the set.
     */
    virtual void getValues(std::list<double>& results) const = 0;

    /**
     * @brief Returns the minimum allowed delta in values between elements of the set.
     */
    virtual double minDelta() const {return m_minDelta;}

    /**
     * @brief Returns a value for number based on the semantics of the domain.
     */
    virtual double translateNumber(double number, bool asMin) const;

    /**
     * @brief Converts the string to its double representation as a value, if it is present in the domain.
     * @param strValue The value as a string
     * @param dblValue The value returned, if available. Only relevant if a member of the domain.
     * @return true if the value was present, otherwise false.
     */
    virtual bool convertToMemberValue(const std::string& strValue, double& dblValue) const = 0;

    /**
     * @brief Tests if both bounds are finite. Trivially true for symbolic domains.
     */
    virtual bool areBoundsFinite() const;

    /**
     * @brief Tests if two domains can be compared. For example, one cannot compare a symbolic
     * enumerated domain with a numeric domain. This is useful to enforce type checking
     * in constraints in particular.
     * @see DomainComparator::canCompare
     */
    static bool canBeCompared(const AbstractDomain& domx, const AbstractDomain& domy);

    /**
     * Tests if 2 values are the same with respect to the minimum difference for the target domain.
     */
    inline virtual bool compareEqual(double a, double b) const {
      return(a < b ? b - a < minDelta() : a - b < minDelta());
    }

    /**
     * @brief Tests if one value is less than another to within minDelta
     */
    inline bool lt(double a, double b) const {
      return (a + minDelta() <= b);
    }

    /**
     * @brief Tests if one value equals another to within minDelta
     */
    inline bool eq(double a, double b) const {
      return compareEqual(a, b);
    }

    /**
     * @brief Tests if one value is less than or equal to another to within minDelta
     */
    inline bool leq(double a, double b) const {
      return (a - minDelta() < b);
    }

    /**
     * "Deeply" copy the concrete C++ object into new memory and return a pointer to it.
     */
    virtual AbstractDomain *copy() const = 0;

    /**
     * @brief Creates a verbose string for displaying the contents of the domain
     */
    virtual std::string toString() const;

    /**
     * @brief Creates a concise string for displaying the value
     * @param value must be a member of the domain.
     */
    virtual std::string toString(double value) const;

  protected:
    /**
     * @brief Constructor.
     * @param closed indicates if the domain is to be dynamic or closed on construction.  Once closed, no
     * additions to extend the contents of the domain will be permitted.
     * @param enumerated true if this is an explicit enumeration.
     * @param typeName indicates the type name to use
     * @todo Review how semantics of closed can be enforced in operations.
     */
    AbstractDomain(bool closed, bool enumerated, const char* typeName);

    /**
     * @brief Copy Constructor
     * @param org Original domain
     */
    AbstractDomain(const AbstractDomain& org);

    /**
     * @brief Helper method to push messages to the listener.
     * @see DomainListener
     */
    void notifyChange(const DomainListener::ChangeType& changeType);

    /**
     * @brief Check that the value is correct w.r.t. the semantics of the domain. Infinite safe if numeric.
     */
    virtual bool check_value(double value) const;

    /**
     * @brief Check the precision of a value in terms of a particular Domain class.
     * @note 'Hook' to permit subclasses to be more restrictive; called by check_value()
     * unless compiled EUROPA_FAST.
     */
    virtual void testPrecision(const double& value) const = 0;

    /**
     * @brief Utility function for internal use
     */
    static void assertSafeComparison(const AbstractDomain& domA, const AbstractDomain& domB);

    bool m_closed; /**< False if the domain is dynamic (can be added to), otherwise true. */
    bool m_enumerated; /**< False if the domain is an interval, otherwise true. */
    DomainListenerId m_listener; /**< Holds reference to attached listener.  May be noId. */
    LabelStr m_typeName; /**< The name of the type of this domain. */
    double m_minDelta; /**< The minimum amount by which elements of this domain may vary.  Once this is set, DO NOT CHANGE IT.*/

  };
}
#endif
