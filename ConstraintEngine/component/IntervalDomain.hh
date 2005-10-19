#ifndef _H_IntervalDomain
#define _H_IntervalDomain

/**
 * @file IntervalDomain.hh
 * @author Conor McGann
 * @date August, 2003
 */

#include "AbstractDomain.hh"

namespace EUROPA {

  /**
   * @class IntervalDomain
   * @brief Abstract base class for all interval domains.
   * Derived classes impose restrictions on the general semantics of this base class.
   *
   * @todo Possible additions: support for open ended intervals and multiple intervals,
   * preferably as new classes rather than impacting the performance of this class.
   * --wedgingt@email.arc.nasa.gov 2004 Feb 26
   */
  class IntervalDomain : public AbstractDomain {
  public:

    /**
     * @brief Override the base class method
     */
    void operator>>(ostream& os) const;

    IntervalDomain();

    IntervalDomain(const char* typeName);

    IntervalDomain(double lb, double ub);

    IntervalDomain(double value);

    IntervalDomain(double lb, double ub, const char* typeName);

    IntervalDomain(const AbstractDomain& org);

    /**
     * @brief Destructor.
     */
    virtual ~IntervalDomain();

    virtual bool isFinite() const;

    /**
     * @brief IntervalDomains are always numeric.
     * @note Even if they are a BoolDomain, which presently inherits from
     * this class via class IntervalIntDomain.
     */
    virtual bool isNumeric() const {
      return(true);
    }

    /**
     * @brief IntervalDomains are not BoolDomain unless it is instantiated
     * as BoolDomain.
     * @note BoolDomain presently inherits from this class via class
     * IntervalIntDomain. 
     */
    virtual bool isBool() const {
      return(false);
    }

    /**
     * @brief Get the default name of the type of the domain.
     * @see AbstractDomain::getTypeName
     */
    static const LabelStr& getDefaultTypeName();

    /**
     * @brief Access upper bound.
     */
    double getUpperBound() const;

    /**
     * @brief Access lower bound.
     */
    double getLowerBound() const;

    /**
     * @brief Access singleton value.
     * @note Must be a singleton or this will fail.
     */
    double getSingletonValue() const;

    /**
     * @brief Access both bounds in a convenience method, and indicate if the domain is infinite.
     * @param lb update this value with the lower bound.
     * @param ub update this value with the upper bound.
     * @return true if !isFinite()
     */
    inline bool getBounds(double& lb, double& ub) const {
      lb = m_lb;
      ub = m_ub;
      return(m_ub == PLUS_INFINITY || m_lb == MINUS_INFINITY);
    }

    /**
     * @brief Set to the specified domain.
     * @note May empty the domain if target does not intersect the current domain.
     * @param value the target singleton value.
     */
    void set(const AbstractDomain& dom);

    /**
     * @brief Set to a singleton.
     * @note May empty the domain if value is not a member of the current domain.
     * @param value the target singleton value.
     */
    void set(double value);

    /**
     * @brief Indicates assigment to the target domain as a relaxation triggered externally.
     * @param value the target singleton value.
     * @see relax
     */
    void reset(const AbstractDomain& dom);

    /**
     * @brief Restricts this domain to the intersection of its values with the given domain.
     * @param dom the domain to intersect with, which cannot be empty.
     * @return true if the intersection results in a change to this domain, otherwise false.
     */
    virtual bool intersect(const AbstractDomain& dom);

    /**
     * @brief Restricts this domain to the difference of its values with the given domain.
     * @param dom the domain to differ with, which cannot be empty.
     * @return true if the operation results in a change to this domain, otherwise false.
     */
    bool difference(const AbstractDomain& dom);

    /**
     * @brief Assign the values from the given domain, to this domain.
     * @note Can only be called on domains that have no listeners
     * attached, since it will not cause propagation.
     */
    AbstractDomain& operator=(const AbstractDomain& dom);

    /**
     * @brief Convenience version of intersect.
     * @param lb the lower bound of domain to intersect with.
     * @param ub the upper bound of domain to intersect with.
     * @return true if the intersection results in a change to this domain, otherwise false.
     * @note ub must be >= lb.
     */
    virtual bool intersect(double lb, double ub);

    /**
     * @brief Force the domain to empty.
     * @see DomainListener::EMPTIED
     */
    void empty();

