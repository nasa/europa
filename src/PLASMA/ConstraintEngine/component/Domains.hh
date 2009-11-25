/*
 * Domains.hh
 *
 *  Created on: Apr 23, 2009
 *      Author: jbarreir
 */

#ifndef DOMAINS_HH_
#define DOMAINS_HH_

#include "AbstractDomain.hh"
#include "DataTypes.hh"

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
	   * @param dt Indicate the type to use as a specialization of enumeration types
	   * @see AbstractDomain::isDynamic()
	   */
	  EnumeratedDomain(const DataTypeId& dt);

	  /**
	   * @brief Constructor.
	   * @param dt Indicate the type to use as a specialization of enumeration types
	   * @param values The initial set of values to populate the domain.
	   * @see AbstractDomain::isDynamic()
	   */
	  EnumeratedDomain(const DataTypeId& dt, const std::list<edouble>& values);

	  /**
	   * @brief Constructor.
	   * @param dt Indicate the type to use as a specialization of enumeration types
	   * @param value Constructs a singleton domain. Closed on construction.
	   */
	  EnumeratedDomain(const DataTypeId& dt, edouble value);

	  /**
	   * @brief Constructor.
	   * @param dt Indicate the type to use as a specialization of enumeration types
	   * @param value Constructs a singleton domain. Closed on construction.
	   */
	  EnumeratedDomain(const DataTypeId& dt, double value);

	  /**
	   * @brief Copy constructor.
	   * @param org The source domain.
	   */
	  EnumeratedDomain(const AbstractDomain& org);

	  /**
	   * @brief Determine if the domain is finite.
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
	  unsigned int getSize() const;

	  /**
	   * @brief Add an element to the set.
	   * @param value The value to insert. If not currently present it will be inserted. Otherwise it will be
	   * ignored. If inserted, this operation constitutes a domain relaxation and will result in a relaxtion event
	   * being raised.
	   * @see DomainListener::DOMAIN_RELAXED
	   * @todo Consider if it makes sense to error out if isMember(value)
	   */
	  virtual void insert(edouble value);

	  virtual void insert(const std::list<edouble>& values);

	  /**
	   * @brief Remove the given element form the domain.
	   * @param value. The value to be removed. If present, removal will generate a value removal event
	   * @see DomainListener::VALUE_REMOVED
	   */
	  void remove(edouble value);

	  /**
	   * @brief Attempt to set the domain to a singleton.
	   *
	   * If the value is not a member of the domain then this will cause the domain to be emptied. If the set is already the singleton
	   * value provided then nothing happens. Otherwise the domain is set to the target value and an event raised.
	   * @param value The target singleton value.
	   * @see DomainListener::EMPTIED, DomainListener::SET_TO_SINGLETON
	   */
	  virtual void set(edouble value);

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
	   * @brief Indicates relaxation to a singleton value. Occurs when domain has been emptied previously
	   * @param The value to relax to
	   */
	  void relax(edouble value);

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
	  edouble getSingletonValue() const;

	  /**
	   * @brief Fill the given list with the contents of the set.
	   *
	   * Should only be called on finite (and thus closed) domains.
	   * @param results The target collection to fill with all values in the set.
	   */
	  void getValues(std::list<edouble>& results) const;


	  /**
	   * @brief Retrieve the contents as a set
	   */
	  const std::set<edouble>& getValues() const;

	  /**
	   * @brief Access upper bound.
	   */
	  edouble getUpperBound() const;

	  /**
	   * @brief Access lower bound.
	   */
	  edouble getLowerBound() const;

	  /**
	   * @brief Access both bounds in a convenience method, and indicate if the domain is infinite.
	   * @param lb update this value with the lower bound.
	   * @param ub update this value with the upper bound.
	   * @return true if !isFinite()
	   */
	  bool getBounds(edouble& lb, edouble& ub) const;

	  /**
	   * @brief Test if the given value is a member of the set.
	   * @param value The value to test
	   * @return true if present, otherwise false.
	   * @note Not allowed to call this if the domain is empty
	   */
	  bool isMember(edouble value) const;

	  /**
	   * @brief Obtain the double encoded value from the string if it is a member.
	   */
	  virtual bool convertToMemberValue(const std::string& strValue, edouble& dblValue) const;

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
	   * @brief Convenience version of intersect.
	   * @param lb the lower bound of domain to intersect with.
	   * @param ub the upper bound of domain to intersect with.
	   * @return true if the intersection results in a change to this domain, otherwise false.
	   * @note ub must be >= lb.
	   */
	  bool intersect(edouble lb, edouble ub);

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

	  /**
	   * @brief Creates a verbose string for displaying the contents of the domain
	   */
	  virtual std::string toString() const;

  protected:

	  /**
	   * @brief Enforces enumeration semantics.
	   * @note Will be compiled out for fast version.
	   * @note No-op for enumerations.
	   */
	  virtual void testPrecision(const edouble& value) const {}

	  /**
	   * @brief Implements equate where both are closed enumerations
	   */
	  bool equateClosedEnumerations(EnumeratedDomain& dom);

	  std::set<edouble> m_values; /**< Holds the contents from which the set membership is then derived. */
  };


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

    IntervalDomain(const DataTypeId& dt = FloatDT::instance());
    IntervalDomain(edouble lb, edouble ub, const DataTypeId& dt = FloatDT::instance());
    IntervalDomain(double lb, double ub, const DataTypeId& dt = FloatDT::instance());
    IntervalDomain(edouble value, const DataTypeId& dt = FloatDT::instance());
    IntervalDomain(double value, const DataTypeId& dt = FloatDT::instance());
    IntervalDomain(const AbstractDomain& org);

    /**
     * @brief Destructor.
     */
    virtual ~IntervalDomain();

    virtual bool isFinite() const;

    /**
     * @brief Access upper bound.
     */
    edouble getUpperBound() const;

    /**
     * @brief Access lower bound.
     */
    edouble getLowerBound() const;

    /**
     * @brief Access singleton value.
     * @note Must be a singleton or this will fail.
     */
    edouble getSingletonValue() const;

    /**
     * @brief Access both bounds in a convenience method, and indicate if the domain is infinite.
     * @param lb update this value with the lower bound.
     * @param ub update this value with the upper bound.
     * @return true if !isFinite()
     */
    inline bool getBounds(edouble& lb, edouble& ub) const {
      lb = m_lb;
      ub = m_ub;
      return(m_ub == PLUS_INFINITY || m_lb == MINUS_INFINITY);
    }

    /**
     * @brief Set to a singleton.
     * @note May empty the domain if value is not a member of the current domain.
     * @param value the target singleton value.
     */
    void set(edouble value);

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
    virtual bool intersect(edouble lb, edouble ub);

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
     * @brief Relax to a singleton value
     */
    void relax(edouble value);

    /**
     * @brief Convenience method for relaxing a domain.
     * @param lb the lower bound of domain to relax to, which must be <= m_lb.
     * @param ub the upper bound of domain to relax to, which must be >= m_ub.
     * @return true if relaxation causes a change to this domain.
     * @see operator=(const AbstractDomain& dom)
     */
    bool relax(edouble lb, edouble ub);

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
    void insert(edouble value);

    void insert(const std::list<edouble>& values);

    /**
     * @brief Remove the given element form the domain.
     * @param value. The value to be removed.
     * @note If the value was in the domain, this call will generate a
     * value removal event.
     * @note An error for real intervals unless not in the set or was
     * singleton and is now empty.
     * @see DomainListener::VALUE_REMOVED
     */
    virtual void remove(edouble value);

    /**
     * @brief test for membership.
     * @param value Value to test for.
     * @return true if a member of the domain, otherwise false.
     * @note Ought to be called 'hasMember()': 'domain.hasMember(value)'.
     */
    bool isMember(edouble value) const;

    virtual edouble translateNumber(edouble number, bool asMin = true) const {return AbstractDomain::translateNumber(number, asMin);}

    /**
     * @brief Convert to member value from string encoding.
     */
    bool convertToMemberValue(const std::string& strValue, edouble& dblValue) const;

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
    unsigned int getSize() const;

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
    void getValues(std::list<edouble>& results) const;

    /**
     * @brief mutually constraint both domains to their respective intersections.
     * @param dom The domain to perform mutual intersection with.
     * @return true if the intersection results in a change to either domain, otherwise false.
     * @note If the intersection is empty, only one domain is actually emptied.
     */
    bool equate(AbstractDomain& dom);

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
    edouble check(const edouble& value) const;

    /**
     * @brief Tests if the given value is of the correct type for the domain type.
     * Mostly used for restricting values of doubles to int. However,
     * we could restrict it in other ways perhaps.
     * @note No-op for real domains.
     */
    virtual void testPrecision(const edouble& value) const;


    /**
     * @brief Carries out the conversion of the given double to do appropriate rounding.
     * @param value The value to be converetd
     * @return The value subject to any rounding required for th sub-type (e.g. int)
     */
    virtual edouble convert(const edouble& value) const;

    /**
     * @brief Conduct common initialization across constructors.
     */
    void commonInit();

    edouble m_ub; /**< The upper bound of the domain */
    edouble m_lb; /**< The lower bound o fthe domain */
  };

  inline IntervalDomain make_int(const edouble v) {return IntervalDomain(v);}
  inline IntervalDomain make_int(const edouble v1, const edouble v2) {return IntervalDomain(v1, v2);}

  // TODO! : All the classes below seem unnecessary now that DataType has been factored out of AbstractDomain

  /**
   * @class StringDomain
   * @brief an enumerated domain of strings
   */
  class StringDomain : public EnumeratedDomain {
  public:

    StringDomain(const DataTypeId& dt = StringDT::instance());
    StringDomain(edouble value, const DataTypeId& dt = StringDT::instance());
    StringDomain(double value, const DataTypeId& dt = StringDT::instance());
    StringDomain(const std::string& value, const DataTypeId& dt = StringDT::instance());
    StringDomain(const std::list<edouble>& values, const DataTypeId& dt = StringDT::instance());
    StringDomain(const AbstractDomain& org);

    virtual StringDomain *copy() const;

    /**
     * @brief Sets a singleton value.
     * @param value The value to set. Must be a LabelStr.
     */
    void set(edouble value);

    bool isMember(edouble value) const;

    /** String specific bindings for user convenience **/
    void set(const std::string& value);
    bool isMember(const std::string& value) const;
    void insert(const std::string& value);
    void insert(edouble value);
  };

  /**
   * @class SymbolDomain
   * @brief an enumerated domain of strings
   */
  class SymbolDomain : public EnumeratedDomain {
  public:
    SymbolDomain(const DataTypeId& dt = SymbolDT::instance());
    SymbolDomain(edouble value,const DataTypeId& dt = SymbolDT::instance());
    SymbolDomain(double value,const DataTypeId& dt = SymbolDT::instance());
    SymbolDomain(const std::list<edouble>& values,const DataTypeId& dt = SymbolDT::instance());
    SymbolDomain(const AbstractDomain& org);

    virtual SymbolDomain *copy() const;
  };

  class NumericDomain : public EnumeratedDomain {
  public:

    NumericDomain(const DataTypeId& dt = FloatDT::instance());
    NumericDomain(edouble value, const DataTypeId& dt = FloatDT::instance());
    NumericDomain(double value, const DataTypeId& dt = FloatDT::instance());
    NumericDomain(const std::list<edouble>& values, const DataTypeId& dt = FloatDT::instance());
    NumericDomain(const AbstractDomain& org);

    virtual NumericDomain *copy() const;
  };

  inline NumericDomain make_num(const edouble v) { return NumericDomain(v);}

  /**
   * @class IntervalIntDomain
   * @brief Imposes restrictions on the more generic super class.
   *
   * Restrictions are:
   * @li If closed then it is finite.
   * @li All modification operations on the bounds must be checked to ensure they are integers
   */
  class IntervalIntDomain: public IntervalDomain {
  public:

    IntervalIntDomain(const DataTypeId& dt = IntDT::instance());
    IntervalIntDomain(eint lb, eint ub, const DataTypeId& dt = IntDT::instance());
    IntervalIntDomain(int lb, int ub, const DataTypeId& dt = IntDT::instance());
    IntervalIntDomain(eint value, const DataTypeId& dt = IntDT::instance());
    IntervalIntDomain(int value, const DataTypeId& dt = IntDT::instance());
    IntervalIntDomain(const AbstractDomain& org);

    virtual ~IntervalIntDomain();

    virtual bool isFinite() const;

    virtual bool isSingleton() const;

    /**
     * @brief Add an element to the domain.
     * @param value The value to insert.
     * @note Will generate a domain relaxation if value is not already in the domain.
     * @note Will generate an error if value not within or "next to" the existing interval.
     * @see DomainListener::DOMAIN_RELAXED, AbstractDomain::insert
     * @note This implementation might also work in IntervalDomain
     * since it uses minDelta().
     */
    virtual void insert(edouble value);

    /**
     * @brief Fill the given list with the contents of the set.
     * @note Should only be called on finite (and thus closed) domains.
     * @param results The target collection to fill with all values in the set.
     */
    virtual void getValues(std::list<edouble>& results) const;

    virtual edouble translateNumber(edouble number, bool asMin = true) const;

    /**
     * @brief Copy the concrete C++ object into new memory and return a pointer to it.
     */
    virtual IntervalIntDomain *copy() const;

    virtual bool intersect(edouble lb, edouble ub);

    virtual bool intersect(const AbstractDomain& dom);

  protected:

    /**
     * @brief Enforce integer semantics.
     * @note Will be compiled out for fast version.
     */
    virtual void testPrecision(const edouble& value) const;

    /**
     * @brief Enforce integer semantics.
     */
    edouble convert(const edouble& value) const;
  };

  inline IntervalDomain make_int_int(const eint v) {return IntervalIntDomain(v);}
  inline IntervalDomain make_int_int(const eint v1, const eint v2) {return IntervalIntDomain(v1, v2);}

  /**
   * @class BoolDomain
   * @brief Imposes restrictions on the more generic super class.
   *
   * Restrictions are:
   * @li Always closed and so always finite.
   * @li All modification operations on the bounds must be checked to ensure they are integers.
   * @li Only values of 0 and 1 allowed.
   */
  class BoolDomain : public IntervalIntDomain {
  public:

    BoolDomain(const DataTypeId& dt = BoolDT::instance());
    BoolDomain(bool value, const DataTypeId& dt = BoolDT::instance());
    BoolDomain(const AbstractDomain& org);

    bool isFinite() const;

    bool isFalse() const;

    bool isTrue() const;

    /**
     * @brief Copy the concrete C++ object into new memory and return a pointer to it.
     */
    virtual BoolDomain *copy() const;

    bool intersect(const AbstractDomain& dom);

    bool intersect(edouble lb, edouble ub);

  private:
    virtual void testPrecision(const edouble& value) const;
  };

}


#endif /* DOMAINS_HH_ */
