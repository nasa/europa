#ifndef _H_EnumeratedDomain
#define _H_EnumeratedDomain

/**
 * @file EnumeratedDomain.hh
 * @brief Provides the declaration for the EnumeratedDomain class, which is a concrete implementation of AbstractDomain
 * @author Conor McGann
 * @date August, 2003
 */
#include "AbstractDomain.hh"
#include <set>

namespace EUROPA {

  /**
   * @class EnumeratedDomain
   * @brief Declares an enumerated domain of doubles..
   * 
   * The implementation uses a sorted set of doubles which hold all the values possible in the set, and then refines membership using
   * a bit vector.
   */
  class EnumeratedDomain : public AbstractDomain {
  public:

    /**
     * @brief Constructor.
     * @param isNumeric Indicate if the set is to be used to store numeric or symbolic values
     * @param typeName Indicate the type name to use as a specialization of enumeration types 
     * @see AbstractDomain::isDynamic()
     */
    EnumeratedDomain(bool isNumeric,
		     const char* typeName);

    /**
     * @brief Constructor.
     * @param values The initial set of values to populate the domain.
     * @param isNumeric Indicate if the set is to be used to store numeric or symbolic values
     * @param typeName Indicate the type name to use as a specialization of enumeration types 
     * @see AbstractDomain::isDynamic()
     */
    EnumeratedDomain(const std::list<double>& values, 
		     bool isNumeric,
		     const char* typeName);

    /**
     * @brief Constructor.
     * @param value Constructs a singleton domain. Closed on construction.
     * @param isNumeric Indicate if the set is to be used to store numeric or symbolic values
     * @param typeName Indicate the type name to use as a specialization of enumeration types 
     */
    EnumeratedDomain(double value,
		     bool isNumeric,
		     const char* typeName);

    /**
     * @brief Copy constructor.
     * @param org The source domain.
     */
    EnumeratedDomain(const AbstractDomain& org);

    /**
     * @brief Get the type of the domain to aid in type checking.
     * @see AbstractDomain::DomainType
     */
    virtual const DomainType& getType() const;

    /**
     * @brief Get the default name of the type of the domain.
     * @see AbstractDomain::getTypeName
     */
    static const LabelStr& getDefaultTypeName();

    /**
     * @brief Determine if the domain is finite.
     */
    bool isFinite() const;

    /**
     * @brief Check if the domain is numeric.
     */
    bool isNumeric() const;

    /**
     * @see AbstractDomain::isSingleton()
     */
    bool isSingleton() const;

    /**
     * @see AbstractDomain::isEmpty()
     */
    bool isEmpty() const;

    /**
     * @see AbstractDomain::empty()
     */
    void empty();

    /**
     * @brief Over-ride to ensure that the doain becomes finite once closed.
     * @see AbstractDomain::close()
     */
    void close();

    /**
     * @brief Return the number of elements in the set.
     * @return isEmpty() <=> 0, isSingleton() <=> 1
     * @see AbstractDomain::getSize()
     */
    int getSize() const;

    /**
     * @brief Add an element to the set.
     * @param value The value to insert. If not currently present it will be inserted. Otherwise it will be
     * ignored. If inserted, this operation constitutes a domain relaxation and will result in a relaxtion event
     * being raised.
     * @see DomainListener::DOMAIN_RELAXED
     * @todo Consider if it makes sense to error out if isMember(value)
     */
    virtual void insert(double value);

    virtual void insert(const std::list<double>& values);

    /**
     * @brief Remove the given element form the domain.
     * @param value. The value to be removed. If present, removal will generate a value removal event
     * @see DomainListener::VALUE_REMOVED
     */
    void remove(double value);

    /**
     * @brief Attempt to set the domain to the target.
     *
     * Indicates an external call to set the domain to the given target.
     * @param dom The target domain.
     * @see DomainListener::EMPTIED, DomainListener::SET, intersect()
     */
    virtual void set(const AbstractDomain& dom);

    /**
     * @brief Attempt to set the domain to a singleton.
     *
     * If the value is not a member of the domain then this will cause the domain to be emptied. If the set is already the singleton
     * value provided then nothing happens. Otherwise the domain is set to the target value and an event raised.
     * @param value The target singleton value.
     * @see DomainListener::EMPTIED, DomainListener::SET_TO_SINGLETON
     */
    virtual void set(double value);

