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
#include <bitset>

namespace Prototype{

  /**
   * @class EnumeratedDomain
   * @brief Declares an enumerated domain of doubles..
   * 
   * The implementation uses a sorted set of doubles which hold all the values possible in the set, and then refines membership using
   * a bit vector.
   */
  class EnumeratedDomain: public AbstractDomain{
  public:

    /**
     * @brief Constructor
     * @param values The initial set of values to populate the domain.
     * @param closed Indicate if the set is initially closed.
     * @param listener Allows connection of a listener to change events on the domain. 
     * @see AbstractDomain::isDynamic()
     */
    EnumeratedDomain(const std::list<double>& values, 
	     bool closed = true,
	     const DomainListenerId& listener = DomainListenerId::noId());

    /**
     * @brief Copy constructor
     * @param org The source domain.
     */
    EnumeratedDomain(const EnumeratedDomain& org);

    /**
     * @brief Empty domain default constructor
     */
    EnumeratedDomain();

    /**
     * @brief Get the type of the domain to aid in type checking
     * @see AbstractDomain::DomainType
     */
    virtual const DomainType& getType() const;

    /**
     * @brief Determine if the domain is finite
     */
    bool isFinite() const;

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
     * @brief Add an elemenet to the set. This is only permitted on dynamic domains.
     *
     * @param value The value to insert. If not currently present it will be inserted. Otherwise it will be
     * ignored. If inserted, this operation constitutes a domain relaxation and will result in a relaxtion event
     * being raised.
     * @see DomainListener::DOMAIN_RELAXED
     * @todo Consider if it makes sense to error out if isMember(value)
     */
    void insert(double value);

    /**
     * @brief Remove the given elemenet form the domain.
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
    void set(const AbstractDomain& dom);

    /**
     * @brief Attempt to set the domain to a singleton.
     *
     * If the value is not a member of the domain then this will cause the domain to be emptied. If the set is already the singleton
     * value provided then nothing happens. Otherwise the domain is set to the target value and an event raised.
     * @param value The target singleton value.
     * @see DomainListener::EMPTIED, DomainListener::SET_TO_SINGLETON
     */
    void set(double value);

    /**
     * @brief Reset the domain to the target value
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
     * This method is provided as a more efficient way to handle the common operation of equatint 2 EnumeratedDomain domains.
     * Tackling changes to both domains at once allows us to take advantage of the sorted order of the domains.
     * @param The domain to be equated with this object. It may change.
     * @return true if a change to either domain has occurred.
     * @see DomainListener::EMPTIED, DomainListener::SET_TO_SINGLETON, DomainListener::VALUE_REMOVED
     */
    bool equate(AbstractDomain& dom);

    /**
     * @brief Return the singleton value. Only callable when it ius in fact a singleton.
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
     * @brief Access upper bound
     */
    double getUpperBound() const;

    /**
     * @brief Access lower bound
     */
    double getLowerBound() const;


    /**
     * @brief Access both bounds in a convenience method, and indicates if the domain is infinite
     * @param lb update this value with the lower bound
     * @param ub update this value with the upper bound
     * @return true if !isFinite()
     */
    bool getBounds(double& lb, double& ub) const;

    /**
     * @brief Test if the given value is a member of the set.
     *
     * Not allowed to call this if the domain is empty
     * @param value The value to test
     * @return true if present, otherwise false.
     */
    bool isMember(double value) const;

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
     * @brief Tests if this object is a subset of the given domain.
     * @param dom the domain to be compared against
     * @return true if all elements in this domain are present in dom, otherwise false.
     */
    bool isSubsetOf(const AbstractDomain& dom) const;
  protected:
    int getIndex(double value) const;
    bool sameBaseSet(const EnumeratedDomain& dom) const;

    static const int MAX_SIZE = 32; /*!< Since bitset have to have a fixed size, we allocate one large enough to hold all */

    std::set<double> m_values; /*!< Holds the contents from which the set membership is then derived. */
    std::bitset<MAX_SIZE> m_membership; /*!< Efficient way to test for membership. */
  };
}
#endif