    /**
     * @brief Relax this domain to that of the given domain.
     * @param dom The domain to relax it to, which cannot be empty and
     * must be a superset of this domain.
     */
    void relax(const AbstractDomain& dom);

    /**
     * @brief Convenience method for relaxing a domain.
     * @param lb the lower bound of domain to relax to, which must be <= m_lb.
     * @param ub the upper bound of domain to relax to, which must be >= m_ub.
     * @return true if relaxation causes a change to this domain.
     * @see operator=(const AbstractDomain& dom)
     */
    bool relax(double lb, double ub);

    /**
     * @brief Add an element to the set.
     * @param value The value to insert. If not currently present it
     * will be inserted. Otherwise it will be ignored. If inserted
     * into a non-dynamic domain, this operation constitutes a domain
     * relaxation and will result in a relaxation event being raised.
     * @see DomainListener::DOMAIN_RELAXED
     * @note An error for real intervals unless already in the set or
     * was empty and now singleton.
     */
    void insert(double value);

    void insert(const std::list<double>& values);

    /**
     * @brief Remove the given element form the domain.
     * @param value. The value to be removed.
     * @note If the value was in the domain, this call will generate a
     * value removal event.
     * @note An error for real intervals unless not in the set or was
     * singleton and is now empty.
     * @see DomainListener::VALUE_REMOVED
     */
    virtual void remove(double value);

    /**
     * @brief test for membership.
     * @param value Value to test for.
     * @return true if a member of the domain, otherwise false.
     * @note Ought to be called 'hasMember()': 'domain.hasMember(value)'.
     */
    bool isMember(double value) const;

    /**
     * @brief Convert to member value from string encoding.
     */
    bool convertToMemberValue(const std::string& strValue, double& dblValue) const;

    /**
     * @brief Test for single valued domain.
     */
    bool isSingleton() const;

    /**
     * @brief Test for empty domain.
     * @note Can only call this on closed domains.
     */
    bool isEmpty() const;

    /**
     * @brief Return the number of elements in the domain.
     * @note Can only be called on finite domains.
     */
    int getSize() const;

    /**
     * @brief Test for equality.
     */
    bool operator==(const AbstractDomain& dom) const;

    /**
     * @brief Test for inequality.
     */
    bool operator!=(const AbstractDomain& dom) const;

    /**
     * @brief Test if this domain is a subset of dom.
     * @param dom the domain tested against.
     * @param true if all elements of this domain are in dom, otherwise false.
     */
    bool isSubsetOf(const AbstractDomain& dom) const;

    /**
     * @brief Test if the intersection between this domain and the given domain is empty.
     * @param dom the domain tested against.
     * @param true if any elements of this domain are in dom, otherwise false.
     */
    bool intersects(const AbstractDomain& dom) const;

    /**
     * @brief Fill the given list with the contents of the set.
     * @note Should only be called on singleton domains.
     * @param results The target collection to fill with the value in the set.
     */
    void getValues(std::list<double>& results) const;

    /**
     * @brief mutually constraint both domains to their respective intersections.
     * @param dom The domain to perform mutual intersection with.
     * @return true if the intersection results in a change to either domain, otherwise false. 
     * @note If the intersection is empty, only one domain is actually emptied.
     */
    bool equate(AbstractDomain& dom);

    /**
     * @brief Enforces semantics of PLUS or MINUS infinity.
     */
    virtual double translateNumber(double number, bool asMin = true) const;

    /**
     * @brief Copy the concrete C++ object into new memory and return a pointer to it.
     */
    virtual IntervalDomain *copy() const;

  protected:

    /**
     * @brief Helper method to test if the given value can be considered an integer.
     * @note Used in derived class.
     * @see testPrecision
     */
    double check(const double& value) const;

    /**
     * @brief Tests if the given value is of the correct type for the domain type.
     * Mostly used for restricting values of doubles to int. However,
     * we could restrict it in other ways perhaps.
     * @note No-op for real domains.
     */
    virtual void testPrecision(const double& value) const;


    /**
     * @brief Carries out the conversion of the given double to do appropriate rounding.
     * @param value The value to be converetd
     * @return The value subject to any rounding required for th sub-type (e.g. int)
     */
    virtual double convert(const double& value) const;

    /**
     * @brief Conduct common initialization across constructors.
     */
    void commonInit();

    double m_ub; /**< The upper bound of the domain */
    double m_lb; /**< The lower bound o fthe domain */
  };
}
#endif