    /**
     * @brief Reset the domain to the target value.
     */
    void reset(const AbstractDomain& dom);

    /**
     * @brief Indicates assigment to the target domain as a relaxation triggered internally.
     * @param value the target singleton value.
     * @see relax
     */
    void relax(const AbstractDomain& dom);

    /**
     * @brief Construct a mutual restriction of the 2 domains to the intersection between them.
     *
     * This method is provided as a more efficient way to handle the common operation of equating 2 EnumeratedDomain domains.
     * Tackling changes to both domains at once allows us to take advantage of the sorted order of the domains.
     * @param The domain to be equated with this object. It may change.
     * @return true if a change to either domain has occurred.
     * @see DomainListener::EMPTIED, DomainListener::SET_TO_SINGLETON, DomainListener::VALUE_REMOVED
     */
    bool equate(AbstractDomain& dom);

    /**
     * @brief Return the singleton value.
     * @note Only callable when it is in fact a singleton.
     * @return A copy of the stored (and encoded) singleton value.
     */
    double getSingletonValue() const;

    /**
     * @brief Fill the given list with the contents of the set.
     * 
     * Should only be called on finite (and thus closed) domains.
     * @param results The target collection to fill with all values in the set.
     */
    void getValues(std::list<double>& results) const;


    /**
     * @brief Retrieve the contents as a set
     */
    const std::set<double>& getValues() const;

    /**
     * @brief Access upper bound.
     */
    double getUpperBound() const;

    /**
     * @brief Access lower bound.
     */
    double getLowerBound() const;

    /**
     * @brief Access both bounds in a convenience method, and indicate if the domain is infinite.
     * @param lb update this value with the lower bound.
     * @param ub update this value with the upper bound.
     * @return true if !isFinite()
     */
    bool getBounds(double& lb, double& ub) const;

    /**
     * @brief Test if the given value is a member of the set.
     * @param value The value to test
     * @return true if present, otherwise false.
     * @note Not allowed to call this if the domain is empty
     */
    bool isMember(double value) const;

    /**
     * @brief Obtain the double encoded value from the string if it is a member.
     */
    virtual bool convertToMemberValue(const std::string& strValue, double& dblValue) const;

    /**
     * @brief Test that the domains are exactly equal.
     * @param dom The domain to test against
     * @return true if the values in each are the same and they are equal according to the base class.
     * @see AbstractDomain::operator==()
     */
    bool operator==(const AbstractDomain& dom) const;

    /**
     * @brief Test that the domains are not equal.
     * @param dom The domain to test against
     * @return true if the values in each are not the same.
     * @see AbstractDomain::operator!=()
     */
    bool operator!=(const AbstractDomain& dom) const;

    /**
     * @brief Computes the intersection of this object and the given object and assigns that intersection to this object.
     * @param dom the domain to be intersected with
     * @return true if a change occurs, otherwise false.
     */
    bool intersect(const AbstractDomain& dom);

    /**
     * @brief restricts this domain to the difference of its values with the given domain.
     * @param dom the domain to differ with. Must not be empty.
     * @return true if the operation results in a change to this domain, otherwise false.
     */
    bool difference(const AbstractDomain& dom);

    /**
     * @brief Assign the values from the given domain, to this domain.
     * @note Can only be called on domains that have no listeners
     * attached, since it will not cause propagation. It is more of a
     * utility.
     */
    AbstractDomain& operator=(const AbstractDomain& dom);

    /**
     * @brief Tests if this object is a subset of the given domain.
     * @param dom the domain to be compared against
     * @return true if all elements in this domain are present in dom, otherwise false.
     */
    bool isSubsetOf(const AbstractDomain& dom) const;

    /**
     * @brief test if the intersection between this domain and the given domain is empty
     * @param dom the domain tested against.
     * @param true if any elements of this domain are in dom. Otherwise false.
     */
    bool intersects(const AbstractDomain& dom) const;

    void operator>>(ostream& os) const;

    /**
     * @brief Copy the concrete C++ object into new memory and return a pointer to it.
     */
    virtual EnumeratedDomain *copy() const;

  protected:

    /**
     * @brief Enforces enumeration semantics.
     * @note Will be compiled out for fast version.
     * @note No-op for enumerations.
     */
    virtual void testPrecision(const double& value) const {
    }

    std::set<double> m_values; /**< Holds the contents from which the set membership is then derived. */
    bool m_isNumeric;
  };
}
#endif
